#include "../Core/ClientPlayer.h"
#include "Util/util_string.h"
#include "Container/cvar_list.h"
#include "Platform/Renderer.h"
#include "Platform/DxGraphDevice.h"
#include "Platform/Window/Window.h"
#include "Platform/MsgInput/MInput.hpp"
#include "Core/Context.h"
#include "Util/UtilTool.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MEMORY_CHECK_LEAKAGE
	// �����ߣ��Ƿ�ȫ��������
	auto pCore = InitCore(CVarList() << 800 << 600 << false << "config.xml");
	auto player = MakeSharedPtr<ClientPlayer>();
	InitCoreList(pCore);

	player->Create();
	auto win = player->GetMainWin();
	auto pDevice = Context::Instance()->GetSubsystem<DxGraphDevice>();
	pDevice->InitDevice(win->GetHWnd(), win->Width(), win->Height(), win->Top(), win->Left(), win->Windows());
	auto render = Context::Instance()->GetSubsystem<Renderer>();
	render->SetFont(pDevice);

	player->Run();
	return 0;
}