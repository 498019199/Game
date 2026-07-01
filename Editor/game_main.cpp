#include <editor/GameApp.h>

#include <base/ZEngine.h>
#include <base/ResLoader.h>

using namespace EditorWorker;
using namespace RenderWorker;
using namespace CommonWorker;

int main()
{
	std::string cfg_path = Context::Instance().ResLoaderInstance().Locate("KlayGE.cfg");
	Context::Instance().LoadConfig(cfg_path.c_str());

	auto app = MakeUniquePtr<GameApp>();
	app->Create();
	app->Run();
	return 0;
}
