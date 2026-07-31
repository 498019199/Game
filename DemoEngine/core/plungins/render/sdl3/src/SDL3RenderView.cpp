#include "SDL3RenderView.h"
#include "SDL3Texture.h"
#include <base/ZEngine.h>

namespace RenderWorker
{

SDL3ShaderResourceView::SDL3ShaderResourceView(TexturePtr const& texture, ElementFormat pf, uint32_t first_array_index,
	uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
	tex_ = texture;
	pf_ = (pf == EF_Unknown) ? texture->Format() : pf;
	first_array_index_ = first_array_index;
	array_size_ = array_size;
	first_level_ = first_level;
	num_levels_ = num_levels;
}

SDL3RenderTargetView::SDL3RenderTargetView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size,
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

SDL3RenderTargetView::SDL3RenderTargetView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem,
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

void SDL3RenderTargetView::ClearColor([[maybe_unused]] Color const& clr) {}
void SDL3RenderTargetView::Discard() {}
void SDL3RenderTargetView::OnAttached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] FrameBuffer::Attachment att) {}
void SDL3RenderTargetView::OnDetached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] FrameBuffer::Attachment att) {}

SDL3DepthStencilView::SDL3DepthStencilView(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
	uint32_t sample_quality)
{
	width_ = width;
	height_ = height;
	pf_ = pf;
	sample_count_ = sample_count;
	sample_quality_ = sample_quality;

	auto tex = MakeSharedPtr<SDL3Texture2D>(width, height, 1, 1, pf, sample_count, sample_quality,
		EAH_GPU_Read | EAH_GPU_Write);
	// Match typical FrameBuffer::Clear depth=1 / stencil=0 (D3D12 OptimizedClearValue).
	float4 const clear_hint(1.0f, 0.0f, 0.0f, 0.0f);
	tex->CreateHWResource({}, &clear_hint);
	tex_ = tex;
	first_array_index_ = 0;
	array_size_ = 1;
	level_ = 0;
}

SDL3DepthStencilView::SDL3DepthStencilView(TexturePtr const& texture, ElementFormat pf, int first_array_index, int array_size,
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

void SDL3DepthStencilView::ClearDepth([[maybe_unused]] float depth) {}
void SDL3DepthStencilView::ClearStencil([[maybe_unused]] int32_t stencil) {}
void SDL3DepthStencilView::ClearDepthStencil([[maybe_unused]] float depth, [[maybe_unused]] int32_t stencil) {}
void SDL3DepthStencilView::Discard() {}
void SDL3DepthStencilView::OnAttached([[maybe_unused]] FrameBuffer& fb) {}
void SDL3DepthStencilView::OnDetached([[maybe_unused]] FrameBuffer& fb) {}

SDL3UnorderedAccessView::SDL3UnorderedAccessView(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	tex_ = texture;
	pf_ = (pf == EF_Unknown) ? texture->Format() : pf;
	first_array_index_ = static_cast<uint32_t>(first_array_index);
	array_size_ = static_cast<uint32_t>(array_size);
	level_ = static_cast<uint32_t>(level);
}

SDL3UnorderedAccessView::SDL3UnorderedAccessView(GraphicsBufferPtr const& gbuffer, ElementFormat pf, uint32_t first_elem,
	uint32_t num_elems)
{
	buff_ = gbuffer;
	pf_ = pf;
	first_elem_ = first_elem;
	num_elems_ = num_elems;
}

void SDL3UnorderedAccessView::Clear([[maybe_unused]] float4 const& val) {}
void SDL3UnorderedAccessView::Clear([[maybe_unused]] uint4 const& val) {}
void SDL3UnorderedAccessView::Discard() {}
void SDL3UnorderedAccessView::OnAttached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] uint32_t att) {}
void SDL3UnorderedAccessView::OnDetached([[maybe_unused]] FrameBuffer& fb, [[maybe_unused]] uint32_t att) {}

} // namespace RenderWorker
