#include "D3D12RenderFactory.h"
#include "D3D12RenderEngine.h"
#include "D3D12FrameBuffer.h"
#include "D3D12RenderStateObject.h"
#include "D3D12ShaderObject.h"
#include "D3D12RenderView.h"
#include "D3D12Fence.h"
#include "D3D12Query.h"
#include "D3D12GraphicsBuffer.h"
#include "D3D12Texture.h"
#include "D3D12RenderLayout.h"
#include <render/Texture.h>

namespace RenderWorker
{

D3D12RenderFactory::D3D12RenderFactory() = default;

RenderLayoutPtr D3D12RenderFactory::MakeRenderLayout()
{
	return MakeSharedPtr<D3D12RenderLayout>();
}

FrameBufferPtr D3D12RenderFactory::MakeFrameBuffer()
{
	return MakeSharedPtr<D3D12FrameBuffer>();
}

GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
{
	return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, D3D12BufferBind::Vertex, size_in_byte,
		structure_byte_stride);
}

GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
{
	return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, D3D12BufferBind::Index, size_in_byte,
		structure_byte_stride);
}

GraphicsBufferPtr D3D12RenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
{
	return MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, D3D12BufferBind::Constant, size_in_byte,
		structure_byte_stride);
}

ShaderObjectPtr D3D12RenderFactory::MakeShaderObject()
{
	return MakeSharedPtr<D3D12ShaderObject>();
}

ShaderStageObjectPtr D3D12RenderFactory::MakeShaderStageObject(ShaderStage stage)
{
	// Always return an object so RenderPass::CompileShaders/CreateHwShaders can call
	// through safely. Non-VS/PS stages no-op inside D3D12ShaderStageObject (MVP).
	return MakeSharedPtr<D3D12ShaderStageObject>(stage);
}

RenderStateObjectPtr D3D12RenderFactory::MakeRenderStateObject(const RasterizerStateDesc& rs_desc,
	const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc)
{
	return MakeSharedPtr<D3D12RenderStateObject>(rs_desc, dss_desc, bs_desc);
}

TexturePtr D3D12RenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
	ElementFormat format, [[maybe_unused]] uint32_t sample_count, [[maybe_unused]] uint32_t sample_quality,
	[[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_1D, width, 1, 1, num_mip_maps, array_size, format, false);
}

TexturePtr D3D12RenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps,
	uint32_t array_size, ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
	return MakeSharedPtr<D3D12Texture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality,
		access_hint);
}

TexturePtr D3D12RenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth,
	uint32_t num_mip_maps, uint32_t array_size, ElementFormat format, [[maybe_unused]] uint32_t sample_count,
	[[maybe_unused]] uint32_t sample_quality, [[maybe_unused]] uint32_t access_hint)
{
	return MakeSharedPtr<VirtualTexture>(Texture::TT_3D, width, height, depth, num_mip_maps, array_size, format, false);
}

	TexturePtr D3D12RenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
		ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	{
		return MakeSharedPtr<D3D12TextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality,
			access_hint);
	}

SamplerStateObjectPtr D3D12RenderFactory::MakeSamplerStateObject(const SamplerStateDesc& desc)
{
	return MakeSharedPtr<D3D12SamplerStateObject>(desc);
}

ShaderResourceViewPtr D3D12RenderFactory::MakeTextureSrv(const TexturePtr& texture, ElementFormat pf,
	uint32_t first_array_index, uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
	return MakeSharedPtr<D3D12ShaderResourceView>(texture, pf, first_array_index, array_size, first_level, num_levels);
}

RenderTargetViewPtr D3D12RenderFactory::Make1DRtv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<D3D12RenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr D3D12RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<D3D12RenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr D3D12RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<D3D12RenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr D3D12RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DRtv(texture, pf, array_index, slice, 1, level);
}

RenderTargetViewPtr D3D12RenderFactory::Make3DRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<D3D12RenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr D3D12RenderFactory::MakeCubeRtv(const TexturePtr& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<D3D12RenderTargetView>(texture, pf, array_index, 1, level);
}

RenderTargetViewPtr D3D12RenderFactory::MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf,
	uint32_t first_elem, uint32_t num_elems)
{
	return MakeSharedPtr<D3D12RenderTargetView>(gbuffer, pf, first_elem, num_elems);
}

DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf,
	uint32_t sample_count, uint32_t sample_quality)
{
	return MakeSharedPtr<D3D12DepthStencilView>(width, height, pf, sample_count, sample_quality);
}

DepthStencilViewPtr D3D12RenderFactory::Make1DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<D3D12DepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<D3D12DepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<D3D12DepthStencilView>(texture, pf, array_index, 1, level);
}

DepthStencilViewPtr D3D12RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DDsv(texture, pf, array_index, slice, 1, level);
}

DepthStencilViewPtr D3D12RenderFactory::Make3DDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<D3D12DepthStencilView>(texture, pf, array_index, 1, level);
}

DepthStencilViewPtr D3D12RenderFactory::MakeCubeDsv(const TexturePtr& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<D3D12DepthStencilView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::Make1DUav(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<D3D12UnorderedAccessView>(texture, pf, first_array_index, array_size, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int first_array_index,
	int array_size, int level)
{
	return MakeSharedPtr<D3D12UnorderedAccessView>(texture, pf, first_array_index, array_size, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] Texture::CubeFaces face, int level)
{
	return MakeSharedPtr<D3D12UnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::Make2DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	uint32_t slice, int level)
{
	return this->Make3DUav(texture, pf, array_index, slice, 1, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::Make3DUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	[[maybe_unused]] uint32_t first_slice, [[maybe_unused]] uint32_t num_slices, int level)
{
	return MakeSharedPtr<D3D12UnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::MakeCubeUav(TexturePtr const& texture, ElementFormat pf, int array_index,
	int level)
{
	return MakeSharedPtr<D3D12UnorderedAccessView>(texture, pf, array_index, 1, level);
}

UnorderedAccessViewPtr D3D12RenderFactory::MakeBufferUav(GraphicsBufferPtr const& gbuffer, ElementFormat pf,
	uint32_t first_elem, uint32_t num_elems)
{
	return MakeSharedPtr<D3D12UnorderedAccessView>(gbuffer, pf, first_elem, num_elems);
}

GraphicsBufferPtr D3D12RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
	void const* init_data, uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, D3D12BufferBind::Vertex, size_in_byte,
		structure_byte_stride);
	ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr D3D12RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
	void const* init_data, uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, D3D12BufferBind::Index, size_in_byte,
		structure_byte_stride);
	ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr D3D12RenderFactory::MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte,
	void const* init_data, uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<D3D12GraphicsBuffer>(usage, access_hint, D3D12BufferBind::Constant, size_in_byte,
		structure_byte_stride);
	ret->CreateHWResource(init_data);
	return ret;
}

QueryPtr D3D12RenderFactory::MakeConditionalRender()
{
	return MakeSharedPtr<D3D12ConditionalRender>();
}

FencePtr D3D12RenderFactory::MakeFence()
{
	return MakeSharedPtr<D3D12Fence>();
}

std::unique_ptr<RenderEngine> D3D12RenderFactory::DoMakeRenderEngine()
{
	return MakeUniquePtr<D3D12RenderEngine>();
}

} // namespace RenderWorker

extern "C"
{
	ZENGINE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<RenderWorker::RenderFactory>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::D3D12RenderFactory>();
	}
}
