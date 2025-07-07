#include <Render/RenderFactory.h>


namespace RenderWorker
{

RenderFactory::RenderFactory() = default;

RenderFactory::~RenderFactory() noexcept
{
}

RenderEngine& RenderFactory::RenderEngineInstance()
{
    if (!re_)
    {
        re_ = this->DoMakeRenderEngine();
    }

    return *re_;
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
}