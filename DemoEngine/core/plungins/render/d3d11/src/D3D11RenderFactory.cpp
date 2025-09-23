#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderFactory.h"
#include "D3D11RenderLayout.h"
#include "D3D11ShaderObject.h"
#include "D3D11Texture.h"
#include "D3D11RenderStateObject.h"
#include "D3D11RenderView.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
D3D11RenderFactory::D3D11RenderFactory() = default;

RenderLayoutPtr D3D11RenderFactory::MakeRenderLayout()
{
    return MakeSharedPtr<D3D11RenderLayout>();
}

GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
    uint32_t size_in_byte, uint32_t structure_byte_stride)
{
    return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, size_in_byte, structure_byte_stride);
}

GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
    uint32_t size_in_byte, uint32_t structure_byte_stride)
{
    return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, size_in_byte, structure_byte_stride);
}

GraphicsBufferPtr D3D11RenderFactory::MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
    uint32_t size_in_byte, uint32_t structure_byte_stride)
{
    return MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_CONSTANT_BUFFER, size_in_byte, structure_byte_stride);
}

ShaderObjectPtr D3D11RenderFactory::MakeShaderObject()
{
    return MakeSharedPtr<D3D11ShaderObject>();
}

ShaderStageObjectPtr D3D11RenderFactory::MakeShaderStageObject(ShaderStage stage)
{
    ShaderStageObjectPtr ret;
    switch (stage)
    {
    case ShaderStage::Vertex:
        ret = MakeSharedPtr<D3D11VertexShaderStageObject>();
        break;

    case ShaderStage::Pixel:
        ret = MakeSharedPtr<D3D11PixelShaderStageObject>();
        break;

    case ShaderStage::Geometry:
        ret = MakeSharedPtr<D3D11GeometryShaderStageObject>();
        break;

    case ShaderStage::Compute:
        //ret = MakeSharedPtr<D3D11ComputeShaderStageObject>();
        break;

    case ShaderStage::Hull:
        //ret = MakeSharedPtr<D3D11HullShaderStageObject>();
        break;

    case ShaderStage::Domain:
        //ret = MakeSharedPtr<D3D11DomainShaderStageObject>();
        break;

    default:
        ZENGINE_UNREACHABLE("Invalid shader stage");
    }
    return ret;
}

RenderStateObjectPtr D3D11RenderFactory::MakeRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc)
{
    return MakeSharedPtr<D3D11RenderStateObject>(rs_desc, dss_desc, bs_desc);
}

TexturePtr D3D11RenderFactory::MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
    return MakeSharedPtr<D3D11Texture1D>(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
}

TexturePtr D3D11RenderFactory::MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
    return MakeSharedPtr<D3D11Texture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
}

TexturePtr D3D11RenderFactory::MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
    return MakeSharedPtr<D3D11Texture3D>(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
}

TexturePtr D3D11RenderFactory::MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
    return MakeSharedPtr<D3D11TextureCube>(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
}

SamplerStateObjectPtr D3D11RenderFactory::MakeSamplerStateObject(const SamplerStateDesc& desc)
{
    return MakeSharedPtr<D3D11SamplerStateObject>(desc);
}

ShaderResourceViewPtr D3D11RenderFactory::MakeTextureSrv(const TexturePtr& texture, ElementFormat pf, uint32_t first_array_index,
            uint32_t array_size, uint32_t first_level, uint32_t num_levels)
{
    return MakeSharedPtr<D3D11TextureShaderResourceView>(texture, pf, first_array_index, array_size, first_level, num_levels);
}

RenderTargetViewPtr D3D11RenderFactory::Make1DRtv(const  TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level)
{
    return MakeSharedPtr<D3D11Texture1D2DCubeRenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr D3D11RenderFactory::Make2DRtv(const  TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level)
{
    return MakeSharedPtr<D3D11Texture1D2DCubeRenderTargetView>(texture, pf, first_array_index, array_size, level);
}

RenderTargetViewPtr D3D11RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level)
{
    return MakeSharedPtr<D3D11TextureCubeFaceRenderTargetView>(texture, pf, array_index, face, level);
}

RenderTargetViewPtr D3D11RenderFactory::Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t slice, int level)
{
    return this->Make3DRtv(texture, pf, array_index, slice, 1, level);
}

RenderTargetViewPtr D3D11RenderFactory::Make3DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level)
{
    return MakeSharedPtr<D3D11Texture3DRenderTargetView>(texture, pf, array_index, first_slice, num_slices, level);
}

RenderTargetViewPtr D3D11RenderFactory::MakeCubeRtv(const TexturePtr& texture, ElementFormat pf, int array_index, int level)
{
    int array_size = 1;
    return MakeSharedPtr<D3D11Texture1D2DCubeRenderTargetView>(texture, pf, array_index, array_size, level);
}

RenderTargetViewPtr D3D11RenderFactory::MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf, uint32_t first_elem,
        uint32_t num_elems)
{
    return MakeSharedPtr<D3D11BufferRenderTargetView>(gbuffer, pf, first_elem, num_elems);
}

DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
        uint32_t sample_quality)
{
    return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(width, height, pf, sample_count, sample_quality);
}

DepthStencilViewPtr D3D11RenderFactory::Make1DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level)
{
    return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level)
{
    return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(texture, pf, first_array_index, array_size, level);
}

DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level)
{
    return MakeSharedPtr<D3D11TextureCubeFaceDepthStencilView>(texture, pf, array_index, face, level);
}

DepthStencilViewPtr D3D11RenderFactory::Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t slice, int level)
{
    return this->Make3DDsv(texture, pf, array_index, slice, 1, level);
}

DepthStencilViewPtr D3D11RenderFactory::Make3DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level)
{
    return MakeSharedPtr<D3D11Texture3DDepthStencilView>(texture, pf, array_index, first_slice, num_slices, level);
}

DepthStencilViewPtr D3D11RenderFactory::MakeCubeDsv(const TexturePtr& texture, ElementFormat pf, int array_index, int level)
{
    int array_size = 1;
    return MakeSharedPtr<D3D11Texture1D2DCubeDepthStencilView>(texture, pf, array_index, array_size, level);
}

GraphicsBufferPtr D3D11RenderFactory::MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_VERTEX_BUFFER, size_in_byte, structure_byte_stride);
    ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr D3D11RenderFactory::MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_INDEX_BUFFER, size_in_byte, structure_byte_stride);
    ret->CreateHWResource(init_data);
	return ret;
}

GraphicsBufferPtr D3D11RenderFactory::MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride)
{
	auto ret = MakeSharedPtr<D3D11GraphicsBuffer>(usage, access_hint, D3D11_BIND_CONSTANT_BUFFER, size_in_byte, structure_byte_stride);
    ret->CreateHWResource(init_data);
	return ret;
}

std::unique_ptr<RenderEngine> D3D11RenderFactory::DoMakeRenderEngine()
{
    return MakeUniquePtr<D3D11RenderEngine>();
}
}

extern "C"
{
	ZENGINE_SYMBOL_EXPORT void MakeRenderFactory(std::unique_ptr<RenderWorker::RenderFactory>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::D3D11RenderFactory>();
	}
}
