#include <editor/EditorManagerD3D11.h>
#include <base/Context.h>
#include <base/ResLoader.h>
#include <base/WinApp.h>
#include <world/World.h>

using namespace EditorWorker;
using namespace RenderWorker;

int main()
{
    Context::Instance().ResLoaderInstance().AddPath("D:\\git\\Game\\DemoEngine\\editor\\resource");

    std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
    Context::Instance().LoadConfig(cfg_path.c_str());
    auto cfg = Context::Instance().Config();

    WinAPP app;
    app.CreateAppWindow(cfg.graphics_cfg);
    Context::Instance().AppInstance(app);
    Context::Instance().WorldInstance().BeginWorld();

    app.Run();
    //EditorManagerD3D11 edtor;
    return 0;
}