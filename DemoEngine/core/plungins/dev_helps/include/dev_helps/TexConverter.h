
#include <dev_helps/TexMetadata.h>
#include <string_view>

namespace RenderWorker
{
class TexConverter final
{
public:
    TexturePtr Load(TexMetadata const& metadata);

    static void GetImageInfo(TexMetadata const& metadata, Texture::TextureType& type, uint32_t& width, uint32_t& height,
        uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ElementFormat& format, uint32_t& row_pitch,
        uint32_t& slice_pitch);

    static bool IsSupported(std::string_view input_name);
};

}