#pragma once
#include <common/macro.h>
#include <common/Timer.h>
#include <Windows.h>
#include <cstdint>
#include <list>

namespace RenderWorker
{
struct RenderSettings;

class WinAPP
{
public:
    WinAPP();
    ~WinAPP();
    
    bool CreateAppWindow(const RenderSettings& settings);
    
    int Run();

    HWND GetHWND() const { return hwnd_; }

    float AspectRatio() const { return static_cast<float>(width_) / height_; }
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
    
	LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void CalculateFrameStats();
protected:
    // Stats
    uint32_t total_num_frames_ = 0;
    float	fps_ = 0.f;
    float	accumulate_time_ = 0.f;
    uint32_t num_frames_ = 0;

    float app_time_ = 0.f;
	float frame_time_ = 0.f;
    CommonWorker::Timer timer_;
    bool is_paused = false;

    uint32_t	left_;
    uint32_t	top_;
    uint32_t	width_;
    uint32_t	height_;
    HWND hwnd_;
    uint32_t win_style_;
    float dpi_scale_ = 1.f;

    bool active_;
	bool ready_;
	bool closed_;
};



}