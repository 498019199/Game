#include "Core/App.h"
#include "Util/util_string.h"
#include "Container/cvar_list.h"
#include "Platform/Renderer.h"
#include "Platform/DxGraphDevice.h"
#include "Platform/Window/Window.h"
#include "Core/Context.h"
#include "Util/UtilTool.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MEMORY_CHECK_LEAKAGE
	// øÌ£¨∏ﬂ£¨ «∑Ò»´∆¡£¨≈‰÷√
	auto pCore = InitCore(CVarList() << 800 << 600 << false << "config.xml");
	auto app = MakeSharedPtr<App>("Game 2.0", nullptr);
	app->Create();
	
	InitCoreList(pCore);
	auto win = app->GetMainWin();
	auto pDevice = Context::Instance()->GetSubsystem<DxGraphDevice>();
	pDevice->InitDevice(win->GetHWnd(), win->Width(), win->Height(), win->Top(), win->Left(), win->Windows());
	auto render = Context::Instance()->GetSubsystem<Renderer>();
	render->Inilize(win->GetHWnd(), hInstance);
	render->SetFont(pDevice);
	pCore->Test();
	app->Run();
	return 0;
}