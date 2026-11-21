#include "MetalRenderWindow.h"
#include "MetalRenderEngine.h"
#include "MetalRenderView.h"
#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <render/RenderFactory.h>

namespace RenderWorker
{

MetalRenderWindow::MetalRenderWindow(std::string const& name, RenderSettings const& settings)
	: name_(name), is_full_screen_(settings.full_screen), sync_interval_(settings.sync_interval)
{
	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	auto const& main_wnd = Context::Instance().AppInstance().MainWnd();
	float const dpi_scale = main_wnd ? main_wnd->DPIScale() : 1.0f;

	width_ = static_cast<uint32_t>(settings.width * dpi_scale + 0.5f);
	height_ = static_cast<uint32_t>(settings.height * dpi_scale + 0.5f);
	left_ = settings.left;
	top_ = settings.top;

	viewport_->Left(0);
	viewport_->Top(0);
	viewport_->Width(width_);
	viewport_->Height(height_);

    if (!main_wnd) TMSG("Renderer requires a main window");
    window_ = main_wnd->NativeHandle();
    auto* device = metal::CreateDevice(window_, sync_interval_, settings.debug_context);
    MetalCheck(device != nullptr, "Create graphics device");
	re.Device(device, window_);

	depth_stencil_fmt_ = settings.depth_stencil_fmt;
	sample_count_ = settings.sample_count;
	sample_quality_ = settings.sample_quality;
	if (IsDepthFormat(depth_stencil_fmt_))
	{
		auto dsv = MakeSharedPtr<MetalDepthStencilView>(width_, height_, depth_stencil_fmt_, settings.sample_count,
			settings.sample_quality);
		this->Attach(dsv);
	}

    LogInfo() << "Created Metal render window " << width_ << "x" << height_ << std::endl;
}

MetalRenderWindow::~MetalRenderWindow()
{
	this->Destroy();
}

void MetalRenderWindow::Destroy()
{
	if (!window_)
	{
		return;
	}

	if (Context::Instance().RenderFactoryValid())
	{
		auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.Window() == window_)
		{
			// Device lifetime is owned by MetalRenderEngine::DoDestroy.
			re.DetachWindow();
		}
	}
	window_ = nullptr;
}

void MetalRenderWindow::SwapBuffers()
{
}

void MetalRenderWindow::Resize(uint32_t width, uint32_t height)
{
	if (!width || !height || (width == width_ && height == height_)) return;
	// The graphics device resizes the swapchain; our depth attachment and viewport must follow it.
	if (IsDepthFormat(depth_stencil_fmt_))
		Attach(MakeSharedPtr<MetalDepthStencilView>(width, height, depth_stencil_fmt_, sample_count_, sample_quality_));
	width_ = width;
	height_ = height;
	viewport_->Width(width);
	viewport_->Height(height);
	views_dirty_ = true;
}

} // namespace RenderWorker
