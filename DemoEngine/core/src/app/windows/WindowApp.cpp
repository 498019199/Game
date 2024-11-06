#include "WindowApp.h"
#include <windowsx.h>
#include <core/IContext.h>

namespace CoreWorker
{
WindowApp::WindowApp(std::string const& strName, const FWindowDesc& Settings, void* pNativeWnd)
{
	CommonWorker::Convert(swcName_, strName);
	keep_screen_on_ = Settings.keep_screen_on;
	hide_ = Settings.hide_win;

	if (pNativeWnd != nullptr)
	{
		Hwnd_ = static_cast<HWND>(pNativeWnd);
		DefaultWndProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(Hwnd_, GWLP_WNDPROC));
		::SetWindowLongPtr(Hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
		bExternalWnd_ = true;
	}
	else
	{
		HINSTANCE hInst = ::GetModuleHandle(nullptr);

		// Register the window class
		WNDCLASSEXW wc;
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
		wc.lpszClassName = swcName_.c_str();
		wc.hIconSm = nullptr;
		::RegisterClassExW(&wc);

		if (Settings.full_screen)
		{
			WinStype_ = WS_POPUP;
		}
		else
		{
			WinStype_ = WS_OVERLAPPEDWINDOW;
		}

		RECT rc = { 0, 0, static_cast<LONG>(Settings.width * dpi_scale_ + 0.5f), static_cast<LONG>(Settings.height * dpi_scale_ + 0.5f) };
		::AdjustWindowRect(&rc, WinStype_, false);

		// Create our main window
		Hwnd_ = ::CreateWindowW(swcName_.c_str(), swcName_.c_str(), WinStype_, 
            Settings.left, Settings.top, rc.right - rc.left, rc.bottom - rc.top, 0, 0, hInst, nullptr);
		DefaultWndProc_ = ::DefWindowProc;
		bExternalWnd_ = false;
	}

	RECT window_rect;
	::GetClientRect(Hwnd_, &window_rect);
	AdjustWindowRectEx(&window_rect,
		GetWindowStyle(Hwnd_),
		GetMenu(Hwnd_) != NULL,
		GetWindowExStyle(Hwnd_));
	left_ = -window_rect.left;
	top_ = -window_rect.top;
	width_ = window_rect.right - window_rect.left;
	nHeight_ = window_rect.bottom - window_rect.top;

	::SetWindowLongPtr(Hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	::ShowWindow(Hwnd_, hide_ ? SW_HIDE : SW_SHOWNORMAL);
	::UpdateWindow(Hwnd_);

	ready_ = true;
}

WindowApp::~WindowApp()
{
	if (keep_screen_on_)
	{
		::SetThreadExecutionState(ES_CONTINUOUS);
	}

	if (Hwnd_ != nullptr)
	{
		::SetWindowLongPtr(Hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
		if (!bExternalWnd_)
		{
			::DestroyWindow(Hwnd_);
		}

		Hwnd_ = nullptr;
	}
}

void WindowApp::Create()
{
	auto config = Context::Instance()->GetConfig();
	auto d3d_re = Context::Instance()->GetRenderDevice();
    if(d3d_re)
    {
        d3d_re->CreateRenderWindow(config.title, config.win_desc);
    }
}

void WindowApp::Run()
{
	auto d3d_re = Context::Instance()->GetRenderDevice();
    bool gotMsg;
    MSG  msg;
    ::PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
    while (WM_QUIT != msg.message)
    {
        // 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
        // 不然, 用 GetMessage() 减少 CPU 占用率
        if (active_)
        {
            gotMsg = (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) != 0);
        }
        else
        {
            gotMsg = (::GetMessage(&msg, nullptr, 0, 0) != 0);
        }

        if (gotMsg)
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            d3d_re->Refresh();
        }
    }
}

void WindowApp::Close()
{
    
}

LRESULT CALLBACK WindowApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WindowApp* win = reinterpret_cast<WindowApp*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (win != nullptr)
	{
		return win->MsgProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void WindowApp::UpdateDpiScale(float scale)
{
    dpi_scale_ = scale;
}

LRESULT WindowApp::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		active_ = (WA_INACTIVE != LOWORD(wParam));
		//this->OnActive()(*this, m_bActive);
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_CHAR:
		//this->OnChar()(*this, static_cast<wchar_t>(wParam));
		break;

	case WM_SIZE:
	{
		RECT rc;
		::GetClientRect(Hwnd_, &rc);
		AdjustWindowRectEx(&rc,
			GetWindowStyle(Hwnd_),
			GetMenu(Hwnd_) != NULL,
			GetWindowExStyle(Hwnd_));
		left_ = -rc.left;
		top_ = -rc.top;
		width_ = rc.right - rc.left;
		nHeight_ = rc.bottom - rc.top;

		// Check to see if we are losing or gaining our window.  Set the
		// active flag to match
		if ((SIZE_MAXHIDE == wParam) || (SIZE_MINIMIZED == wParam))
		{
			active_ = false;
			//this->OnSize()(*this, false);
		}
		else
		{
			active_ = true;
			//this->OnSize()(*this, true);
		}
	}
	break;

	// 键盘
	case WM_INPUT:
		//this->OnRawInput()(*this, reinterpret_cast<HRAWINPUT>(lParam));
	break;

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
	case WM_POINTERDOWN:
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(this->GetHWnd(), &pt);
		//this->OnPointerDown()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam));
	}
	break;

	case WM_POINTERUP:
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(this->GetHWnd(), &pt);
		//this->OnPointerUp()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam));
	}
	break;

	case WM_POINTERUPDATE:
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(this->GetHWnd(), &pt);
		// this->OnPointerUpdate()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam),
		// 	IS_POINTER_INCONTACT_WPARAM(wParam));
	}
	break;

	case WM_POINTERWHEEL:
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		::ScreenToClient(this->GetHWnd(), &pt);
		// this->OnPointerWheel()(*this, int2(pt.x, pt.y), GET_POINTERID_WPARAM(wParam),
		// 	GET_WHEEL_DELTA_WPARAM(wParam));
	}
	break;

	case WM_DPICHANGED:
	{
		RECT rc;
		::GetClientRect(Hwnd_, &rc);
		rc.left = static_cast<int32_t>(rc.left / dpi_scale_ + 0.5f);
		rc.top = static_cast<int32_t>(rc.top / dpi_scale_ + 0.5f);
		rc.right = static_cast<uint32_t>(rc.right / dpi_scale_ + 0.5f);
		rc.bottom = static_cast<uint32_t>(rc.bottom / dpi_scale_ + 0.5f);

		this->UpdateDpiScale(static_cast<float>(HIWORD(wParam)) / USER_DEFAULT_SCREEN_DPI);

		rc.left = static_cast<int32_t>(rc.left * dpi_scale_ + 0.5f);
		rc.top = static_cast<int32_t>(rc.top * dpi_scale_ + 0.5f);
		rc.right = static_cast<uint32_t>(rc.right * dpi_scale_ + 0.5f);
		rc.bottom = static_cast<uint32_t>(rc.bottom * dpi_scale_ + 0.5f);

		::AdjustWindowRect(&rc, WinStype_, false);
		::SetWindowPos(this->GetHWnd(), HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION);
	}
	break;

	case WM_CLOSE:
		//Context::Instance()->Close();
        active_ = false;
        ready_ = false;
        closed_ = true;
        ::PostQuitMessage(0);
		break;
#endif
	}

	return DefaultWndProc_(hWnd, uMsg, wParam, lParam);
}

}