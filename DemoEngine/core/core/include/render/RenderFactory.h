#pragma once
#include <render/GraphicsBuffer.h>
#include <render/RenderLayout.h>
#include <render/ShaderObject.h>
#include <render/Texture.h>
#include <render/RenderStateObject.h>
#include <render/RenderView.h>

namespace RenderWorker
{

class RenderFactory
{
public:
    RenderFactory();
    virtual ~RenderFactory() noexcept;

    virtual RenderLayoutPtr MakeRenderLayout() = 0;

    virtual ShaderObjectPtr MakeShaderObject() = 0;
    virtual ShaderStageObjectPtr MakeShaderStageObject(ShaderStage stage) = 0;
    virtual RenderStateObjectPtr MakeRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc) = 0;

    virtual TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;

    virtual SamplerStateObjectPtr MakeSamplerStateObject(const SamplerStateDesc& desc) = 0;

    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture);
    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, ElementFormat pf);
    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels);
    virtual ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, uint32_t first_level, uint32_t num_levels) = 0;

    virtual GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) = 0;
    virtual GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) = 0;
    virtual GraphicsBufferPtr MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) = 0;
};





using RenderFactoryPtr = std::shared_ptr<RenderFactory>;
}
