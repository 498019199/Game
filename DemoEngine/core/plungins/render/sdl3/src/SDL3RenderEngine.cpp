#include "SDL3RenderEngine.h"
#include "SDL3RenderWindow.h"
#include <base/ZEngine.h>

namespace RenderWorker
{

SDL3RenderEngine::SDL3RenderEngine() = default;

SDL3RenderEngine::~SDL3RenderEngine()
{
	this->Destroy();
}

void SDL3RenderEngine::Device(SDL_GPUDevice* device, SDL_Window* window)
{
	device_ = device;
	window_ = window;
	this->FillRenderDeviceCaps();
}

void SDL3RenderEngine::FillRenderDeviceCaps()
{
	caps_.max_shader_model = ShaderModel(5, 0);
	caps_.max_texture_width = 16384;
	caps_.max_texture_height = 16384;
	caps_.max_texture_depth = 2048;
	caps_.max_texture_cube_size = 16384;
	caps_.max_texture_array_length = 2048;
	caps_.max_vertex_texture_units = 16;
	caps_.max_pixel_texture_units = 16;
	caps_.max_geometry_texture_units = 16;
	caps_.max_simultaneous_rts = 8;
	caps_.max_simultaneous_uavs = 8;
	caps_.max_vertex_streams = 16;
	caps_.max_texture_anisotropy = 16;

	caps_.primitive_restart_support = true;
	caps_.multithread_rendering_support = false;
	caps_.multithread_res_creating_support = false;
	caps_.arbitrary_multithread_rendering_support = false;
	caps_.mrt_independent_bit_depths_support = true;
	caps_.logic_op_support = false;
	caps_.independent_blend_support = true;
	caps_.depth_texture_support = true;
	caps_.fp_color_support = true;
	caps_.pack_to_rgba_required = false;
	caps_.draw_indirect_support = true;
	caps_.no_overwrite_support = true;
	caps_.full_npot_texture_support = true;
	caps_.render_to_texture_array_support = true;
	caps_.explicit_multi_sample_support = true;
	caps_.load_from_buffer_support = true;
	caps_.uavs_at_every_stage_support = false;
	caps_.rovs_support = false;
	caps_.flexible_srvs_support = true;
	caps_.vp_rt_index_at_every_stage_support = false;
	caps_.gs_support = false;
	caps_.cs_support = true;
	caps_.hs_support = false;
	caps_.ds_support = false;
}

void SDL3RenderEngine::BeginFrame()
{
	RenderEngine::BeginFrame();
	if (!device_ || !window_)
	{
		return;
	}

	cmd_ = SDL_AcquireGPUCommandBuffer(device_);
	if (!cmd_)
	{
		LogError() << "[SDL3] AcquireGPUCommandBuffer failed: " << SDL_GetError() << std::endl;
		return;
	}

	Uint32 sw = 0;
	Uint32 sh = 0;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_, window_, &swapchain_tex_, &sw, &sh))
	{
		LogError() << "[SDL3] WaitAndAcquireGPUSwapchainTexture failed: " << SDL_GetError() << std::endl;
		swapchain_tex_ = nullptr;
	}
}

void SDL3RenderEngine::EndFrame()
{
	if (cmd_)
	{
		SDL_SubmitGPUCommandBuffer(cmd_);
		cmd_ = nullptr;
		swapchain_tex_ = nullptr;
	}
	RenderEngine::EndFrame();
}

void SDL3RenderEngine::DoCreateRenderWindow(std::string const& name, RenderSettings const& settings)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL3Check(SDL_InitSubSystem(SDL_INIT_VIDEO), "SDL_InitSubSystem(SDL_INIT_VIDEO)");
		owns_sdl_init_ = true;
	}

	auto win = MakeSharedPtr<SDL3RenderWindow>(name, settings);
	native_shader_platform_name_ = "sdl3_gpu";
	this->BindFrameBuffer(win);
}

void SDL3RenderEngine::DoRender([[maybe_unused]] const RenderEffect& effect,
	[[maybe_unused]] const RenderTechnique& tech, [[maybe_unused]] const RenderLayout& rl)
{
	// Full draw-path (pipelines / vertex bind) will be filled in incrementally.
}

void SDL3RenderEngine::DoBindFrameBuffer([[maybe_unused]] FrameBufferPtr const& fb)
{
}

void SDL3RenderEngine::DoBindSOBuffers([[maybe_unused]] const RenderLayoutPtr& rl)
{
}

void SDL3RenderEngine::DoDestroy()
{
	if (cmd_)
	{
		SDL_CancelGPUCommandBuffer(cmd_);
		cmd_ = nullptr;
	}
	swapchain_tex_ = nullptr;

	if (device_ && window_)
	{
		SDL_ReleaseWindowFromGPUDevice(device_, window_);
	}
	window_ = nullptr;

	if (device_)
	{
		SDL_DestroyGPUDevice(device_);
		device_ = nullptr;
	}

	if (owns_sdl_init_)
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		owns_sdl_init_ = false;
	}
}

} // namespace RenderWorker
