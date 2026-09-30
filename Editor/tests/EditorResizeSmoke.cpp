#include <editor/EditorManager.h>
#include <base/Window.h>
#include <world/World.h>
#include <render/RenderEngine.h>
#include <imgui_internal.h>
#include <common/ErrorHandling.h>
#include <iostream>
#include <cmath>
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
    auto const hwnd = MainWnd()->GetHWND();
    auto const selection = selected_asset_info_;
    auto material = std::dynamic_pointer_cast<AssetMaterialInfo>(selection);
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
        for (int i = 0; i < 3; ++i) Context::Instance().WorldInstance().Update();
        RECT client{};
        require(::GetClientRect(hwnd, &client) != 0, "GetClientRect failed");
        auto const& screen = re.ScreenFrameBuffer();
        require(setting_.srcWidth == client.right && setting_.srcHeight == client.bottom, "Stale Editor layout");
        require(screen->Width() == client.right && screen->Height() == client.bottom, "Stale screen framebuffer");
        require(screen->Viewport()->Width() == client.right && screen->Viewport()->Height() == client.bottom, "Stale viewport");
        if (auto const& depth = screen->AttachedDsv())
            require(depth->Width() == client.right && depth->Height() == client.bottom, "Stale screen depth buffer");
        if (auto const& color = screen->AttachedRtv(RenderWorker::FrameBuffer::Attachment::Color0))
            require(color->Width() == client.right && color->Height() == client.bottom, "Stale swapchain backbuffer");
        require(game_view_color_tex_->Width(0) == setting_.gameViewWidth && game_view_color_tex_->Height(0) == setting_.gameViewHeight,
            "Stale Game view texture");
        require(back_face_depth_fb_->Width() == setting_.gameViewWidth && back_face_depth_fb_->Height() == setting_.gameViewHeight,
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
        std::cout << "PASS resize " << phase << ": " << client.right << 'x' << client.bottom << std::endl;
    };
    // Test only this process's own native window. No asset writes or input injection.
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
}
}
