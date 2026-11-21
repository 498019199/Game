#pragma once

#include <render/RenderView.h>

namespace RenderWorker
{

class D3D12ShaderResourceView final : public ShaderResourceView
{
public:
	D3D12ShaderResourceView(TexturePtr const& texture, ElementFormat pf, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels);
};

class D3D12RenderTargetView final : public RenderTargetView
{
public:
	D3D12RenderTargetView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);
	D3D12RenderTargetView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

	void ClearColor(Color const& clr) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
	void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) override;
};

class D3D12DepthStencilView final : public DepthStencilView
{
public:
	D3D12DepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count, uint32_t sample_quality);
	D3D12DepthStencilView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);

	void ClearDepth(float depth) override;
	void ClearStencil(int32_t stencil) override;
	void ClearDepthStencil(float depth, int32_t stencil) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb) override;
	void OnDetached(FrameBuffer& fb) override;
};

class D3D12UnorderedAccessView final : public UnorderedAccessView
{
public:
	D3D12UnorderedAccessView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size, int level);
	D3D12UnorderedAccessView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem, uint32_t num_elems);

	void Clear(float4 const& val) override;
	void Clear(uint4 const& val) override;
	void Discard() override;
	void OnAttached(FrameBuffer& fb, uint32_t att) override;
	void OnDetached(FrameBuffer& fb, uint32_t att) override;
};

} // namespace RenderWorker
