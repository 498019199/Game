#include "D3D12RenderWindow.h"
#include "D3D12RenderEngine.h"
#include "D3D12RenderView.h"
#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <render/RenderFactory.h>

namespace RenderWorker
{

D3D12RenderWindow::D3D12RenderWindow(std::string const& name, RenderSettings const& settings)
	: name_(name), is_full_screen_(settings.full_screen), sync_interval_(settings.sync_interval)
{
	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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
    auto* device = d3d12::CreateDevice(window_, sync_interval_, settings.debug_context);
    D3D12Check(device != nullptr, "Create graphics device");
	re.Device(device, window_);

	depth_stencil_fmt_ = settings.depth_stencil_fmt;
	sample_count_ = settings.sample_count;
	sample_quality_ = settings.sample_quality;
	if (IsDepthFormat(depth_stencil_fmt_))
	{
		auto dsv = MakeSharedPtr<D3D12DepthStencilView>(width_, height_, depth_stencil_fmt_, settings.sample_count,
			settings.sample_quality);
		this->Attach(dsv);
	}

    LogInfo() << "Created D3D12 render window " << width_ << "x" << height_ << std::endl;
}

D3D12RenderWindow::~D3D12RenderWindow()
{
	this->Destroy();
}

void D3D12RenderWindow::Destroy()
{
	if (!window_)
	{
		return;
	}

	if (Context::Instance().RenderFactoryValid())
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.Window() == window_)
		{
			// Device lifetime is owned by D3D12RenderEngine::DoDestroy.
			re.DetachWindow();
		}
	}
	window_ = nullptr;
}

void D3D12RenderWindow::SwapBuffers()
{
}

void D3D12RenderWindow::Resize(uint32_t width, uint32_t height)
{
	if (!width || !height || (width == width_ && height == height_)) return;
	// The graphics device resizes the swapchain; our depth attachment and viewport must follow it.
	if (IsDepthFormat(depth_stencil_fmt_))
		Attach(MakeSharedPtr<D3D12DepthStencilView>(width, height, depth_stencil_fmt_, sample_count_, sample_quality_));
	width_ = width;
	height_ = height;
	viewport_->Width(width);
	viewport_->Height(height);
	views_dirty_ = true;
}

} // namespace RenderWorker
