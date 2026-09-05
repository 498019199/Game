#pragma once
#include <base/ZEngine.h>
#include <base/RenderSettings.h>

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
using MsgProcFunc = LRESULT(*)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

struct SDL_Window;
union SDL_Event;

namespace RenderWorker
{

class ZENGINE_CORE_API Window final
{
    ZENGINE_NONCOPYABLE(Window);
public:
    enum WindowRotation
    {
        WR_Unspecified,
        WR_Identity,
        WR_Rotate90,
        WR_Rotate180,
        WR_Rotate270
    };

    Window(const std::string& name, const RenderSettings& settings, void* native_wnd);
    ~Window();

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    HWND GetHWND() const { return wnd_; }
    void BindMsgProc(MsgProcFunc func) { msg_proc_ptr_ = func; }
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

	SDL_Window* GetSDLWindow() const noexcept { return sdl_wnd_; }
	static void PumpEvents();

    int32_t Left() const { return left_; }
    int32_t Top() const { return top_; }
    uint32_t Width() const { return width_; }
    uint32_t Height() const { return height_; }

    bool Active() const { return active_; }
    void Active(bool active){ active_ = active; }
    bool Ready() const { return ready_; }
    void Ready(bool ready) { ready_ = ready; }
    bool Closed() const { return closed_; }
    void Closed(bool closed) { closed_ = closed; }
    
    float DPIScale() const
    {
        return dpi_scale_;
    }

    float EffectiveDPIScale() const
    {
        return effective_dpi_scale_;
    }

    WindowRotation Rotation() const
    {
        return win_rotation_;
    }
private:
    void UpdateDpiScale(float scale);

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
		static BOOL CALLBACK EnumMonProc(HMONITOR mon, HDC dc_mon, RECT* rc_mon, LPARAM lparam) noexcept;
#endif
    // 防止系统进入睡眠或休眠状态
    void KeepScreenOn();

    // Windows 系统 DPI 适配
    void DetectsDpi(int32_t left, int32_t top);
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

public:
    using SDLEvent = Signal::Signal<void(SDL_Event const&)>;
    SDLEvent& OnSDLEvent() { return sdl_event_; }
    typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id)> PointerDownEvent;
    typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id)> PointerUpEvent;
    typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id, bool down)> PointerUpdateEvent;
    typedef Signal::Signal<void(Window const& wnd, int2 const& pt, uint32_t id, int32_t wheel_delta)> PointerWheelEvent;
    typedef Signal::Signal<void(Window const& wnd)> CloseEvent;

    PointerDownEvent& OnPointerDown()
    {
        return pointer_down_event_;
    }
    PointerUpEvent& OnPointerUp()
    {
        return pointer_up_event_;
    }
    PointerUpdateEvent& OnPointerUpdate()
    {
        return pointer_update_event_;
    }
    PointerWheelEvent& OnPointerWheel()
    {
        return pointer_wheel_event_;
    }
    CloseEvent& OnClose()
    {
        return close_event_;
    }

private:
    SDLEvent sdl_event_;
    PointerDownEvent pointer_down_event_;
    PointerUpEvent pointer_up_event_;
    PointerUpdateEvent pointer_update_event_;
    PointerWheelEvent pointer_wheel_event_;
    CloseEvent close_event_;

protected:
    uint32_t	left_;
    uint32_t	top_;
    uint32_t	width_;
    uint32_t	height_;

#if defined ZENGINE_PLATFORM_WINDOWS
    bool hide_;
    bool external_wnd_;
    std::wstring wname_;
#if defined ZENGINE_PLATFORM_WINDOWS_DESKTOP
    HWND wnd_;
    uint32_t win_style_;
    WNDPROC default_wnd_proc_;

    MsgProcFunc msg_proc_ptr_ {nullptr};
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP
#endif //ZENGINE_PLATFORM_WINDOWS

	SDL_Window* sdl_wnd_{nullptr};
#if !defined(ZENGINE_PLATFORM_WINDOWS)
	bool hide_{false};
	std::string name_;
#endif

    float dpi_scale_;
    float effective_dpi_scale_;
    WindowRotation win_rotation_;

    bool active_;
	bool ready_;
	bool closed_;
	bool keep_screen_on_;
};



}
