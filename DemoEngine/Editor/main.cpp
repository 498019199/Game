#include <editor/EditorManagerD3D11.h>
#include <base/Context.h>
#include <base/ResLoader.h>
#include <base/App3D.h>
#include <world/World.h>

using namespace EditorWorker;
using namespace RenderWorker;

class EditorApp:public App3D
{
public:
    EditorApp()
        :App3D("Editor App")
    {
    }
};

int main()
{
    Context::Instance().ResLoaderInstance().AddPath("../../Assets");
    std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
    Context::Instance().LoadConfig(cfg_path.c_str());

    auto app = MakeUniquePtr<EditorApp>();
    app->Create();
    app->Run();

    EditorManagerD3D11 edtor;
    edtor.Init();
    edtor.SetWindowSize(200, 200, 200);

    app->Destroy();
    return 0;
}