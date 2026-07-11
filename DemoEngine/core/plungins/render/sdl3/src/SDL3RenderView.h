#pragma once

#include <render/RenderView.h>

namespace RenderWorker
{

class SDL3ShaderResourceView final : public ShaderResourceView
{
public:
	SDL3ShaderResourceView(TexturePtr const& texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels);
};

class SDL3RenderTargetView final : public RenderTargetView
{
public:
	SDL3RenderTargetView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);
	SDL3RenderTargetView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

	void ClearColor(Color const& clr) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
	void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
};

class SDL3DepthStencilView final : public DepthStencilView
{
public:
	SDL3DepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
	SDL3DepthStencilView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);

	void ClearDepth(float depth) override;
	void ClearStencil(int32_t stencil) override;
	void ClearDepthStencil(float depth, int32_t stencil) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb) override;
	void OnDetached(FrameBuffer& fb) override;
};

class SDL3UnorderedAccessView final : public UnorderedAccessView
{
public:
	SDL3UnorderedAccessView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);
	SDL3UnorderedAccessView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

	void Clear(float4 const& val) override;
	void Clear(uint4 const& val) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb, uint32_t att) override;
	void OnDetached(FrameBuffer& fb, uint32_t att) override;
};

} // namespace RenderWorker
