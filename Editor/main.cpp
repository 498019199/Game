#include <editor/EditorManager.h>
#include <editor/EditorManagerD3D11.h>
#include <editor/EditorManagerSDL3.h>
#include <base/ZEngine.h>
#include <base/ResLoader.h>
#include <base/Window.h>
#include <world/World.h>
#include <base/UIManager.h>
#include <game/GameContext.h>
#include <filesystem>

#include <windows.h>

using namespace EditorWorker;
using namespace RenderWorker;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	LRESULT CALLBACK EditorWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
#ifndef EDITOR_DEBUG_MODE
		// SDL3 owns ImGui input when the SDL_GPU renderer is active. The legacy
		// Win32 backend must not also consume the same native messages, otherwise
		// keyboard and mouse events are submitted twice.
		if (Context::Instance().RenderFactoryValid())
		{
			CommonWorker::checked_cast<EditorManager&>(Context::Instance().AppInstance()).ProcessWindowMessage(hWnd, msg, wParam, lParam);
		}
#endif
		return -1;
	}

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

int main()
{
    std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
    Context::Instance().LoadConfig(cfg_path.c_str());
    auto config = Context::Instance().Config();
    const uint32_t defaultHierarchyWidth = 150;
	const uint32_t defaultProjectHeight = 200;
	const uint32_t defaultInspectorWidth = 300;
	const uint32_t defaultMainBarHeight = 58;
    uint32_t srcWidth = config.graphics_cfg.width;
	uint32_t srcHeight = config.graphics_cfg.height;
    uint32_t fullWidth = srcWidth + defaultHierarchyWidth + defaultInspectorWidth;
	uint32_t fullHeight = srcHeight + defaultProjectHeight + defaultMainBarHeight;
    uint32_t hWidth = defaultHierarchyWidth;
    uint32_t pHeight = defaultProjectHeight;
    uint32_t iWidth = defaultInspectorWidth;

    bool const dpi_aware = EnableDpiAwareness();
    RECT work_area = {};
    if (!::SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0))
    {
        work_area.right = ::GetSystemMetrics(SM_CXSCREEN);
        work_area.bottom = ::GetSystemMetrics(SM_CYSCREEN);
    }

    HDC hdc = GetDC(NULL);
    int const dpi_x = dpi_aware ? GetDeviceCaps(hdc, LOGPIXELSX) : USER_DEFAULT_SCREEN_DPI;
    int const dpi_y = dpi_aware ? GetDeviceCaps(hdc, LOGPIXELSY) : USER_DEFAULT_SCREEN_DPI;
    ReleaseDC(NULL, hdc);
    uint32_t const screenResolutionX = To96DpiUnits(work_area.right - work_area.left, dpi_x);
    uint32_t const screenResolutionY = To96DpiUnits(work_area.bottom - work_area.top, dpi_y);

    // 宽度已经超过了屏幕分辨率，自适应缩小
    if (fullWidth > screenResolutionX && screenResolutionX > 0)
    {
        float scaleRatio = static_cast<float>(screenResolutionX) / static_cast<float>(fullWidth);
        hWidth = static_cast<uint32_t>(static_cast<float>(hWidth) * scaleRatio);
        iWidth = static_cast<uint32_t>(static_cast<float>(iWidth) * scaleRatio);
        srcWidth = static_cast<uint32_t>(static_cast<float>(srcWidth) * scaleRatio);
    }
    // 高度已经超过了屏幕分辨率，自适应缩小
    if (fullHeight > screenResolutionY && screenResolutionY > defaultMainBarHeight)
    {
        float scaleRatio = static_cast<float>(screenResolutionY - defaultMainBarHeight) / static_cast<float>(fullHeight - defaultMainBarHeight);
        pHeight = static_cast<uint32_t>(static_cast<float>(pHeight) * scaleRatio);
        srcHeight = static_cast<uint32_t>(static_cast<float>(srcHeight) * scaleRatio);
    }

    EditorSetting setting;
    setting.SetWindowSize(srcWidth, srcHeight, hWidth, pHeight, iWidth);
    config.graphics_cfg.width = setting.srcWidth;
    config.graphics_cfg.height = setting.srcHeight;
    Context::Instance().Config(config);

    auto& res_loader = Context::Instance().ResLoaderInstance();
    res_loader.AddPath("../../Assets/Config");
    res_loader.AddPath("../../Assets/Prefabs");
    res_loader.AddPath("../../Assets/Shaders");
    res_loader.AddPath("../../Assets");
    res_loader.AddPath("../../Assets/rmlui");

    std::unique_ptr<EditorManager> app;
    if (config.render_factory_name == "SDL3")
    {
        app = MakeUniquePtr<EditorManagerSDL3>();
    }
    else
    {
        app = MakeUniquePtr<EditorManagerD3D11>();
    }
    app->GetEditorSetting( setting );
    app->Create();
#ifndef EDITOR_DEBUG_MODE
    app->MainWnd()->BindMsgProc(EditorWndProc);
#endif //EDITOR_DEBUG_MODE

    app->Run();
    app.reset();
    GameContext::Instance().Shutdown();
    Context::Destroy();
    return 0;
}
