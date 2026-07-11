#include "SDL3FrameBuffer.h"
#include "SDL3RenderEngine.h"
#include <base/ZEngine.h>
#include <math/color.h>

namespace RenderWorker
{

SDL3FrameBuffer::SDL3FrameBuffer() = default;
SDL3FrameBuffer::~SDL3FrameBuffer() = default;

void SDL3FrameBuffer::OnBind()
{
	views_dirty_ = false;
}

void SDL3FrameBuffer::OnUnbind()
{
}

void SDL3FrameBuffer::Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil)
{
	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUCommandBuffer* cmd = re.CurrentCommandBuffer();
	SDL_GPUTexture* color_tex = re.CurrentSwapchainTexture();
	if (!cmd || !color_tex)
	{
		return;
	}

	SDL_GPUColorTargetInfo color_info{};
	color_info.texture = color_tex;
	color_info.clear_color.r = clr.r();
	color_info.clear_color.g = clr.g();
	color_info.clear_color.b = clr.b();
	color_info.clear_color.a = clr.a();
	color_info.load_op = (flags & CBM_Color) ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
	color_info.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPUDepthStencilTargetInfo ds_info{};
	bool has_ds = false;
	if (ds_view_ && ((flags & CBM_Depth) || (flags & CBM_Stencil)))
	{
		// Depth texture attachment will be wired when SDL3Texture is complete.
		has_ds = false;
		(void)depth;
		(void)stencil;
		(void)ds_info;
	}

	SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color_info, 1, has_ds ? &ds_info : nullptr);
	if (pass)
	{
		SDL_EndGPURenderPass(pass);
	}
}

void SDL3FrameBuffer::Discard([[maybe_unused]] uint32_t flags)
{
}

} // namespace RenderWorker
