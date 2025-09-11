#pragma once
#include <base/ZEngine.h>
#include <base/RenderSettings.h>

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

namespace RenderWorker
{

class ZENGINE_CORE_API Window final
{
    ZENGINE_NONCOPYABLE(Window);
public:
    Window(const std::string& name, const RenderSettings& settings, void* native_wnd);
    ~Window();

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    HWND GetHWND() const { return wnd_; }
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

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
private:


#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // 防止系统进入睡眠或休眠状态
    void KeepScreenOn();

    void DetectsDpi();
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

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
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP
#endif //ZENGINE_PLATFORM_WINDOWS

    float dpi_scale_ = 1.f;

    bool active_ = false;
	bool ready_ = false;
	bool closed_ = false;
	bool keep_screen_on_;
};



}