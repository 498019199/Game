#include "SDL3RenderFactory.h"
#include "SDL3RenderEngine.h"
#include "SDL3FrameBuffer.h"
#include "SDL3RenderStateObject.h"
#include "SDL3ShaderObject.h"
#include "SDL3RenderView.h"
#include "SDL3Fence.h"
#include "SDL3Query.h"
#include <render/GraphicsBuffer.h>
#include <render/Texture.h>
#include <render/RenderLayout.h>

namespace RenderWorker
{

SDL3RenderFactory::SDL3RenderFactory() = default;

RenderLayoutPtr SDL3RenderFactory::MakeRenderLayout()
{
	return MakeSharedPtr<RenderLayout>();
}

FrameBufferPtr SDL3RenderFactory::MakeFrameBuffer()
{
	return MakeSharedPtr<SDL3FrameBuffer>();
}

GraphicsBufferPtr SDL3RenderFactory::MakeDelayCreationVertexBuffer([[maybe_unused]] BufferUsage usage,
	[[maybe_unused]] uint32_t access_hint, uint32_t size_in_byte, [[maybe_unused]] uint32_t structure_byte_stride)
{
	return MakeSharedPtr<SoftwareGraphicsBuffer>(size_in_byte, false);
}

GraphicsBufferPtr SDL3RenderFactory::MakeDelayCreationIndexBuffer([[maybe_unused]] BufferUsage usage,
	[[maybe_unused]] uint32_t access_hint, uint32_t size_in_byte, [[maybe_unused]] uint32_t structure_byte_stride)
{
	return MakeSharedPtr<SoftwareGraphicsBuffer>(size_in_byte, false);
}

GraphicsBufferPtr SDL3RenderFactory::MakeDelayCreationConstantBuffer([[maybe_unused]] BufferUsage usage,
	[[maybe_unused]] uint32_t access_hint, uint32_t size_in_byte, [[maybe_unused]] uint32_t structure_byte_stride)
{
	return MakeSharedPtr<SoftwareGraphicsBuffer>(size_in_byte, false);
}

ShaderObjectPtr SDL3RenderFactory::MakeShaderObject()
{
	return MakeSharedPtr<SDL3ShaderObject>();
}

ShaderStageObjectPtr SDL3RenderFactory::MakeShaderStageObject(ShaderStage stage)
{
	return MakeSharedPtr<SDL3ShaderStageObject>(stage);
}

RenderStateObjectPtr SDL3RenderFactory::MakeRenderStateObject(const RasterizerStateDesc& rs_desc,
	const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc)
{
	return MakeSharedPtr<SDL3RenderStateObject>(rs_desc, dss_desc, bs_desc);
}

TexturePtr SDL3RenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
	ElementFormat format, [[maybe_unused]] uint32_t sample_count, [[maybe_unused]] uint32_t sample_quality,
	[[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_1D, width, 1, 1, num_mip_maps, array_size, format, false);
}

TexturePtr SDL3RenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps,
	uint32_t array_size, ElementFormat format, [[maybe_unused]] uint32_t sample_count,
	[[maybe_unused]] uint32_t sample_quality, [[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_2D, width, height, 1, num_mip_maps, array_size, format, false);
}

TexturePtr SDL3RenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth,
	uint32_t num_mip_maps, uint32_t array_size, ElementFormat format, [[maybe_unused]] uint32_t sample_count,
	[[maybe_unused]] uint32_t sample_quality, [[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_3D, width, height, depth, num_mip_maps, array_size, format, false);
}

TexturePtr SDL3RenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
	ElementFormat format, [[maybe_unused]] uint32_t sample_count, [[maybe_unused]] uint32_t sample_quality,
	[[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_Cube, size, size, 1, num_mip_maps, array_size, format, false);
}

SamplerStateObjectPtr SDL3RenderFactory::MakeSamplerStateObject(const SamplerStateDesc& desc)
{
	return MakeSharedPtr<SDL3SamplerStateObject>(desc);
}

ShaderResourceViewPtr SDL3RenderFactory::MakeTextureSrv(const TexturePtr& texture, ElementFormat pf,
	uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
	return MakeSharedPtr<SDL3ShaderResourceView>(texture, pf, first_array_index, array_size, first_level, num_levels);
}

RenderTargetViewPtr SDL3RenderFactory::Make1DRtv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<SDL3RenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr SDL3RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<SDL3RenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr SDL3RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<SDL3RenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr SDL3RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DRtv(texture, pf, array_index, slice, 1, level);
}

