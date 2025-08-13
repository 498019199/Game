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

TexturePtr D3D11RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
{
    TexturePtr ret =  MakeSharedPtr<D3D11Texture2D>(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
    return ret;
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
	void MakeRenderFactory(std::unique_ptr<RenderWorker::RenderFactory>& ptr)
	{
		ptr = CommonWorker::MakeUniquePtr<RenderWorker::D3D11RenderFactory>();
	}
}
