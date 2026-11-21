#include <editor/EditorManager.h>
#include <base/DesktopBackend.h>
#include <base/ZEngine.h>
#include <base/ResLoader.h>
#include <base/Window.h>
#include <world/World.h>
#include <base/UIManager.h>
#include <game/GameContext.h>
#include <filesystem>
#include <iostream>

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
#include <editor/EditorManagerD3D11.h>
#include <editor/EditorManagerD3D12.h>
#include <windows.h>
#else
#include <editor/EditorManagerMetal.h>
#include <GLFW/glfw3.h>
#endif

using namespace EditorWorker;
using namespace RenderWorker;
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP

namespace
{
    bool EnableDpiAwareness()
    {
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
        HMODULE const user32 = ::LoadLibraryW(L"user32.dll");
        if (!user32)
        {
            return false;
        }

        typedef BOOL (WINAPI *SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT value);
        auto* set_process_dpi_awareness_context =
            reinterpret_cast<SetProcessDpiAwarenessContextFunc>(::GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
        if (set_process_dpi_awareness_context)
        {
            set_process_dpi_awareness_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        ::FreeLibrary(user32);
        return set_process_dpi_awareness_context != nullptr;
#else
        return false;
#endif
    }

    uint32_t To96DpiUnits(int value, int dpi)
    {
        if (value <= 0)
        {
            return 0;
        }
        if (dpi <= 0)
        {
            return static_cast<uint32_t>(value);
        }
        return static_cast<uint32_t>(static_cast<float>(value) * USER_DEFAULT_SCREEN_DPI / dpi + 0.5f);
    }
}

#endif

int main(int argc, char** argv)
{
    std::string backend, smoke_material;
    bool smoke_resize = false, smoke_material_binding = false;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg.starts_with("--backend=")) backend = arg.substr(10);
        else if (arg.starts_with("--smoke-material=")) smoke_material = arg.substr(17);
        else if (arg == "--smoke-resize") smoke_resize = true;
        else if (arg == "--smoke-material-binding") smoke_material_binding = true;
        else { std::cerr << "Unknown argument: " << arg << std::endl; return 2; }
    }
    if (!backend.empty() && backend != DesktopRendererName
#ifdef _WIN32
        && backend != "D3D11"
#endif
    ) { std::cerr << "Unsupported render backend: " << backend << std::endl; return 2; }
    if ((smoke_material_binding || smoke_resize) && smoke_material.empty()) smoke_material = "materials/m2demo/Seed.mat";
    std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
    Context::Instance().LoadConfig(cfg_path.c_str());
    auto config = Context::Instance().Config();
#ifndef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    config.render_factory_name = DesktopRendererName;
#endif
    if (!backend.empty()) config.render_factory_name = backend;
    const uint32_t defaultHierarchyWidth = 150;
	const uint32_t defaultProjectHeight = 200;
	const uint32_t defaultInspectorWidth = 300;
	const uint32_t defaultMainBarHeight = 58;
    // KlayGE.cfg describes the complete Editor client area. The game view is
    // the space left after the surrounding Editor panels are laid out.
    uint32_t editorWidth = config.graphics_cfg.width;
    uint32_t editorHeight = config.graphics_cfg.height;
    uint32_t hWidth = defaultHierarchyWidth;
    uint32_t pHeight = defaultProjectHeight;
    uint32_t iWidth = defaultInspectorWidth;

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    bool const dpi_aware = EnableDpiAwareness();
    RECT available_area = {};
    if (config.graphics_cfg.full_screen)
    {
        available_area.right = ::GetSystemMetrics(SM_CXSCREEN);
        available_area.bottom = ::GetSystemMetrics(SM_CYSCREEN);
    }
    else if (!::SystemParametersInfoW(SPI_GETWORKAREA, 0, &available_area, 0))
    {
        available_area.right = ::GetSystemMetrics(SM_CXSCREEN);
        available_area.bottom = ::GetSystemMetrics(SM_CYSCREEN);
    }

    HDC hdc = GetDC(NULL);
    int const dpi_x = dpi_aware ? GetDeviceCaps(hdc, LOGPIXELSX) : USER_DEFAULT_SCREEN_DPI;
    int const dpi_y = dpi_aware ? GetDeviceCaps(hdc, LOGPIXELSY) : USER_DEFAULT_SCREEN_DPI;
    ReleaseDC(NULL, hdc);
    uint32_t const screenResolutionX = To96DpiUnits(available_area.right - available_area.left, dpi_x);
    uint32_t const screenResolutionY = To96DpiUnits(available_area.bottom - available_area.top, dpi_y);
#else
    Window::InitializeDesktop();
    auto* display = glfwGetPrimaryMonitor();
    int x=0, y=0, w=0, h=0;
    if (display) {
        glfwGetMonitorWorkarea(display, &x, &y, &w, &h);
        if (config.graphics_cfg.full_screen) {
            if (auto mode=glfwGetVideoMode(display)) { w=mode->width; h=mode->height; }
        }
    }
    uint32_t const screenResolutionX = w>0 ? static_cast<uint32_t>(w) : editorWidth;
    uint32_t const screenResolutionY = h>0 ? static_cast<uint32_t>(h) : editorHeight;
#endif

