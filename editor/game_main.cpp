#include <editor/GameApp.h>

#include <base/ZEngine.h>
#include <base/ResLoader.h>
#include <game/GameContext.h>

using namespace EditorWorker;
using namespace RenderWorker;
using namespace CommonWorker;

int main()
{
	std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
	Context::Instance().LoadConfig(cfg_path.c_str());

	auto& res_loader = Context::Instance().ResLoaderInstance();
	res_loader.AddPath("../../assets/config");
	res_loader.AddPath("../../assets/prefabs");
	res_loader.AddPath("../../assets/shaders");
	res_loader.AddPath("../../assets");
	res_loader.AddPath("../../assets/rmlui");

	auto app = MakeUniquePtr<GameApp>();
	app->Create();
	app->Run();
	app.reset();
	GameContext::Instance().Shutdown();
	Context::Destroy();
	return 0;
}
