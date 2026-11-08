#include <render/RenderFactory.h>


namespace RenderWorker
{

RenderFactory::RenderFactory() = default;

RenderFactory::~RenderFactory() noexcept
{
    re_.reset();
}

RenderEngine& RenderFactory::RenderEngineInstance()
{
    if (!re_)
    {
        re_ = this->DoMakeRenderEngine();
    }

    return *re_;
}

TexturePtr RenderFactory::MakeTexture1D(uint32_t width, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    TexturePtr ret = this->MakeDelayCreationTexture1D(width, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
    ret->CreateHWResource(init_data, clear_value_hint);
    return ret;
}

TexturePtr RenderFactory::MakeTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    TexturePtr ret = this->MakeDelayCreationTexture2D(width, height, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
    ret->CreateHWResource(init_data, clear_value_hint);
    return ret;
}

TexturePtr RenderFactory::MakeTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    TexturePtr ret = this->MakeDelayCreationTexture3D(width, height, depth, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
    ret->CreateHWResource(init_data, clear_value_hint);
    return ret;
}

TexturePtr RenderFactory::MakeTextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size,
        ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint,
        std::span<ElementInitData const> init_data, float4 const * clear_value_hint)
{
    TexturePtr ret = this->MakeDelayCreationTextureCube(size, num_mip_maps, array_size, format, sample_count, sample_quality, access_hint);
    ret->CreateHWResource(init_data, clear_value_hint);
    return ret;
}

ShaderResourceViewPtr RenderFactory::MakeTextureSrv(const TexturePtr& texture)
{
    return this->MakeTextureSrv(texture, texture->Format(), 0, texture->ArraySize(), 0, texture->MipMapsNum());
}

ShaderResourceViewPtr RenderFactory::MakeTextureSrv(const TexturePtr& texture, ElementFormat pf)
{
    return this->MakeTextureSrv(texture, pf, 0, texture->ArraySize(), 0, texture->MipMapsNum());
}

ShaderResourceViewPtr RenderFactory::MakeTextureSrv(const TexturePtr& texture, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels)
{
    return this->MakeTextureSrv(texture, texture->Format(), first_array_index, array_size, first_level, num_levels);
}

RenderTargetViewPtr RenderFactory::Make1DRtv(const TexturePtr& texture, int first_array_index, int array_size, int level)
{
    return this->Make1DRtv(texture, texture->Format(), first_array_index, array_size, level);
}

RenderTargetViewPtr RenderFactory::Make2DRtv(const TexturePtr& texture, int first_array_index, int array_size, int level)
{
    return this->Make2DRtv(texture, texture->Format(), first_array_index, array_size, level);
}

RenderTargetViewPtr RenderFactory::Make2DRtv(const TexturePtr& texture, int array_index, Texture::CubeFaces face, int level)
{
    return this->Make2DRtv(texture, texture->Format(), array_index, face, level);
}

RenderTargetViewPtr RenderFactory::Make2DRtv(const TexturePtr& texture, int array_index, uint32_t slice, int level)
{
	return this->Make2DRtv(texture, texture->Format(), array_index, slice, level);
}

RenderTargetViewPtr RenderFactory::Make3DRtv(const TexturePtr& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
{
    return this->Make3DRtv(texture, texture->Format(), array_index, first_slice, num_slices, level);
}

RenderTargetViewPtr RenderFactory::MakeCubeRtv(const TexturePtr& texture, int array_index, int level)
{
    return this->MakeCubeRtv(texture, texture->Format(), array_index, level);
}

RenderTargetViewPtr RenderFactory::MakeTextureRtv(const TexturePtr& texture, uint32_t level)
{
    switch (texture->Type())
    {
    case Texture::TT_1D:
        return this->Make1DRtv(texture, 0, static_cast<int>(texture->ArraySize()), level);

    case Texture::TT_2D:
        return this->Make2DRtv(texture, 0, static_cast<int>(texture->ArraySize()), level);

    case Texture::TT_3D:
        return this->Make3DRtv(texture, 0, 0, texture->Depth(0), level);

    case Texture::TT_Cube:
        return this->MakeCubeRtv(texture, 0, level);

    default:
        ZENGINE_UNREACHABLE("Invalid texture type");
    }
}

RenderTargetViewPtr RenderFactory::MakeBufferRtv(const GraphicsBufferPtr& gbuffer, ElementFormat pf)
{
    return this->MakeBufferRtv(gbuffer, pf, 0, gbuffer->Size() / NumFormatBytes(pf));
}

DepthStencilViewPtr RenderFactory::Make1DDsv(const TexturePtr& texture, int first_array_index, int array_size, int level)
{
    return this->Make1DDsv(texture, texture->Format(), first_array_index, array_size, level);
}

DepthStencilViewPtr RenderFactory::Make2DDsv(const TexturePtr& texture, int first_array_index, int array_size, int level)
{
	return this->Make2DDsv(texture, texture->Format(), first_array_index, array_size, level);
}

DepthStencilViewPtr RenderFactory::Make2DDsv(const TexturePtr& texture, int array_index, uint32_t slice, int level)
{
    return this->Make2DDsv(texture, texture->Format(), array_index, slice, level);
}

DepthStencilViewPtr RenderFactory::Make3DDsv(const TexturePtr& texture, int array_index, uint32_t first_slice, uint32_t num_slices, int level)
{
    return this->Make3DDsv(texture, texture->Format(), array_index, first_slice, num_slices, level);
}

DepthStencilViewPtr RenderFactory::MakeCubeDsv(const TexturePtr& texture, int array_index, int level)
{
    return this->MakeCubeDsv(texture, texture->Format(), array_index, level);
}

DepthStencilViewPtr RenderFactory::MakeTextureDsv(const TexturePtr& texture, uint32_t level)
{
    switch (texture->Type())
    {
    case Texture::TT_1D:
        return this->Make1DDsv(texture, 0, static_cast<int>(texture->ArraySize()), level);

    case Texture::TT_2D:
        return this->Make2DDsv(texture, 0, static_cast<int>(texture->ArraySize()), level);

    case Texture::TT_3D:
        return this->Make3DDsv(texture, 0, 0, texture->Depth(0), level);

    case Texture::TT_Cube:
        return this->MakeCubeDsv(texture, 0, level);

    default:
        ZENGINE_UNREACHABLE("Invalid texture type");
    }
}

	UnorderedAccessViewPtr RenderFactory::Make1DUav(TexturePtr const & texture, int first_array_index, int array_size, int level)
	{
		return this->Make1DUav(texture, texture->Format(), first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr RenderFactory::Make2DUav(TexturePtr const & texture, int first_array_index, int array_size, int level)
	{
		return this->Make2DUav(texture, texture->Format(), first_array_index, array_size, level);
	}

	UnorderedAccessViewPtr RenderFactory::Make2DUav(TexturePtr const & texture, int array_index, Texture::CubeFaces face, int level)
	{
		return this->Make2DUav(texture, texture->Format(), array_index, face, level);
	}

	UnorderedAccessViewPtr RenderFactory::Make2DUav(TexturePtr const & texture, int array_index, uint32_t slice, int level)
	{
		return this->Make2DUav(texture, texture->Format(), array_index, slice, level);
	}

	UnorderedAccessViewPtr RenderFactory::Make3DUav(TexturePtr const & texture, int array_index, uint32_t first_slice, uint32_t num_slices,
		int level)
	{
		return this->Make3DUav(texture, texture->Format(), array_index, first_slice, num_slices, level);
	}

	UnorderedAccessViewPtr RenderFactory::MakeCubeUav(TexturePtr const & texture, int array_index, int level)
	{
		return this->MakeCubeUav(texture, texture->Format(), array_index, level);
	}

	UnorderedAccessViewPtr RenderFactory::MakeTextureUav(TexturePtr const & texture, uint32_t level)
	{
		switch (texture->Type())
		{
		case Texture::TT_1D:
			return this->Make1DUav(texture, 0, texture->ArraySize(), level);

		case Texture::TT_2D:
			return this->Make2DUav(texture, 0, texture->ArraySize(), level);

		case Texture::TT_3D:
			return this->Make3DUav(texture, 0, 0, texture->Depth(0), level);

		case Texture::TT_Cube:
			return this->MakeCubeUav(texture, 0, level);

		default:
			ZENGINE_UNREACHABLE("Invalid texture type");
		}
	}

	UnorderedAccessViewPtr RenderFactory::MakeBufferUav(GraphicsBufferPtr const & gbuffer, ElementFormat pf)
	{
		return this->MakeBufferUav(gbuffer, pf, 0, gbuffer->Size() / NumFormatBytes(pf));
	}

}