#pragma once
#include <base/ZEngine.h>
#include <render/Texture.h>
#include <render/RenderDeviceCaps.h>


#if ZENGINE_IS_DEV_PLATFORM
namespace RenderWorker
{
class ZENGINE_CORE_API DevHelper
{
    ZENGINE_NONCOPYABLE(DevHelper);
public:

    DevHelper() noexcept;
    virtual ~DevHelper() noexcept;

    virtual void GetImageInfo(std::string_view input_name, std::string_view metadata_name, RenderDeviceCaps const * caps,
        Texture::TextureType& type,
        uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
        ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch) = 0;
};
}
#endif// ZENGINE_IS_DEV_PLATFORM