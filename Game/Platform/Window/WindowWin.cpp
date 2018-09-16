#include "../Container/macro.h"

#ifdef STX_PLATFORM_WINDOWS_DESKTOP
#include "Window.h"
#include "../Tool/UtilString.h"

Window::Window(std::string const & name, const WindowDesc& settings, void* native_wnd)
	:m_bActive(false), m_bReady(false), m_bClosed(false), m_bKeepScreenOn(settings.bKeepScreenOn),
	m_fDpiScale(1),m_bHide(settings.bHideWin)
{
#if STX_PLATFORM_WIN64
	m_swcName = UtilString::as_wstring(name.c_str());
#else
	m_swcName = name;
#endif
	
	if (native_wnd != nullptr)
	{
		m_Hwnd = static_cast<HWND>(native_wnd);
		m_DefaultWndProc = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(m_Hwnd, GWLP_WNDPROC));
		::SetWindowLongPtr(m_Hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
		m_bExternalWnd = true;
	}
	else
	{
		HINSTANCE hInst = ::GetModuleHandle(nullptr);

		// Register the window class
		WNDCLASSEX wc;
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(this);
		wc.hInstance = hInst;
		wc.hIcon = nullptr;
		wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = m_swcName.c_str();
		wc.hIconSm = nullptr;
		::RegisterClassEx(&wc);

		if (settings.bFullScreen)
		{
			m_WinStype = WS_POPUP;
		}
		else
		{
			m_WinStype = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, static_cast<LONG>(settings.nWidth * m_fDpiScale + 0.5f), static_cast<LONG>(settings.nHeight * m_fDpiScale + 0.5f) };
		::AdjustWindowRect(&rc, m_WinStype, false);

		// Create our main window
		m_Hwnd = ::CreateWindow(m_swcName.c_str(), m_swcName.c_str(), m_WinStype, settings.nLeft, settings.nTop,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, nullptr);
		m_DefaultWndProc = ::DefWindowProc;
		m_bExternalWnd = false;
	}

	RECT rc;
	::GetClientRect(m_Hwnd, &rc);
	m_nLeft = rc.left;
	m_nTop = rc.top;
	m_nWidth = rc.right - rc.left;
	m_Height = rc.bottom - rc.top;
	::SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	::ShowWindow(m_Hwnd, m_bHide ? SW_HIDE : SW_SHOWNORMAL);
	::UpdateWindow(m_Hwnd);

	m_bReady = true;
}

Window::~Window()
{
	if (m_bKeepScreenOn)
	{
#if defined(STX_PLATFORM_WINDOWS_DESKTOP)
		::SetThreadExecutionState(ES_CONTINUOUS);
#endif
	}

	if (m_Hwnd != nullptr)
	{
		::SetWindowLongPtr(m_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
		if (!m_bExternalWnd)
		{
			::DestroyWindow(m_Hwnd);
		}

		m_Hwnd = nullptr;
	}
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* win = reinterpret_cast<Window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (win != nullptr)
	{
		return win->MsgProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT Window::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		m_bActive = (WA_INACTIVE != LOWORD(wParam));
		this->OnActive()(*this, m_bActive);
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_CHAR:
		this->OnChar()(*this, static_cast<wchar_t>(wParam));
		break;

	case WM_INPUT:
		this->OnRawInput()(*this, reinterpret_cast<HRAWINPUT>(lParam));
		break;
	}

	return m_DefaultWndProc(hWnd, uMsg, wParam, lParam);
}
#endif


