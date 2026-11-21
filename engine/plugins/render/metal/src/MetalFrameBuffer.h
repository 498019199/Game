#pragma once

#include <render/FrameBuffer.h>
#include "MetalUtil.h"

namespace RenderWorker
{

class MetalFrameBuffer : public FrameBuffer
{
public:
	MetalFrameBuffer();
	~MetalFrameBuffer() override;

	void OnBind() override;
	void OnUnbind() override;
	void Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil) override;
	void Discard(uint32_t flags) override;

	void SwapchainTexture(metal::Texture* tex) noexcept { swapchain_tex_ = tex; }
	metal::Texture* SwapchainTexture() const noexcept { return swapchain_tex_; }

protected:
	metal::Texture* swapchain_tex_{nullptr};
};

} // namespace RenderWorker
