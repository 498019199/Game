#include "MetalRenderFactory.h"
#include "MetalRenderEngine.h"
#include "MetalFrameBuffer.h"
#include "MetalRenderStateObject.h"
#include "MetalShaderObject.h"
#include "MetalRenderView.h"
#include "MetalFence.h"
#include "MetalQuery.h"
#include "MetalGraphicsBuffer.h"
#include "MetalTexture.h"
#include "MetalRenderLayout.h"
#include <render/Texture.h>

namespace RenderWorker
{

MetalRenderFactory::MetalRenderFactory() = default;

RenderLayoutPtr MetalRenderFactory::MakeRenderLayout()
{
	return MakeSharedPtr<MetalRenderLayout>();
}

FrameBufferPtr MetalRenderFactory::MakeFrameBuffer()
{
	return MakeSharedPtr<MetalFrameBuffer>();
}

GraphicsBufferPtr MetalRenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
{
	return MakeSharedPtr<MetalGraphicsBuffer>(usage, access_hint, MetalBufferBind::Vertex, size_in_byte,
		structure_byte_stride);
}

GraphicsBufferPtr MetalRenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
{
	return MakeSharedPtr<MetalGraphicsBuffer>(usage, access_hint, MetalBufferBind::Index, size_in_byte,
		structure_byte_stride);
}

GraphicsBufferPtr MetalRenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
{
	return MakeSharedPtr<MetalGraphicsBuffer>(usage, access_hint, MetalBufferBind::Constant, size_in_byte,
		structure_byte_stride);
}

ShaderObjectPtr MetalRenderFactory::MakeShaderObject()
{
	return MakeSharedPtr<MetalShaderObject>();
}

ShaderStageObjectPtr MetalRenderFactory::MakeShaderStageObject(ShaderStage stage)
{
	// Always return an object so RenderPass::CompileShaders/CreateHwShaders can call
	// through safely. Non-VS/PS stages no-op inside MetalShaderStageObject (MVP).
	return MakeSharedPtr<MetalShaderStageObject>(stage);
}

RenderStateObjectPtr MetalRenderFactory::MakeRenderStateObject(const RasterizerStateDesc& rs_desc,
	const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc)
{
	return MakeSharedPtr<MetalRenderStateObject>(rs_desc, dss_desc, bs_desc);
}

TexturePtr MetalRenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
	ElementFormat format, [[maybe_unused]] uint32_t sample_count, [[maybe_unused]] uint32_t sample_quality,
	[[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_1D, width, 1, 1, num_mip_maps, array_size, format, false);
}

TexturePtr MetalRenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps,
	uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
	return MakeSharedPtr<MetalTexture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality,
		access_hint);
}

TexturePtr MetalRenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth,
	uint32_t num_mip_maps, uint32_t array_size, ElementFormat format, [[maybe_unused]] uint32_t sample_count,
	[[maybe_unused]] uint32_t sample_quality, [[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_3D, width, height, depth, num_mip_maps, array_size, format, false);
}

	TexturePtr MetalRenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
		ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<MetalTextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality,
			access_hint);
	}

SamplerStateObjectPtr MetalRenderFactory::MakeSamplerStateObject(const SamplerStateDesc& desc)
{
	return MakeSharedPtr<MetalSamplerStateObject>(desc);
}

ShaderResourceViewPtr MetalRenderFactory::MakeTextureSrv(const TexturePtr& texture, ElementFormat pf,
	uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
	return MakeSharedPtr<MetalShaderResourceView>(texture, pf, first_array_index, array_size, first_level, num_levels);
}

RenderTargetViewPtr MetalRenderFactory::Make1DRtv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<MetalRenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr MetalRenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<MetalRenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr MetalRenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<MetalRenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr MetalRenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DRtv(texture, pf, array_index, slice, 1, level);
}

RenderTargetViewPtr MetalRenderFactory::Make3DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<MetalRenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr MetalRenderFactory::MakeCubeRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<MetalRenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr MetalRenderFactory::MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf,
	uint32_t first_elem, uint32_t num_elems)
{
	return MakeSharedPtr<MetalRenderTargetView>(gbuffer, pf, first_elem, num_elems);
}

DepthStencilViewPtr MetalRenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf,
	uint32_t sample_count, uint32_t sample_quality)
{
	return MakeSharedPtr<MetalDepthStencilView>(width, height, pf, sample_count, sample_quality);
}

DepthStencilViewPtr MetalRenderFactory::Make1DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<MetalDepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr MetalRenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<MetalDepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr MetalRenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<MetalDepthStencilView>(texture, pf, array_index, 1, level);
}

DepthStencilViewPtr MetalRenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DDsv(texture, pf, array_index, slice, 1, level);
}

DepthStencilViewPtr MetalRenderFactory::Make3DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<MetalDepthStencilView>(texture, pf, array_index, 1, level);
}

DepthStencilViewPtr MetalRenderFactory::MakeCubeDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<MetalDepthStencilView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr MetalRenderFactory::Make1DUav(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<MetalUnorderedAccessView>(texture, pf, first_array_index, array_size, level);
}

UnorderedAccessViewPtr MetalRenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<MetalUnorderedAccessView>(texture, pf, first_array_index, array_size, level);
}

UnorderedAccessViewPtr MetalRenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<MetalUnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr MetalRenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DUav(texture, pf, array_index, slice, 1, level);
}

UnorderedAccessViewPtr MetalRenderFactory::Make3DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<MetalUnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr MetalRenderFactory::MakeCubeUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<MetalUnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr MetalRenderFactory::MakeBufferUav(GraphicsBufferPtr const& gbuffer, ElementFormat pf,
	uint32_t first_elem, uint32_t num_elems)
{
	return MakeSharedPtr<MetalUnorderedAccessView>(gbuffer, pf, first_elem, num_elems);
}

GraphicsBufferPtr MetalRenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
	void const* init_data, uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<MetalGraphicsBuffer>(usage, access_hint, MetalBufferBind::Vertex, size_in_byte,
		structure_byte_stride);
	ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr MetalRenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
	void const* init_data, uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<MetalGraphicsBuffer>(usage, access_hint, MetalBufferBind::Index, size_in_byte,
		structure_byte_stride);
	ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr MetalRenderFactory::MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
	void const* init_data, uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<MetalGraphicsBuffer>(usage, access_hint, MetalBufferBind::Constant, size_in_byte,
		structure_byte_stride);
	ret->CreateHWResource(init_data);
	return ret;
}

QueryPtr MetalRenderFactory::MakeConditionalRender()
{
	return MakeSharedPtr<MetalConditionalRender>();
}

FencePtr MetalRenderFactory::MakeFence()
{
	return MakeSharedPtr<MetalFence>();
}

std::unique_ptr<RenderEngine> MetalRenderFactory::DoMakeRenderEngine()
{
	return MakeUniquePtr<MetalRenderEngine>();
}

} // namespace RenderWorker

extern "C"
{
	ZENGINE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<RenderWorker::RenderFactory>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::MetalRenderFactory>();
	}
}
