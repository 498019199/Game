#include <base/Window.h>
#include <base/Context.h>
#include <world/World.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    Window* win = reinterpret_cast<Window*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    if (win != nullptr)
    {
        return win->MsgProc(hWnd, uMsg, wParam, lParam);
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Window::Window(const std::string& name, const RenderSettings& settings, void* native_wnd)
    :keep_screen_on_(settings.keep_screen_on), hide_(settings.hide_win)
{
    this->DetectsDpi();
    this->KeepScreenOn();
	Convert(wname_, name);

    if(nullptr != native_wnd)
    {

    }
    else
	{    
		HINSTANCE hInst = ::GetModuleHandle(nullptr);
		std::wstring  wname = wname_;
		WNDCLASSEXW wc;
		wc.cbSize = sizeof(wc);
		wc.style         = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc   = WndProc; 
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = sizeof(this);
		wc.hInstance     = hInst;
		wc.hIcon         = nullptr;
		wc.hCursor       = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName  = nullptr;
		wc.hIconSm = nullptr;
		wc.lpszClassName = wname.c_str();
		if( !::RegisterClassExW(&wc) )
		{
			// 获取错误码
			::MessageBoxW(0, L"RegisterClass Failed.", 0, 0);
		}

		if (settings.full_screen)
		{
			win_style_ = WS_POPUP;
		}
		else
		{
			win_style_ = WS_OVERLAPPEDWINDOW;
		}

		// Compute window rectangle dimensions based on requested client area dimensions.
		RECT rc = { 0, 0, static_cast<LONG>(settings.width * dpi_scale_ + 0.5f), static_cast<LONG>(settings.height * dpi_scale_ + 0.5f) };
		::AdjustWindowRect(&rc, win_style_, false);

        // Create our main window
		// Pass pointer to self
		wnd_ = ::CreateWindowW(wname.c_str(), wname.c_str(),  win_style_, settings.left, settings.top,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
		if( !wnd_ )
		{
			::MessageBoxW(0, L"CreateWindow Failed.", 0, 0);
		}

        default_wnd_proc_ = ::DefWindowProc;
        external_wnd_ = false;
	}

	RECT rc;
	::GetClientRect(wnd_, &rc);
	left_ = rc.left;
	top_ = rc.top;
	width_ = rc.right - rc.left;
	height_ = rc.bottom - rc.top;

	::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	::ShowWindow(wnd_, hide_ ? SW_HIDE : SW_SHOWNORMAL);
	::UpdateWindow(wnd_);

    ready_ = true;
}

Window::~Window()
{
    if (keep_screen_on_)
    {
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
        ::SetThreadExecutionState(ES_CONTINUOUS);
#endif
    }

    if (wnd_ != nullptr)
    {
        ::SetWindowLongPtrW(wnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(nullptr));
        if (!external_wnd_)
        {
            ::DestroyWindow(wnd_);
        }

        wnd_ = nullptr;
    }
}

LRESULT Window::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
        case WM_ACTIVATE:
            active_ = (WA_INACTIVE != LOWORD(wParam));
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_CLOSE:
        {		
            active_ = false;
            ready_ = false;
            closed_ = true;
            ::PostQuitMessage(0);
            return 0;
        }
	}

	return default_wnd_proc_(hWnd, uMsg, wParam, lParam);
}

void Window::KeepScreenOn()
{
    if (keep_screen_on_)
    {
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
        ::SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
#endif
    }
}

void Window::DetectsDpi()
{
    
}

}