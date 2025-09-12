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

    auto app = MakeUniquePtr<EditorManagerD3D11>();
    app->Create();
    app->MainWnd()->BindMsgProc(ImGui_ImplWin32_WndProcHandler);
    app->SetWindowSize(200, 200, 200);
    app->Run();

    return 0;
}