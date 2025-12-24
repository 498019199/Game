#include <editor/EditorManagerD3D11.h>
#include <base/ZEngine.h>
#include <base/ResLoader.h>
#include <world/World.h>
#include <base/Window.h>

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
    uint32_t fullWidth = config.graphics_cfg.width + defaultHierarchyWidth + defaultInspectorWidth;
	uint32_t fullHeight = config.graphics_cfg.height + defaultProjectHeight + defaultMainBarHeight;

    uint32_t hWidth = defaultHierarchyWidth;
    uint32_t pHeight = defaultProjectHeight;
    uint32_t iWidth = defaultInspectorWidth;
    config.graphics_cfg.width = fullWidth;
    config.graphics_cfg.height = fullHeight;
    Context::Instance().Config(config);

    auto app = MakeUniquePtr<EditorManagerD3D11>();
    app->Create();
    app->MainWnd()->BindMsgProc(ImGui_ImplWin32_WndProcHandler);

    // 这种方式获取的是屏幕的实际物理分辨率
    //HDC hdc = GetDC(NULL);
    //uint32_t screenResolutionX = GetDeviceCaps(hdc, DESKTOPHORZRES);
    //uint32_t screenResolutionY = GetDeviceCaps(hdc, DESKTOPVERTRES);
    //ReleaseDC(NULL, hdc);

    app->SetWindowSize(hWidth, pHeight, iWidth);
    app->Run();
    return 0;
}