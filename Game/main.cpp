#include <windows.h>
#include <windowsx.h> 
#include "Util/util_string.h"
#include "Container/cvar_list.h"
#include "Platform/Renderer.h"
#include "Platform/DxGraphDevice.h"
#include "Core/Context.h"
#include "Util/UtilTool.h"

const int g_height = 600;    // 显示框高
const int g_width = 800;	// 显示框宽
const bool g_Windows = true; // 是否是窗口模式
const char* szWinName = "Game 1.0"; // 窗口名字
char g_szWorkPath[1024] = { 0 };
HWND g_hwnd;
#define BOOST_DISABLE_ASSERTS

void get_work_path(char* szPath, char* szOldPath)
{
	char szPath1[1024] = { 0 };

	trim_string(szPath1, szOldPath);
	char szPath2[1024] = { 0 };
	uint32_t nSize = UtilString::length(szPath1);
	char* big2 = strrchr(szPath1, '\\');
	if (NULL == big2)
	{
		memcpy(szPath, szPath1, nSize);
		return;
	}
	nSize = static_cast<uint32_t>(big2 - szPath1);
	memcpy(szPath2, szPath1, nSize);

	char *big3 = strrchr(szPath2, '\\');
	if (NULL == big3)
	{
		memcpy(szPath, szPath2, nSize);
		return;
	}
	nSize = big3 - szPath2;
	memcpy(szPath, szPath2, nSize);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Context::Instance()->SetQuit();
		::PostQuitMessage(0);
		break;

	case WM_PAINT:
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			::DestroyWindow(hwnd);
		}
		break;
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	MEMORY_CHECK_LEAKAGE
	Renderer app;

	//初始化窗口
	WNDCLASS wc;
	wc.style = CS_DBLCLKS | CS_OWNDC |
		CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = ::LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = ::LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = 0;
	wc.lpszClassName = (szWinName);

	if (!::RegisterClass(&wc))
	{
		fm_log("register windows class fail!  function[%s]", "RegisterClass");
		return false;
	}

	int nRealWidth = g_width;
	int nRealHight = g_height;
	bool bRealWindows = g_Windows;
	char* szArgs = GetCommandLineA();
	get_work_path(g_szWorkPath, &szArgs[1]);


	DWORD dwStype = WS_POPUP | WS_VISIBLE;
	if (bRealWindows)
	{
		dwStype = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION; 
	}
	RECT  winRect;
	SetRect(&winRect, 0, 0, nRealWidth, nRealHight);
	AdjustWindowRectEx(&winRect, WS_EX_TOPMOST, false, dwStype);
	int x = winRect.right - winRect.left;
	int y = winRect.bottom - winRect.top;
	int sx = (GetSystemMetrics(SM_CXSCREEN) - x) / 2;
	int sy = (GetSystemMetrics(SM_CYSCREEN) - y) / 2;
	g_hwnd = CreateWindow((szWinName), ("Game"), dwStype,
		sx, sy, nRealWidth, nRealHight, 0, 0, hInstance, 0);
	if (!g_hwnd)
	{
		fm_log("create windows class fail! function[%s]", "CreateWindow");
		return false;
	}
	if (bRealWindows)
	{
		// 不知道为什么非要加这句话，要不让就不显示表面了
		RECT window_rect = { 0,0,nRealWidth - 1,nRealHight - 1 };
		AdjustWindowRectEx(&window_rect,
			GetWindowStyle(g_hwnd),
			GetMenu(g_hwnd) != NULL,
			GetWindowExStyle(g_hwnd));
		sx = -window_rect.left;
		sy = -window_rect.top;
	}
	::ShowWindow(g_hwnd, SW_SHOW);
	::UpdateWindow(g_hwnd);

	app.Init().Inilize(nRealWidth, nRealHight, g_szWorkPath, hInstance);
	auto pDevice = Context::Instance()->GetSubsystem<DxGraphDevice>();
	pDevice->InitDevice(g_hwnd, nRealWidth, nRealHight, sx, sy, bRealWindows);
	app.SetFont(pDevice);

	MSG msg;
	::ZeroMemory(&msg, sizeof(msg));
	app.StarTimer();
	while (true)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		app.Run();

		if (Context::Instance()->GetQuit())
		{
			break;
		}
	}

	Context::Instance()->Close();
	pDevice->ShutDown();
	return 0;
}