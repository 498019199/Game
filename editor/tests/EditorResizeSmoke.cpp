#include <editor/EditorManager.h>
#include <base/Window.h>
#include <world/World.h>
#include <render/RenderEngine.h>
#include <imgui_internal.h>
#include <common/ErrorHandling.h>
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

namespace EditorWorker
{
void EditorManager::RunResizeSmokeTest()
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    auto const hwnd = MainWnd()->GetHWND();
#else
    auto const& window = MainWnd();
#endif
    auto const selection = selected_asset_info_;
    auto material = std::dynamic_pointer_cast<AssetMaterialInfo>(selection);
    if (!material || !material->document) TMSG("Resize smoke needs a selected material document");
    if (material && material->document)
    {
        auto values = material->document->Parameters();
        values.name = "Unsaved resize smoke test";
        material->document->Edit(values); // In-memory only; deliberately leave dirty during resize.
    }
    auto const version = material && material->document ? material->document->Version() : 0;
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    auto require = [](bool value, char const* message) { if (!value) TMSG(message); };
    auto check = [&](char const* phase) {
        std::cout << "CHECK resize " << phase << std::endl;
        // Render through the same BeginFrame resize path used by the interactive app.
        for (int i = 0; i < 3; ++i)
        {
#ifdef ZENGINE_PLATFORM_DARWIN
            Window::PumpEvents();
#endif
            Context::Instance().WorldInstance().Update();
        }
        int layout_width = 0, layout_height = 0;
        uint32_t pixel_width = 0, pixel_height = 0;
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
        RECT client{};
        require(::GetClientRect(hwnd, &client) != 0, "GetClientRect failed");
        layout_width = client.right;
        layout_height = client.bottom;
        pixel_width = static_cast<uint32_t>(client.right);
        pixel_height = static_cast<uint32_t>(client.bottom);
#else
        layout_width = window->Width();
        layout_height = window->Height();
        auto const pixels = window->FramebufferSize();
        pixel_width = static_cast<uint32_t>(pixels.x());
        pixel_height = static_cast<uint32_t>(pixels.y());
#endif
        auto const& screen = re.ScreenFrameBuffer();
        require(setting_.srcWidth == layout_width && setting_.srcHeight == layout_height, "Stale Editor layout");
        require(screen->Width() == pixel_width && screen->Height() == pixel_height, "Stale screen framebuffer");
        require(screen->Viewport()->Width() == static_cast<int>(pixel_width) && screen->Viewport()->Height() == static_cast<int>(pixel_height), "Stale viewport");
        if (auto const& depth = screen->AttachedDsv())
            require(depth->Width() == pixel_width && depth->Height() == pixel_height, "Stale screen depth buffer");
        if (auto const& color = screen->AttachedRtv(RenderWorker::FrameBuffer::Attachment::Color0))
            require(color->Width() == pixel_width && color->Height() == pixel_height, "Stale swapchain backbuffer");
        require(game_view_color_tex_->Width(0) == static_cast<uint32_t>(setting_.gameViewWidth) && game_view_color_tex_->Height(0) == static_cast<uint32_t>(setting_.gameViewHeight),
            "Stale Game view texture");
        require(back_face_depth_fb_->Width() == static_cast<uint32_t>(setting_.gameViewWidth) && back_face_depth_fb_->Height() == static_cast<uint32_t>(setting_.gameViewHeight),
            "Stale backface target");
        require(std::abs(screen->Viewport()->Camera()->Aspect() - static_cast<float>(setting_.gameViewWidth) / setting_.gameViewHeight) < 1e-5f,
            "Incorrect Game camera aspect");
        auto panel = [&](char const* name, int x, int y, int w, int h) {
            auto const* window = ImGui::FindWindowByName(name);
            require(window && window->Pos.x == x && window->Pos.y == y && window->Size.x == w && window->Size.y == h,
                "ImGui panel does not match resized layout");
        };
        panel("Editor", 0, 0, setting_.mainBarWidth, setting_.mainBarHeight);
        panel("Hierarchy", 0, setting_.mainBarHeight, setting_.hierarchyWidth, setting_.hierarchyHeight);
        panel("Game", setting_.hierarchyWidth, setting_.mainBarHeight, setting_.gameViewWidth, setting_.gameViewHeight);
        panel("Inspector", setting_.hierarchyWidth + setting_.gameViewWidth, setting_.mainBarHeight, setting_.inspectorWidth, setting_.inspectorHeight);
        panel("Project", 0, setting_.mainBarHeight + setting_.hierarchyHeight, setting_.projectWidth, setting_.projectHeight);
        panel("Console", setting_.projectWidth, setting_.mainBarHeight + setting_.hierarchyHeight, setting_.consoleWidth, setting_.consoleHeight);
        require(selected_asset_info_ == selection, "Resize discarded the material selection");
        if (material && material->document)
            require(material->document->Dirty() && material->document->Version() == version
                && material->document->Parameters().name == "Unsaved resize smoke test", "Resize lost unsaved material edits");
        auto const texture = game_view_color_tex_;
        Context::Instance().WorldInstance().Update();
        require(game_view_color_tex_ == texture, "Unchanged window recreates Game texture each frame");
        std::cout << "PASS resize " << phase << ": layout=" << layout_width << 'x' << layout_height
            << " pixels=" << pixel_width << 'x' << pixel_height << std::endl;
    };
    // Test only this process's own native window. No asset writes or input injection.
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    ::ShowWindow(hwnd, SW_RESTORE);
    require(::SetWindowPos(hwnd, nullptr, 0, 0, 1100, 760, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE) != 0, "SetWindowPos failed");
    check("windowed");
    for (int i = 0; i < 2; ++i)
    {
        ::ShowWindow(hwnd, SW_MAXIMIZE);
        check("maximized");
        ::ShowWindow(hwnd, SW_RESTORE);
        check("restored");
    }
    ::ShowWindow(hwnd, SW_MINIMIZE);
    auto const texture = game_view_color_tex_;
    Context::Instance().WorldInstance().Update();
    require(game_view_color_tex_ == texture, "Minimize recreated Game texture");
    ::ShowWindow(hwnd, SW_RESTORE);
    check("restore-after-minimize");
    require(::SetWindowPos(hwnd, nullptr, 0, 0, 900, 650, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE) != 0, "SetWindowPos failed");
    check("smaller");
    ::ShowWindow(hwnd, SW_HIDE);
#else
    // Cocoa reports minimize/restore after the window animation completes.
    auto waitFor = [&](auto ready) {
        using namespace std::chrono_literals;
        auto const deadline = std::chrono::steady_clock::now() + 4s;
        while (!ready() && std::chrono::steady_clock::now() < deadline)
        {
            Window::PumpEvents();
            std::this_thread::sleep_for(20ms);
        }
        Window::PumpEvents();
        require(ready(), "Timed out waiting for the requested window state");
    };
    window->SetVisible(true);
    window->Restore();
    window->SetContentSize(1100, 760);
    waitFor([&]{ return !window->Minimized(); });
    check("windowed");
    auto const initial_width = setting_.srcWidth;
    auto const initial_height = setting_.srcHeight;
    for (int i = 0; i < 2; ++i)
    {
        window->Maximize();
        waitFor([&]{ return window->IsMaximized(); });
        require((window->IsMaximized()) != 0, "Window did not maximize");
        check("maximized");
        require(setting_.srcWidth != initial_width || setting_.srcHeight != initial_height, "Maximize did not resize the window");
        window->Restore();
        waitFor([&]{ return !window->IsMaximized(); });
        check("restored");
        require(setting_.srcWidth == initial_width && setting_.srcHeight == initial_height, "Restore did not recover the window size");
    }
    window->Minimize();
    waitFor([&]{ return window->Minimized(); });
    require((window->Minimized()) != 0, "Window did not minimize");
    auto const texture = game_view_color_tex_;
    // Exercise the same resize gate without drawing to an unavailable drawable.
    re.BeginFrame();
    re.EndFrame();
    require(game_view_color_tex_ == texture, "Minimize recreated Game texture");
    window->Restore();
    waitFor([&]{ return !window->Minimized(); });
    check("restore-after-minimize");
    window->SetContentSize(900, 650);
    Window::PumpEvents();
    check("smaller");
    require(setting_.srcWidth == 900 && setting_.srcHeight == 650, "Smaller size was not applied");
    window->SetVisible(false);
#endif
}
}
