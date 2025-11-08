#include <editor/EditorManagerD3D11.h>
#include <base/ZEngine.h>
#include <base/ResLoader.h>
#include <world/World.h>
#include <base/Window.h>
#include <dxgi1_3.h>    // 添加 DXGI 头文件

using namespace EditorWorker;
using namespace RenderWorker;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main()
{
    std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
    Context::Instance().LoadConfig(cfg_path.c_str());
    auto config = Context::Instance().Config();
    const uint32_t defaultHierarchyWidth = 150;
	const uint32_t defaultProjectHeight = 200;
	const uint32_t defaultInspectorWidth = 300;
	const uint32_t defaultMainBarHeight = 58;
    uint32_t hWidth = defaultHierarchyWidth;
    uint32_t pHeight = defaultProjectHeight;
    uint32_t iWidth = defaultInspectorWidth;
    uint32_t fullWidth = config.graphics_cfg.width + defaultHierarchyWidth + defaultInspectorWidth;
	uint32_t fullHeight = config.graphics_cfg.height + defaultProjectHeight + defaultMainBarHeight;
    config.graphics_cfg.width = fullWidth;
    config.graphics_cfg.height = fullHeight;
    Context::Instance().Config(config);

    auto app = MakeUniquePtr<EditorManagerD3D11>();
    app->Create();
    app->MainWnd()->BindMsgProc(ImGui_ImplWin32_WndProcHandler);

    // 这种方式获取的是屏幕的实际物理分辨率
    HDC hdc = GetDC(NULL);
    uint32_t screenResolutionX = GetDeviceCaps(hdc, DESKTOPHORZRES);
    uint32_t screenResolutionY = GetDeviceCaps(hdc, DESKTOPVERTRES);
    ReleaseDC(NULL, hdc);

    app->SetWindowSize(hWidth, pHeight, iWidth);
    app->Run();

    // 添加 DXGI 调试报告
#ifdef _DEBUG
    {
        IDXGIDebug1* debug = nullptr;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
        {
            debug->ReportLiveObjects(DXGI_DEBUG_ALL, 
                DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL));
            debug->Release();
        }
    }
#endif
    return 0;
}