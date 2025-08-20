#include <editor/EditorManagerD3D11.h>
#include <base/Context.h>
#include <base/WinApp.h>
#include <world/World.h>

using namespace EditorWorker;
using namespace RenderWorker;

int main()
{
    Context::Instance().LoadConfig("KlayGE.cfg");
    WinAPP app;
    app.CreateAppWindow(Context::Instance().Config().graphics_cfg);
    Context::Instance().AppInstance(app);
    Context::Instance().WorldInstance().BeginWorld();

    EditorManagerD3D11 edtor;
    return 0;
}