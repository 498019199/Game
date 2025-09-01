#pragma once
#include <common/macro.h>
#include <common/Timer.h>
#include <base/RenderSettings.h>
#include <cstdint>
#include <list>
#include <string>
#include <memory>

namespace RenderWorker
{
class Window;
using WindowPtr = std::shared_ptr<Window>;
// 一个用于创建3D应用程序框架的基类。建立一个3D应用程序需要继承这个类，
//			然后重载以下函数:
//
//			OnCreate()				- Called when the app is creating.
//			OnDestroy()				- Called when the app is destroying.
//			OnSuspend()				- Called when the app is suspending
//			OnResume()				- Called when the app is resuming.
//			DoUpdate()				- 刷新场景
//			DoUpdateOverlay()		- 刷新Overlay物体
/////////////////////////////////////////////////////////////////////////////////

class App3D
{
public:
    explicit App3D(const std::string& name);
    App3D(const std::string& name, void* native_wnd);
    virtual ~App3D();
    
    bool Create();
    int Run();
    void Suspend();
    // 退出程序
    void Quit();
    void Destroy();

    const std::string& Name() const;
    const WindowPtr& MainWnd() const;
    
    WindowPtr MakeWindow(std::string const & name, RenderSettings const & settings);
    WindowPtr MakeWindow(std::string const & name, RenderSettings const & settings, void* native_wnd);

    virtual void OnResize(uint32_t width, uint32_t height);

    uint32_t TotalNumFrames() const;
    float FPS() const;
    float AppTime() const;
    float FrameTime() const;
protected:
    uint32_t Update(uint32_t pass);
    void UpdateStats();

private:
    virtual void OnCreate()
    {
    }
    virtual void OnDestroy()
    {
    }
    virtual void OnSuspend()
    {
    }
    virtual void OnResume()
    {
    }

protected:
    std::string name_;

    // Stats
    uint32_t total_num_frames_ = 0;
    float	fps_ = 0.f;
    float	accumulate_time_ = 0.f;
    uint32_t num_frames_ = 0;

    float app_time_ = 0.f;
	float frame_time_ = 0.f;
    CommonWorker::Timer timer_;

    WindowPtr main_wnd_;
};



}