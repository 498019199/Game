#pragma once

#include <render/FrameBuffer.h>
#include "D3D12Util.h"

namespace RenderWorker
{

class D3D12FrameBuffer : public FrameBuffer
{
public:
	D3D12FrameBuffer();
	~D3D12FrameBuffer() override;

	void OnBind() override;
	void OnUnbind() override;
	void Clear(uint32_t flags, Color const& clr, float depth, int32_t stencil) override;
	void Discard(uint32_t flags) override;

	void SwapchainTexture(d3d12::Texture* tex) noexcept { swapchain_tex_ = tex; }
	d3d12::Texture* SwapchainTexture() const noexcept { return swapchain_tex_; }

protected:
	d3d12::Texture* swapchain_tex_{nullptr};
};

} // namespace RenderWorker
