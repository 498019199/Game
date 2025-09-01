#include <base/App3D.h>
#include <base/Context.h>
#include <render/RenderFactory.h>
#include <render/RenderEngine.h>

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
#ifdef ZENGINE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4355) // Ignore "this" in member initializer list
#endif
#include <future>
#ifdef ZENGINE_COMPILER_MSVC
#pragma warning(pop)
#endif
#include <windows.h>
#if defined(ZENGINE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(push)
#pragma warning(disable : 4265) // Ignore non-virtual destructor
#pragma warning(disable : 4946) // Ignore warnings of reinterpret_cast
#pragma warning(disable : 5246) // Turn of warnings of _Elems
#endif
#if defined(ZENGINE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(pop)
#endif
#endif//ZENGINE_PLATFORM_WINDOWS_DESKTOP

namespace RenderWorker
{

App3D::App3D(const std::string& name)
    : App3D(name, nullptr)
{
    
}

App3D::App3D(const std::string& name, void* native_wnd)
{
    Context::Instance().AppInstance(*this);

    ContextConfig cfg = Context::Instance().Config();
    main_wnd_ = this->MakeWindow(name_, cfg.graphics_cfg, native_wnd);
#ifndef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    auto const & win = Context::Instance().AppInstance().MainWnd();
    float const eff_dpi_scale = win->EffectiveDPIScale();
    cfg.graphics_cfg.left = static_cast<uint32_t>(main_wnd_->Left() / eff_dpi_scale + 0.5f);
    cfg.graphics_cfg.top = static_cast<uint32_t>(main_wnd_->Top() / eff_dpi_scale + 0.5f);
    cfg.graphics_cfg.width = static_cast<uint32_t>(main_wnd_->Width() / eff_dpi_scale + 0.5f);
    cfg.graphics_cfg.height = static_cast<uint32_t>(main_wnd_->Height() / eff_dpi_scale + 0.5f);
    Context::Instance().Config(cfg);
#endif
}

App3D::~App3D()
{
    Destroy();
}

bool App3D::Create()
{
    ContextConfig cfg = Context::Instance().Config();
    Context::Instance().RenderFactoryInstance().RenderEngineInstance().CreateRenderWindow(name_, cfg.graphics_cfg);

    OnCreate();
    OnResize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
}

int App3D::Run()
{
    
}

void App3D::Suspend()
{
    Context::Instance().Suspend();
    OnSuspend();
}
	
void App3D::Quit()
{
#ifdef ZENGINE_PLATFORM_WINDOWS
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    ::PostQuitMessage(0);
#endif
#else
    exit(0);
#endif
}

void App3D::Destroy()
{
    if (Context::Instance().RenderFactoryValid())
    {
        Context::Instance().RenderFactoryInstance().RenderEngineInstance().DestroyRenderWindow();
    }

    main_wnd_.reset();

    Context::Destroy();
}

const std::string& App3D::Name() const
{
    return name_;
}

const WindowPtr& App3D::MainWnd() const
{
    return main_wnd_;
}

WindowPtr App3D::MakeWindow(std::string const & name, RenderSettings const & settings)
{
    return MakeSharedPtr<Window>(name, settings, nullptr);
}

WindowPtr App3D::MakeWindow(std::string const & name, RenderSettings const & settings, void* native_wnd)
{
    return MakeSharedPtr<Window>(name, settings, native_wnd);
}

void App3D::OnResize(uint32_t width, uint32_t height)
{
    
}


uint32_t App3D::Update(uint32_t pass)
{
    
}

// 获取渲染目标的每秒帧数
/////////////////////////////////////////////////////////////////////////////////
uint32_t App3D::TotalNumFrames() const
{
    return total_num_frames_;
}

float App3D::FPS() const
{
	return fps_;
}

float App3D::AppTime() const
{
    return app_time_;
}

float App3D::FrameTime() const
{
    return frame_time_;
}

void App3D::UpdateStats()
{
    ++ total_num_frames_;

	// measure statistics
	frame_time_ = static_cast<float>(timer_.elapsed());
	++ num_frames_;
	accumulate_time_ += frame_time_;
	app_time_ += frame_time_;

	// check if new second
	if (accumulate_time_ > 1)
	{
		// new second - not 100% precise
		fps_ = num_frames_ / accumulate_time_;

		accumulate_time_ = 0;
		num_frames_  = 0;
	}

	timer_.restart();
}

}