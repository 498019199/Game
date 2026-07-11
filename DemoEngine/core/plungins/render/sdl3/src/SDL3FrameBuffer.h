#pragma once

#include <render/FrameBuffer.h>
#include "SDL3Util.h"

namespace RenderWorker
{

class SDL3FrameBuffer : public FrameBuffer
{
public:
	SDL3FrameBuffer();
	~SDL3FrameBuffer() override;

	void OnBind() override;
	void OnUnbind() override;
	void Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil) override;
	void Discard(uint32_t flags) override;

	void SwapchainTexture(SDL_GPUTexture* tex) noexcept { swapchain_tex_ = tex; }
	SDL_GPUTexture* SwapchainTexture() const noexcept { return swapchain_tex_; }

protected:
	SDL_GPUTexture* swapchain_tex_{nullptr};
};

} // namespace RenderWorker
