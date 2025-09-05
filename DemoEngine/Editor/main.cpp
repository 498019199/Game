#include <editor/EditorManagerD3D11.h>
#include <base/Context.h>
#include <base/ResLoader.h>

#include <world/World.h>

using namespace EditorWorker;
using namespace RenderWorker;

int main()
{
    Context::Instance().ResLoaderInstance().AddPath("../../Assets");
    std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
    Context::Instance().LoadConfig(cfg_path.c_str());

    auto app = MakeUniquePtr<EditorManagerD3D11>();
    app->Create();
    app->SetWindowSize(200, 200, 200);
    app->Run();

    return 0;
}