#pragma once
#include <base/ZEngine.h>
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

class ZENGINE_CORE_API App3D
{
    ZENGINE_NONCOPYABLE(App3D);
    friend class World;
public:
    enum UpdateRetValue
    {
        URV_NeedFlush = 1UL << 0,               // 需要刷新
        URV_Finished = 1UL << 1,                // 表示更新操作已成功完成
        URV_Overlay = 1UL << 2,                 // 表示需要处理叠加层渲染（如 HUD、UI 叠加）。
        URV_SkipPostProcess = 1UL << 3,         // 跳过后期处理
        URV_OpaqueOnly = 1UL << 4,              // 表示只需要更新 / 渲染不透明物体（忽略透明物体）。
        URV_TransparencyBackOnly = 1UL << 5,    // 表示只处理透明物体的背面渲染。
        URV_TransparencyFrontOnly = 1UL << 6,   // 表示只处理透明物体的正面渲染。
        URV_ReflectionOnly = 1UL << 7,          // 表示只需要更新 / 渲染反射效果（如水面反射、镜面反射）。
        URV_SpecialShadingOnly = 1UL << 8,      // 表示只需要应用特殊着色器（如卡通渲染、体积光）。
        URV_SimpleForwardOnly = 1UL << 9,       // 表示只使用简单的前向渲染路径（不启用复杂光照）。
        URV_VDMOnly = 1UL << 10                 // 仅视口依赖映射（Viewport-Dependent Mapping）
    };

public:
    explicit App3D(const std::string& name);
    App3D(const std::string& name, void* native_wnd);
    virtual ~App3D();
    
    void Create();
    void Run();
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

    virtual uint32_t DoUpdate(uint32_t pass) = 0;

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