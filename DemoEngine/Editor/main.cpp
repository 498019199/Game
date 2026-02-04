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
    uint32_t srcWidth = config.graphics_cfg.width;
	uint32_t srcHeight = config.graphics_cfg.height;
    uint32_t fullWidth = srcWidth + defaultHierarchyWidth + defaultInspectorWidth;
	uint32_t fullHeight = srcHeight + defaultProjectHeight + defaultMainBarHeight;
    uint32_t hWidth = defaultHierarchyWidth;
    uint32_t pHeight = defaultProjectHeight;
    uint32_t iWidth = defaultInspectorWidth;

    //这种方式获取的是屏幕的实际物理分辨率
    HDC hdc = GetDC(NULL);
    uint32_t screenResolutionX = GetDeviceCaps(hdc, DESKTOPHORZRES);
    uint32_t screenResolutionY = GetDeviceCaps(hdc, DESKTOPVERTRES);
    ReleaseDC(NULL, hdc);

    // 宽度已经超过了屏幕分辨率，自适应缩小
    if (fullWidth > screenResolutionX && screenResolutionX > 0)
    {
        float scaleRatio = static_cast<float>(screenResolutionX) / static_cast<float>(fullWidth);
        hWidth = static_cast<uint32_t>(static_cast<float>(hWidth) * scaleRatio);
        iWidth = static_cast<uint32_t>(static_cast<float>(iWidth) * scaleRatio);
        srcWidth = static_cast<uint32_t>(static_cast<float>(srcWidth) * scaleRatio);
    }
    // 高度已经超过了屏幕分辨率，自适应缩小
    if (fullHeight > screenResolutionY && screenResolutionY > 0)
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

    auto app = MakeUniquePtr<EditorManagerD3D11>();
    app->GetEditorSetting( setting );
    app->Create();
    app->MainWnd()->BindMsgProc(ImGui_ImplWin32_WndProcHandler);
    app->Run();
    return 0;
}