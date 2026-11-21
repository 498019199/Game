#include "D3D12RenderView.h"
#include "D3D12Texture.h"
#include <base/ZEngine.h>

namespace RenderWorker
{

D3D12ShaderResourceView::D3D12ShaderResourceView(TexturePtr const& texture, ElementFormat pf, uint32_t first_array_index,
	uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
	tex_ = texture;
	pf_ = (pf == EF_Unknown) ? texture->Format() : pf;
	first_array_index_ = first_array_index;
	array_size_ = array_size;
	first_level_ = first_level;
	num_levels_ = num_levels;
}

D3D12RenderTargetView::D3D12RenderTargetView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size,
	int level)
{
	tex_ = texture;
	pf_ = (pf == EF_Unknown) ? texture->Format() : pf;
	first_array_index_ = static_cast<uint32_t>(first_array_index);
	array_size_ = static_cast<uint32_t>(array_size);
	level_ = static_cast<uint32_t>(level);
	width_ = texture->Width(level_);
	height_ = texture->Height(level_);
	sample_count_ = texture->SampleCount();
	sample_quality_ = texture->SampleQuality();
}

D3D12RenderTargetView::D3D12RenderTargetView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem,
	uint32_t num_elems)
{
	buff_ = gbuffer;
	pf_ = pf;
	first_elem_ = first_elem;
	num_elems_ = num_elems;
	width_ = num_elems;
	height_ = 1;
	sample_count_ = 1;
	sample_quality_ = 0;
}

void D3D12RenderTargetView::ClearColor([[maybe_unused]] Color const& clr) {}
void D3D12RenderTargetView::Discard() {}
void D3D12RenderTargetView::OnAttached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] FrameBuffer::Attachment att) {}
void D3D12RenderTargetView::OnDetached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] FrameBuffer::Attachment att) {}

D3D12DepthStencilView::D3D12DepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
	uint32_t sample_quality)
{
	width_ = width;
	height_ = height;
	pf_ = pf;
	sample_count_ = sample_count;
	sample_quality_ = sample_quality;

	auto tex = MakeSharedPtr<D3D12Texture2D>(width, height, 1, 1, pf, sample_count, sample_quality,
		EAH_GPU_Read | EAH_GPU_Write);
	// Match typical FrameBuffer::Clear depth=1 / stencil=0 (D3D12 OptimizedClearValue).
	float4 const clear_hint(1.0f, 0.0f, 0.0f, 0.0f);
	tex->CreateHWResource({}, &clear_hint);
	tex_ = tex;
	first_array_index_ = 0;
	array_size_ = 1;
	level_ = 0;
}

D3D12DepthStencilView::D3D12DepthStencilView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size,
	int level)
{
	tex_ = texture;
	pf_ = (pf == EF_Unknown) ? texture->Format() : pf;
	first_array_index_ = static_cast<uint32_t>(first_array_index);
	array_size_ = static_cast<uint32_t>(array_size);
	level_ = static_cast<uint32_t>(level);
	width_ = texture->Width(level_);
	height_ = texture->Height(level_);
	sample_count_ = texture->SampleCount();
	sample_quality_ = texture->SampleQuality();
}

void D3D12DepthStencilView::ClearDepth([[maybe_unused]] float depth) {}
void D3D12DepthStencilView::ClearStencil([[maybe_unused]] int32_t stencil) {}
void D3D12DepthStencilView::ClearDepthStencil([[maybe_unused]] float depth, [[maybe_unused]] int32_t stencil) {}
void D3D12DepthStencilView::Discard() {}
void D3D12DepthStencilView::OnAttached([[maybe_unused]] FrameBuffer& fb) {}
void D3D12DepthStencilView::OnDetached([[maybe_unused]] FrameBuffer& fb) {}

D3D12UnorderedAccessView::D3D12UnorderedAccessView(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	tex_ = texture;
	pf_ = (pf == EF_Unknown) ? texture->Format() : pf;
	first_array_index_ = static_cast<uint32_t>(first_array_index);
	array_size_ = static_cast<uint32_t>(array_size);
	level_ = static_cast<uint32_t>(level);
}

D3D12UnorderedAccessView::D3D12UnorderedAccessView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem,
	uint32_t num_elems)
{
	buff_ = gbuffer;
	pf_ = pf;
	first_elem_ = first_elem;
	num_elems_ = num_elems;
}

void D3D12UnorderedAccessView::Clear([[maybe_unused]] float4 const& val) {}
void D3D12UnorderedAccessView::Clear([[maybe_unused]] uint4 const& val) {}
void D3D12UnorderedAccessView::Discard() {}
void D3D12UnorderedAccessView::OnAttached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] uint32_t att) {}
void D3D12UnorderedAccessView::OnDetached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] uint32_t att) {}

} // namespace RenderWorker
