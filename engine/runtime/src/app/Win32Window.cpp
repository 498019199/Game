#include <base/Window.h>
#include <base/App3D.h>
#include <base/Input.h>
#include <windowsx.h>

namespace RenderWorker
{
namespace
{
constexpr wchar_t WindowClass[] = L"ZEngineDesktopWindow";

int Modifiers()
{
    int mods = 0;
    if (GetKeyState(VK_SHIFT) < 0) mods |= WindowInput::Shift;
    if (GetKeyState(VK_CONTROL) < 0) mods |= WindowInput::Control;
    if (GetKeyState(VK_MENU) < 0) mods |= WindowInput::Alt;
    if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0) mods |= WindowInput::Meta;
    if (GetKeyState(VK_CAPITAL) & 1) mods |= WindowInput::CapsLock;
    if (GetKeyState(VK_NUMLOCK) & 1) mods |= WindowInput::NumLock;
    return mods;
}

int ScanCode(WPARAM key, LPARAM flags)
{
    switch (key)
    {
    case VK_PAUSE: return KS_Pause;
    case VK_NUMLOCK: return KS_NumLock;
    case VK_SNAPSHOT: return KS_SysRQ;
    default: break;
    }

    // KeyboardSemantic stores the PC scan code, with bit 7 set for E0 keys.
    int const code = (flags >> 16) & 0xff;
    return code && code < 0x80 ? code | ((flags & 0x01000000) ? 0x80 : 0) : 0;
}
}

void Window::InitializeDesktop()
{
    static bool const initialized = [] {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        WNDCLASSW cls{};
        cls.style = CS_DBLCLKS;
        cls.lpfnWndProc = &Window::WndProc;
        cls.hInstance = GetModuleHandleW(nullptr);
        cls.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        cls.lpszClassName = WindowClass;
        return RegisterClassW(&cls) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }();
    if (!initialized)
        TMSG("Unable to register the Win32 window class");
}

Window::Window(std::string const& name, RenderSettings const& settings, void* native_wnd)
    : left_(settings.left), top_(settings.top), width_(settings.width), height_(settings.height),
      dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity), active_(false), ready_(false),
      closed_(false), keep_screen_on_(settings.keep_screen_on)
{
    InitializeDesktop();
    auto const parent = static_cast<HWND>(native_wnd);
    DWORD const style = parent ? WS_CHILD : settings.full_screen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
    std::wstring title;
    CommonWorker::Convert(title, name);
    window_ = CreateWindowExW(0, WindowClass, title.c_str(), style, settings.left, settings.top, 0, 0, parent,
        nullptr, GetModuleHandleW(nullptr), this);
    if (!window_)
        TMSG("Unable to create the Win32 window");

    UINT const dpi = GetDpiForWindow(window_);
    UpdateDpiScale(float(dpi) / USER_DEFAULT_SCREEN_DPI);
    RECT frame{0, 0, MulDiv(settings.width, dpi, USER_DEFAULT_SCREEN_DPI),
        MulDiv(settings.height, dpi, USER_DEFAULT_SCREEN_DPI)};
    int x = settings.left, y = settings.top;
    if (settings.full_screen && !parent)
    {
        MONITORINFO monitor{};
        monitor.cbSize = sizeof(monitor);
        if (GetMonitorInfoW(MonitorFromWindow(window_, MONITOR_DEFAULTTOPRIMARY), &monitor))
        {
            x = monitor.rcMonitor.left;
            y = monitor.rcMonitor.top;
            frame.right = monitor.rcMonitor.right - x;
            frame.bottom = monitor.rcMonitor.bottom - y;
        }
    }
    AdjustWindowRectExForDpi(&frame, style, FALSE, 0, dpi);
    SetWindowPos(window_, nullptr, x, y, frame.right - frame.left, frame.bottom - frame.top,
        SWP_NOZORDER | SWP_NOACTIVATE);
    auto const size = FramebufferSize();
    width_ = size.x();
    height_ = size.y();

    if (keep_screen_on_)
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
    ready_ = true;
    if (!settings.hide_win)
        SetVisible(true);
}

Window::~Window()
{
    if (keep_screen_on_)
        SetThreadExecutionState(ES_CONTINUOUS);
    SetWindowLongPtrW(window_, GWLP_USERDATA, 0);
    DestroyWindow(window_);
}

HWND Window::GetHWND() const
{
    return window_;
}

void* Window::NativeHandle() const
{
    return window_;
}