    if (config.graphics_cfg.full_screen)
    {
        // A borderless full-screen window has the same size as its client area.
        editorWidth = screenResolutionX;
        editorHeight = screenResolutionY;
    }
    // Editor 客户区宽度已经超过工作区，自适应缩小
    else if (editorWidth > screenResolutionX && screenResolutionX > 0)
    {
        float scaleRatio = static_cast<float>(screenResolutionX) / static_cast<float>(editorWidth);
        hWidth = static_cast<uint32_t>(static_cast<float>(hWidth) * scaleRatio);
        iWidth = static_cast<uint32_t>(static_cast<float>(iWidth) * scaleRatio);
        editorWidth = screenResolutionX;
    }
    // Editor 客户区高度已经超过工作区，自适应缩小
    if (!config.graphics_cfg.full_screen && editorHeight > screenResolutionY && screenResolutionY > defaultMainBarHeight)
    {
        float scaleRatio = static_cast<float>(screenResolutionY - defaultMainBarHeight)
            / static_cast<float>(editorHeight - defaultMainBarHeight);
        pHeight = static_cast<uint32_t>(static_cast<float>(pHeight) * scaleRatio);
        editorHeight = screenResolutionY;
    }

    EditorSetting setting;
    setting.SetWindowSize(editorWidth, editorHeight, hWidth, pHeight, iWidth);
    config.graphics_cfg.width = setting.srcWidth;
    config.graphics_cfg.height = setting.srcHeight;
    Context::Instance().Config(config);

    auto& res_loader = Context::Instance().ResLoaderInstance();
    res_loader.AddPath("../../assets/config");
    res_loader.AddPath("../../assets/prefabs");
    res_loader.AddPath("../../assets/shaders");
    res_loader.AddPath("../../assets");
    res_loader.AddPath("../../assets/rmlui");

    std::unique_ptr<EditorManager> app;
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    if (config.render_factory_name == "D3D11")
        app = MakeUniquePtr<EditorManagerD3D11>();
    else
        app = MakeUniquePtr<EditorManagerD3D12>();
#else
    app = MakeUniquePtr<EditorManagerMetal>();
#endif
    app->GetEditorSetting( setting );
    app->Create();


    int result = 0;
    if (smoke_material.empty() && !smoke_resize) app->Run();
    else {
        // Bounded rendering tests. Scene bindings change only in memory; no asset writes.
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
        ::ShowWindow(app->MainWnd()->GetHWND(), SW_HIDE);
#else
        glfwHideWindow(app->MainWnd()->DesktopWindow());
#endif
        try {
            if (!smoke_material.empty()) app->SelectMaterialDocument(app->Materials().Open(smoke_material));
#if defined(ZENGINE_PLATFORM_WINDOWS_DESKTOP) || defined(ZENGINE_PLATFORM_DARWIN)
            if (smoke_resize) app->RunResizeSmokeTest();
#endif
            auto render_frames = [] { for (int i = 0; i < 3; ++i) Context::Instance().WorldInstance().Update(); };
            render_frames();
            if (smoke_material_binding) {
                RenderModelPtr model;
                Context::Instance().WorldInstance().SceneRootNode().Traverse([&](SceneNode& node) {
                    if (!model) {
                        auto candidate = app->Scene().FindModelForNode(node);
                        if (candidate && candidate->NumMaterials() && app->Scene().ContainsModel(candidate)) model = candidate;
                    }
                    return true;
                });
                if (!model) throw std::runtime_error("Material binding smoke needs a scene model with a material slot");
                auto original = model->GetMaterial(0);
                auto document = app->Materials().Open(smoke_material);
                if (original->EffectName() == document->Material().EffectName())
                    throw std::runtime_error("Material binding smoke needs two different shader templates");
                // Warm the original buffers, then alternate both shader directions.
                // This exercises effect lifetime and the real World render queue.
                for (int cycle = 0; cycle < 3; ++cycle) {
                    app->Scene().AssignMaterial(model, 0, document->Path(), document->Material().Clone());
                    render_frames();
                    model->GetMaterial(0) = original;
                    model->ForEachMesh([&](Renderable& renderable) {
                        if (auto* mesh = dynamic_cast<StaticMesh*>(&renderable); mesh && mesh->MaterialID() == 0)
                            mesh->Material(original);
                    });
                    render_frames();
                }
                std::cout << "PASS: 6 material shader switches rendered (" << config.render_factory_name << ")" << std::endl;
                auto saved = document->Parameters();
                auto edited = saved;
                edited.metalness = saved.metalness < 0.5f ? 0.9f : 0.1f;
                document->Edit(edited);
                app->Scene().AssignMaterial(model, 0, document->Path(), document->Material().Clone());
                render_frames();
                if (model->GetMaterial(0)->Metalness() != edited.metalness)
                    throw std::runtime_error("Edited scene material was not synchronized");
                if (app->CloseMaterialDocument(document, MaterialCloseChoice::Cancel) ||
                    app->PreviewDocument() != document || !document->Dirty())
                    throw std::runtime_error("Cancel close changed the active document");
                if (!app->CloseMaterialDocument(document, MaterialCloseChoice::Discard))
                    throw std::runtime_error("Discard close did not close the document");
                render_frames();
                if (app->PreviewDocument() || app->Materials().Documents().count(document->Path()) ||
                    model->GetMaterial(0)->Metalness() != saved.metalness)
                    throw std::runtime_error("Discard close left a stale preview or scene material");
                app->SelectMaterialDocument(app->Materials().Open(smoke_material));
                render_frames();
                std::cout << "PASS: material close cancel/discard, scene restore, preview release and reopen" << std::endl;

            } else {
                std::cout << "PASS: Editor material Inspector rendered 3 frames (" << config.render_factory_name << ")" << std::endl;
            }
        } catch (std::exception const& e) { std::cerr << e.what() << std::endl; result = 1; }
        app->OnDestroy();
    }
    app.reset();
    GameContext::Instance().Shutdown();
    Context::Destroy();
    return result;
}