RenderTargetViewPtr SDL3RenderFactory::Make3DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<SDL3RenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr SDL3RenderFactory::MakeCubeRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<SDL3RenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr SDL3RenderFactory::MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf,
	uint32_t first_elem, uint32_t num_elems)
{
	return MakeSharedPtr<SDL3RenderTargetView>(gbuffer, pf, first_elem, num_elems);
}

DepthStencilViewPtr SDL3RenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf,
	uint32_t sample_count, uint32_t sample_quality)
{
	return MakeSharedPtr<SDL3DepthStencilView>(width, height, pf, sample_count, sample_quality);
}

DepthStencilViewPtr SDL3RenderFactory::Make1DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<SDL3DepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr SDL3RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<SDL3DepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr SDL3RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<SDL3DepthStencilView>(texture, pf, array_index, 1, level);
}

DepthStencilViewPtr SDL3RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DDsv(texture, pf, array_index, slice, 1, level);
}

DepthStencilViewPtr SDL3RenderFactory::Make3DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<SDL3DepthStencilView>(texture, pf, array_index, 1, level);
}

DepthStencilViewPtr SDL3RenderFactory::MakeCubeDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<SDL3DepthStencilView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::Make1DUav(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<SDL3UnorderedAccessView>(texture, pf, first_array_index, array_size, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<SDL3UnorderedAccessView>(texture, pf, first_array_index, array_size, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<SDL3UnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DUav(texture, pf, array_index, slice, 1, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::Make3DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<SDL3UnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::MakeCubeUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<SDL3UnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr SDL3RenderFactory::MakeBufferUav(GraphicsBufferPtr const& gbuffer, ElementFormat pf,
	uint32_t first_elem, uint32_t num_elems)
{
	return MakeSharedPtr<SDL3UnorderedAccessView>(gbuffer, pf, first_elem, num_elems);
}

GraphicsBufferPtr SDL3RenderFactory::MakeVertexBuffer([[maybe_unused]] BufferUsage usage,
	[[maybe_unused]] uint32_t access_hint, uint32_t size_in_byte, void const* init_data,
	[[maybe_unused]] uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<SoftwareGraphicsBuffer>(size_in_byte, false);
	ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr SDL3RenderFactory::MakeIndexBuffer([[maybe_unused]] BufferUsage usage,
	[[maybe_unused]] uint32_t access_hint, uint32_t size_in_byte, void const* init_data,
	[[maybe_unused]] uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<SoftwareGraphicsBuffer>(size_in_byte, false);
	ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr SDL3RenderFactory::MakeConstantBuffer([[maybe_unused]] BufferUsage usage,
	[[maybe_unused]] uint32_t access_hint, uint32_t size_in_byte, void const* init_data,
	[[maybe_unused]] uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<SoftwareGraphicsBuffer>(size_in_byte, false);
	ret->CreateHWResource(init_data);
	return ret;
}

QueryPtr SDL3RenderFactory::MakeConditionalRender()
{
	return MakeSharedPtr<SDL3ConditionalRender>();
}

FencePtr SDL3RenderFactory::MakeFence()
{
	return MakeSharedPtr<SDL3Fence>();
}

std::unique_ptr<RenderEngine> SDL3RenderFactory::DoMakeRenderEngine()
{
	return MakeUniquePtr<SDL3RenderEngine>();
}

} // namespace RenderWorker

extern "C"
{
	ZENGINE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<RenderWorker::RenderFactory>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::SDL3RenderFactory>();
	}
}
