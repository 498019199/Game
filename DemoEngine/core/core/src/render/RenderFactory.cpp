#include <Render/RenderFactory.h>


namespace RenderWorker
{

RenderFactory::RenderFactory() = default;

RenderFactory::~RenderFactory() noexcept
{
}

ShaderResourceViewPtr RenderFactory::MakeTextureSrv(const TexturePtr& texture)
{
    return MakeTextureSrv(texture, texture->Format(), 0, texture->ArraySize(), 0, texture->MipMapsNum());
}

ShaderResourceViewPtr RenderFactory::MakeTextureSrv(const TexturePtr& texture, ElementFormat pf)
{
    return MakeTextureSrv(texture, pf, 0, texture->ArraySize(), 0, texture->MipMapsNum());
}

ShaderResourceViewPtr RenderFactory::MakeTextureSrv(const TexturePtr& texture, uint32_t first_array_index, uint32_t array_size,
        uint32_t first_level, uint32_t num_levels)
{
    return MakeTextureSrv(texture, texture->Format(), first_array_index, array_size, first_level, num_levels);
}
}