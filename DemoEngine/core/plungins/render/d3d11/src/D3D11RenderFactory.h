#pragma once
#include <render/RenderFactory.h>

namespace RenderWorker
{

class D3D11RenderFactory: public RenderFactory
{
public:
    D3D11RenderFactory();

    RenderLayoutPtr MakeRenderLayout() override;
    
    ShaderObjectPtr MakeShaderObject() override;
    ShaderStageObjectPtr MakeShaderStageObject(ShaderStage stage) override;
    RenderStateObjectPtr MakeRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc) override;

    TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;

    SamplerStateObjectPtr MakeSamplerStateObject(const SamplerStateDesc& desc) override;

    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, uint32_t first_level, uint32_t num_levels) override;

    GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) override;
    GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) override;
    GraphicsBufferPtr MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) override;

private:
    std::unique_ptr<RenderEngine> DoMakeRenderEngine() override;
    
private:
	D3D11RenderFactory(D3D11RenderFactory const & rhs);
	D3D11RenderFactory& operator=(D3D11RenderFactory const & rhs);
};
}
