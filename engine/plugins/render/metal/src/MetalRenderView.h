#pragma once

#include <render/RenderView.h>

namespace RenderWorker
{

class MetalShaderResourceView final : public ShaderResourceView
{
public:
	MetalShaderResourceView(TexturePtr const& texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels);
};

class MetalRenderTargetView final : public RenderTargetView
{
public:
	MetalRenderTargetView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);
	MetalRenderTargetView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

	void ClearColor(Color const& clr) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
	void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
};

class MetalDepthStencilView final : public DepthStencilView
{
public:
	MetalDepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
	MetalDepthStencilView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);

	void ClearDepth(float depth) override;
	void ClearStencil(int32_t stencil) override;
	void ClearDepthStencil(float depth, int32_t stencil) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb) override;
	void OnDetached(FrameBuffer& fb) override;
};

class MetalUnorderedAccessView final : public UnorderedAccessView
{
public:
	MetalUnorderedAccessView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);
	MetalUnorderedAccessView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

	void Clear(float4 const& val) override;
	void Clear(uint4 const& val) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb, uint32_t att) override;
	void OnDetached(FrameBuffer& fb, uint32_t att) override;
};

} // namespace RenderWorker
