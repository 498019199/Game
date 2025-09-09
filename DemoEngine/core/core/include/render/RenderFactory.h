#pragma once
#include <render/GraphicsBuffer.h>
#include <render/RenderLayout.h>
#include <render/ShaderObject.h>
#include <render/Texture.h>
#include <render/RenderStateObject.h>
#include <render/RenderView.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{

class RenderFactory
{
public:
    RenderFactory();
    virtual ~RenderFactory() noexcept;

    RenderEngine& RenderEngineInstance();
    virtual RenderLayoutPtr MakeRenderLayout() = 0;

    virtual ShaderObjectPtr MakeShaderObject() = 0;
    virtual ShaderStageObjectPtr MakeShaderStageObject(ShaderStage stage) = 0;
    virtual RenderStateObjectPtr MakeRenderStateObject(const RasterizerStateDesc& rs_desc, const DepthStencilStateDesc& dss_desc, const BlendStateDesc& bs_desc) = 0;

    virtual TexturePtr MakeDelayCreationTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
    virtual TexturePtr MakeDelayCreationTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
    virtual TexturePtr MakeDelayCreationTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;
    virtual TexturePtr MakeDelayCreationTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint) = 0;

    TexturePtr MakeTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data = {}, float4 const * clear_value_hint = nullptr);
    TexturePtr MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data = {}, float4 const * clear_value_hint = nullptr);
    TexturePtr MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data = {}, float4 const * clear_value_hint = nullptr);
    TexturePtr MakeTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data = {}, float4 const * clear_value_hint = nullptr);

    virtual SamplerStateObjectPtr MakeSamplerStateObject(const SamplerStateDesc& desc) = 0;

    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture);
    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, ElementFormat pf);
    ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, uint32_t first_array_index, uint32_t array_size,
		uint32_t first_level, uint32_t num_levels);
    virtual ShaderResourceViewPtr MakeTextureSrv(const TexturePtr& texture, ElementFormat pf, uint32_t first_array_index,
			uint32_t array_size, uint32_t first_level, uint32_t num_levels) = 0;

    RenderTargetViewPtr Make1DRtv(const TexturePtr& texture, int first_array_index, int array_size, int level);
    virtual RenderTargetViewPtr Make1DRtv(const  TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) = 0;
    RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, int first_array_index, int array_size, int level);
    virtual RenderTargetViewPtr Make2DRtv(const  TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) = 0;
    RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, int array_index, Texture::CubeFaces face, int level);
    virtual RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level) = 0;
    RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, int array_index, uint32_t slice, int level);
    virtual RenderTargetViewPtr Make2DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t slice, int level) = 0;
    RenderTargetViewPtr Make3DRtv(const TexturePtr& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
    virtual RenderTargetViewPtr Make3DRtv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level) = 0;
    RenderTargetViewPtr MakeCubeRtv(const TexturePtr& texture, int array_index, int level);
    virtual RenderTargetViewPtr MakeCubeRtv(const TexturePtr& texture, ElementFormat pf, int array_index, int level) = 0;
    virtual RenderTargetViewPtr MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf, uint32_t first_elem,
        uint32_t num_elems) = 0;
    RenderTargetViewPtr MakeTextureRtv(const TexturePtr& texture, uint32_t level);
    RenderTargetViewPtr MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf);

    virtual DepthStencilViewPtr Make2DDsv(uint32_t width, uint32_t height, ElementFormat pf, uint32_t sample_count,
        uint32_t sample_quality) = 0;
    DepthStencilViewPtr Make1DDsv(const TexturePtr& texture, int first_array_index, int array_size, int level);
    virtual DepthStencilViewPtr Make1DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) = 0;
    DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, int first_array_index, int array_size, int level);
    virtual DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, ElementFormat pf, int first_array_index, int array_size,
        int level) = 0;
    DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, int array_index, Texture::CubeFaces face, int level);
    virtual DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, Texture::CubeFaces face,
        int level) = 0;
    DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, int array_index, uint32_t slice, int level);
    virtual DepthStencilViewPtr Make2DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t slice, int level) = 0;
    DepthStencilViewPtr Make3DDsv(const TexturePtr& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level);
    virtual DepthStencilViewPtr Make3DDsv(const TexturePtr& texture, ElementFormat pf, int array_index, uint32_t first_slice,
        uint32_t num_slices, int level) = 0;
    DepthStencilViewPtr MakeCubeDsv(const TexturePtr& texture, int array_index, int level);
    virtual DepthStencilViewPtr MakeCubeDsv(const TexturePtr& texture, ElementFormat pf, int array_index, int level) = 0;
    DepthStencilViewPtr MakeTextureDsv(const TexturePtr& texture, uint32_t level);
    
    virtual GraphicsBufferPtr MakeVertexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) = 0;
    virtual GraphicsBufferPtr MakeIndexBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) = 0;
    virtual GraphicsBufferPtr MakeConstantBuffer(BufferUsage usage, uint32_t access_hint, uint32_t size_in_byte, void const * init_data,
        uint32_t structure_byte_stride = 0) = 0;

private:
    virtual std::unique_ptr<RenderEngine> DoMakeRenderEngine() = 0;

protected:
    std::unique_ptr<RenderEngine> re_;
};



using RenderFactoryPtr = std::shared_ptr<RenderFactory>;
}
