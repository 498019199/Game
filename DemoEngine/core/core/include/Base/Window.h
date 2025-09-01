#pragma once
#include <common/macro.h>
#include <base/RenderSettings.h>

namespace RenderWorker
{

class Window final
{
public:
    Window(const std::string& name, const RenderSettings& settings, void* native_wnd);
    ~Window();

#if ZENGINE_PLATFORM_WINDOWS_DESKTOP
    HWND GetHWND() const { return hwnd_; }
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

    float AspectRatio() const { return static_cast<float>(width_) / height_; }
private:


#if ZENGINE_PLATFORM_WINDOWS_DESKTOP
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP

protected:
    uint32_t	left_;
    uint32_t	top_;
    uint32_t	width_;
    uint32_t	height_;

#if ZENGINE_PLATFORM_WINDOWS_DESKTOP
    HWND hwnd_;
    uint32_t win_style_;
#endif// ZENGINE_PLATFORM_WINDOWS_DESKTOP
    
    float dpi_scale_ = 1.f;

    bool active_;
	bool ready_;
	bool closed_;
};
}