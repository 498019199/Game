#pragma once
#include <render/RenderFactory.h>

namespace RenderWorker
{

class D3D11RenderFactory: public RenderFactory
{
public:
    D3D11RenderFactory();

    RenderLayoutPtr MakeRenderLayout() override;
    
    GraphicsBufferPtr MakeDelayCreationVertexBuffer(BufferUsage usage, uint32_t access_hint,
        uint32_t size_in_byte, uint32_t structure_byte_stride = 0) override;
    GraphicsBufferPtr MakeDelayCreationIndexBuffer(BufferUsage usage, uint32_t access_hint,
        uint32_t size_in_byte, uint32_t structure_byte_stride = 0) override;
    GraphicsBufferPtr MakeDelayCreationConstantBuffer(BufferUsage usage, uint32_t access_hint,
        uint32_t size_in_byte, uint32_t structure_byte_stride = 0) override;
        
    ShaderObjectPtr MakeShaderObject() override;
    ShaderStageObjectPtr MakeShaderStageObject(ShaderStage stage) override;
    RenderStateObjectPtr MakeRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc) override;

    TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
    TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
    TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;
    TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) override;

    SamplerStateObjectPtr MakeSamplerStateObject(const SamplerStateDesc& desc) override;

    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, uint32_t first_level, uint32_t num_levels) override;

    RenderTargetViewPtr Make1DRtv(const  TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) override;
    RenderTargetViewPtr Make2DRtv(const  TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) override;
    RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level) override;
    RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t slice, int level) override;
    RenderTargetViewPtr Make3DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level) override;
    RenderTargetViewPtr MakeCubeRtv(const TexturePtr& texture, ElementFormat pf, int array_index, int level) override;
    RenderTargetViewPtr MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf, uint32_t first_elem,
        uint32_t num_elems) override;

    DepthStencilViewPtr Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
        uint32_t sample_quality) override;
    DepthStencilViewPtr Make1DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) override;
    DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) override;
    DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level) override;
    DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t slice, int level) override;;
    DepthStencilViewPtr Make3DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level) override;
    DepthStencilViewPtr MakeCubeDsv(const TexturePtr& texture, ElementFormat pf, int array_index, int level) override;

    UnorderedAccessViewPtr Make1DUav(TexturePtr const & texture, ElementFormat pf, int first_array_index,
        int array_size, int level) override;
    UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int first_array_index,
        int array_size, int level) override;
    UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
        Texture::CubeFaces face, int level) override;
    UnorderedAccessViewPtr Make2DUav(TexturePtr const & texture, ElementFormat pf, int array_index, uint32_t slice,
        int level) override;
    UnorderedAccessViewPtr Make3DUav(TexturePtr const & texture, ElementFormat pf, int array_index,
        uint32_t first_slice, uint32_t num_slices, int level) override;
    UnorderedAccessViewPtr MakeCubeUav(TexturePtr const & texture, ElementFormat pf, int array_index,
        int level) override;
    UnorderedAccessViewPtr MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf,
        uint32_t first_elem, uint32_t num_elems) override;

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
