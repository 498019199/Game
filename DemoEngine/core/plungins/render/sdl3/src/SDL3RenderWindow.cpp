#include "SDL3RenderWindow.h"
#include "SDL3RenderEngine.h"
#include "SDL3RenderView.h"
#include <base/ZEngine.h>
#include <base/Window.h>

namespace RenderWorker
{

namespace
{
SDL_Window* CreateSDLWindow(std::string const& name, RenderSettings const& settings, uint32_t width, uint32_t height)
{
	// Prefer the App3D main window created by WindowSDL.
	auto const& main_wnd = Context::Instance().AppInstance().MainWnd();
	if (main_wnd && main_wnd->GetSDLWindow())
	{
		return main_wnd->GetSDLWindow();
	}

	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if (settings.hide_win)
	{
		flags |= SDL_WINDOW_HIDDEN;
	}
	if (settings.full_screen)
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	SDL_Window* wnd = SDL_CreateWindow(name.c_str(), static_cast<int>(width), static_cast<int>(height), flags);
	SDL3Check(wnd != nullptr, "SDL_CreateWindow");
	if (!settings.hide_win)
	{
		SDL_SetWindowPosition(wnd, settings.left, settings.top);
	}
	return wnd;
}
} // namespace

SDL3RenderWindow::SDL3RenderWindow(std::string const& name, RenderSettings const& settings)
	: name_(name), is_full_screen_(settings.full_screen), sync_interval_(settings.sync_interval)
{
	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

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

	window_ = CreateSDLWindow(name, settings, width_, height_);
	// Only destroy the window if we created it ourselves (not App3D's main window).
	owns_window_ = !(main_wnd && main_wnd->GetSDLWindow() && window_ == main_wnd->GetSDLWindow());

	SDL_GPUShaderFormat shader_formats =
		SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
#ifdef ZENGINE_DEBUG
	bool const debug_mode = true;
#else
	bool const debug_mode = settings.debug_context;
#endif

	SDL_GPUDevice* device = SDL_CreateGPUDevice(shader_formats, debug_mode, nullptr);
	SDL3Check(device != nullptr, "SDL_CreateGPUDevice");
	SDL3Check(SDL_ClaimWindowForGPUDevice(device, window_), "SDL_ClaimWindowForGPUDevice");

	SDL_GPUSwapchainComposition composition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;
	SDL_GPUPresentMode present_mode =
		(sync_interval_ == 0) ? SDL_GPU_PRESENTMODE_IMMEDIATE : SDL_GPU_PRESENTMODE_VSYNC;
	SDL_SetGPUSwapchainParameters(device, window_, composition, present_mode);

	re.Device(device, window_);

	depth_stencil_fmt_ = settings.depth_stencil_fmt;
	if (IsDepthFormat(depth_stencil_fmt_))
	{
		auto dsv = MakeSharedPtr<SDL3DepthStencilView>(width_, height_, depth_stencil_fmt_, settings.sample_count,
			settings.sample_quality);
		this->Attach(dsv);
	}

	char const* driver = nullptr;
	SDL_PropertiesID device_props = SDL_GetGPUDeviceProperties(device);
	if (device_props)
	{
		driver = SDL_GetStringProperty(device_props, SDL_PROP_GPU_DEVICE_DRIVER_NAME_STRING, nullptr);
	}
	LogInfo() << "[SDL3] Render window \"" << name_ << "\" " << width_ << "x" << height_
			  << " driver=" << (driver ? driver : "unknown") << std::endl;
}

SDL3RenderWindow::~SDL3RenderWindow()
{
	this->Destroy();
}

void SDL3RenderWindow::Destroy()
{
	if (!window_)
	{
		return;
	}

	if (Context::Instance().RenderFactoryValid())
	{
		auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.Window() == window_)
		{
			// Device lifetime is owned by SDL3RenderEngine::DoDestroy.
			re.DetachWindow();
		}
	}

	if (owns_window_)
	{
		SDL_DestroyWindow(window_);
	}
	window_ = nullptr;
	owns_window_ = false;
}

void SDL3RenderWindow::SwapBuffers()
{
}

} // namespace RenderWorker
