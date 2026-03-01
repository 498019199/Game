#include <base/Window.h>
#include <base/ZEngine.h>
#include <world/World.h>
#include <render/RenderEngine.h>
#include <common/DllLoader.h>

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
#include <VersionHelpers.h>
#include <ShellScalingAPI.h>
#endif

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

// win8.1以上，回调函数：每枚举到一台显示器就调用一次
#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
BOOL Window::EnumMonProc(HMONITOR mon, [[maybe_unused]] HDC dc_mon, [[maybe_unused]] RECT* rc_mon, LPARAM lparam) noexcept
{
	DllLoader shcore;
	if (shcore.Load("SHCore.dll"))
	{
		typedef HRESULT (CALLBACK *GetDpiForMonitorFunc)(HMONITOR mon, MONITOR_DPI_TYPE dpi_type, UINT* dpi_x, UINT* dpi_y);
		auto* DynamicGetDpiForMonitor = reinterpret_cast<GetDpiForMonitorFunc>(shcore.GetProcAddress("GetDpiForMonitor"));
		if (DynamicGetDpiForMonitor)
		{
			UINT dpi_x, dpi_y;
			if (S_OK == DynamicGetDpiForMonitor(mon, MDT_DEFAULT, &dpi_x, &dpi_y))
			{
				Window* win = reinterpret_cast<Window*>(lparam);
				win->UpdateDpiScale(std::max(win->dpi_scale_, static_cast<float>(std::max(dpi_x, dpi_y)) / USER_DEFAULT_SCREEN_DPI));
			}
		}

		shcore.Free();
	}

	return TRUE;
}
#endif

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
	if( msg_proc_ptr_ )
	{
		msg_proc_ptr_(hWnd, uMsg, wParam, lParam);
	}
	
	switch (uMsg)
	{
        case WM_ACTIVATE:
            active_ = (WA_INACTIVE != LOWORD(wParam));
            break;

        case WM_ERASEBKGND:
            return 1;

		case WM_SIZE:
			{
				RECT rc;
				::GetClientRect(wnd_, &rc);
				left_ = rc.left;
				top_ = rc.top;
				width_ = rc.right - rc.left;
				height_ = rc.bottom - rc.top;
			}
			break;

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		case WM_DPICHANGED:
			{
				RECT rc;
				::GetClientRect(wnd_, &rc);
				rc.left = static_cast<int32_t>(rc.left / dpi_scale_ + 0.5f);
				rc.top = static_cast<int32_t>(rc.top / dpi_scale_ + 0.5f);
				rc.right = static_cast<uint32_t>(rc.right / dpi_scale_ + 0.5f);
				rc.bottom = static_cast<uint32_t>(rc.bottom / dpi_scale_ + 0.5f);

				this->UpdateDpiScale(static_cast<float>(HIWORD(wParam)) / USER_DEFAULT_SCREEN_DPI);

				rc.left = static_cast<int32_t>(rc.left * dpi_scale_ + 0.5f);
				rc.top = static_cast<int32_t>(rc.top * dpi_scale_ + 0.5f);
				rc.right = static_cast<uint32_t>(rc.right * dpi_scale_ + 0.5f);
				rc.bottom = static_cast<uint32_t>(rc.bottom * dpi_scale_ + 0.5f);

				::AdjustWindowRect(&rc, win_style_, false);

				::SetWindowPos(this->GetHWND(), HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION);
			}
			break;
#endif
        case WM_CLOSE:
			{		
				active_ = false;
				ready_ = false;
				closed_ = true;
				::PostQuitMessage(0);
				return 0;
			}
			break;
	}

	return default_wnd_proc_(hWnd, uMsg, wParam, lParam);
}

void Window::KeepScreenOn()
{
    if (keep_screen_on_)
    {
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
		// 阻止系统进入睡眠或休眠状态
        ::SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
#endif
    }
}

void Window::DetectsDpi()
{
	DllLoader shcore;
	if (shcore.Load("user32.dll"))
	{
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)
		typedef BOOL (WINAPI *SetProcessDpiAwarenessContextFunc)(DPI_AWARENESS_CONTEXT value);
		auto* DynamicSetProcessDpiAwarenessContext =
			reinterpret_cast<SetProcessDpiAwarenessContextFunc>(shcore.GetProcAddress("SetProcessDpiAwarenessContext")); 
		DynamicSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#else
		typedef HRESULT (WINAPI *SetProcessDpiAwarenessFunc)(PROCESS_DPI_AWARENESS value);
		auto* DynamicSetProcessDpiAwareness =
			reinterpret_cast<SetProcessDpiAwarenessFunc>(shcore.GetProcAddress("SetProcessDpiAwareness"));
		DynamicSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE); 
#endif
		shcore.Free();
	}

	typedef NTSTATUS (WINAPI *RtlGetVersionFunc)(OSVERSIONINFOEXW* pVersionInformation);
	DllLoader ntdll;
	ntdll.Load("ntdll.dll");
	auto* DynamicRtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(ntdll.GetProcAddress("RtlGetVersion"));
	if (DynamicRtlGetVersion)
	{
		OSVERSIONINFOEXW os_ver_info;
		os_ver_info.dwOSVersionInfoSize = sizeof(os_ver_info);
		DynamicRtlGetVersion(&os_ver_info);

		if ((os_ver_info.dwMajorVersion > 6) || ((6 == os_ver_info.dwMajorVersion) && (os_ver_info.dwMinorVersion >= 3)))
		{
			// 获取每台显示器的 DPI 和分辨率
			HDC desktop_dc = ::GetDC(nullptr);
			::EnumDisplayMonitors(desktop_dc, nullptr, EnumMonProc, reinterpret_cast<LPARAM>(this));
			::ReleaseDC(nullptr, desktop_dc);
		}
	}
}

}