void Window::PumpEvents()
{
    MSG message;
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            auto& ctx = Context::Instance();
            if (ctx.AppValid() && ctx.AppInstance().MainWnd())
                ctx.AppInstance().MainWnd()->Closed(true);
            break;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

void Window::SetVisible(bool visible)
{
    ShowWindow(window_, visible ? SW_SHOW : SW_HIDE);
}

bool Window::Minimized() const
{
    return IsIconic(window_) != FALSE;
}

int2 Window::FramebufferSize() const
{
    RECT rect{};
    GetClientRect(window_, &rect);
    return int2(rect.right - rect.left, rect.bottom - rect.top);
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NCCREATE)
    {
        auto* creation = reinterpret_cast<CREATESTRUCTW const*>(lparam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
    }
    auto* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    return window ? window->HandleMessage(hwnd, message, wparam, lparam) : DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT Window::HandleMessage(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT const handled = message_handler_ ? message_handler_(hwnd, message, wparam, lparam) : 0;
    switch (message)
    {
    case WM_SETFOCUS:
        Active(true);
        break;
    case WM_KILLFOCUS:
        Active(false);
        break;
    case WM_CLOSE:
        if (ready_ && RequestClose())
            ShowWindow(hwnd, SW_HIDE);
        return 0;
    case WM_SIZE:
    {
        RECT rect{};
        if (wparam != SIZE_MINIMIZED && GetClientRect(hwnd, &rect))
        {
            width_ = rect.right;
            height_ = rect.bottom;
        }
        break;
    }
    case WM_MOVE:
    {
        RECT rect{};
        if (GetWindowRect(hwnd, &rect))
        {
            left_ = rect.left;
            top_ = rect.top;
        }
        break;
    }
    case WM_DPICHANGED:
    {
        UpdateDpiScale(float(HIWORD(wparam)) / USER_DEFAULT_SCREEN_DPI);
        auto const& rect = *reinterpret_cast<RECT const*>(lparam);
        SetWindowPos(hwnd, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYDOWN: case WM_SYSKEYDOWN: case WM_KEYUP: case WM_SYSKEYUP:
    {
        bool const up = message == WM_KEYUP || message == WM_SYSKEYUP;
        int const key = ScanCode(wparam, lparam);
        DispatchInput({WindowInput::Key, key, up ? WindowInput::Released
            : (lparam & 0x40000000) ? WindowInput::Repeated : WindowInput::Pressed, Modifiers()});
        break;
    }
    case WM_CHAR:
        DispatchText(static_cast<char16_t>(wparam));
        return 0;
    case WM_MOUSEMOVE:
        DispatchInput({WindowInput::Motion, 0, 0, Modifiers(), 0, double(GET_X_LPARAM(lparam)), double(GET_Y_LPARAM(lparam))});
        break;
    case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
    {
        bool const up = message == WM_LBUTTONUP || message == WM_RBUTTONUP || message == WM_MBUTTONUP || message == WM_XBUTTONUP;
        bool const extra = message == WM_XBUTTONDOWN || message == WM_XBUTTONUP || message == WM_XBUTTONDBLCLK;
        int const button = extra ? (GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? 3 : 4)
            : message <= WM_LBUTTONDBLCLK ? 0 : message <= WM_RBUTTONDBLCLK ? 1 : 2;
        DispatchInput({WindowInput::Button, button, up ? WindowInput::Released : WindowInput::Pressed,
            Modifiers(), 0, double(GET_X_LPARAM(lparam)), double(GET_Y_LPARAM(lparam))});
        if (!up && !GetCapture())
            SetCapture(hwnd);
        if (up && GetCapture() == hwnd && std::none_of(buttons_.begin(), buttons_.end(), [](bool pressed) { return pressed; }))
            ReleaseCapture();
        if (extra)
            return TRUE;
        break;
    }
    case WM_CAPTURECHANGED:
        // ImGui may release capture before this procedure handles the button-up.
        // That message still updates the engine state, so leave it queued as-is.
        if (lparam && reinterpret_cast<HWND>(lparam) != hwnd)
            for (int button = 0; button < ButtonCount; ++button)
                if (buttons_[button])
                    DispatchInput({WindowInput::Button, button, WindowInput::Released, Modifiers(), 0,
                        double(pointer_position_.x()), double(pointer_position_.y())});
        break;
    case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
    {
        POINT pos{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        ScreenToClient(hwnd, &pos);
        double const delta = double(GET_WHEEL_DELTA_WPARAM(wparam)) / WHEEL_DELTA;
        DispatchInput({WindowInput::Scroll, 0, 0, Modifiers(), 0, double(pos.x), double(pos.y),
            message == WM_MOUSEHWHEEL ? -delta : 0.0, message == WM_MOUSEWHEEL ? delta : 0.0});
        return 0;
    }
    }
    return handled ? handled : DefWindowProcW(hwnd, message, wparam, lparam);
}
}
