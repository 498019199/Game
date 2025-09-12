#include <dev_helps/TexConverter.h>
#include <render/TexCompression.h>
#include <render/Texture.h>

#include <FreeImage.h>
#include <filesystem>
#include "ImagePlane.h"

namespace
{
using namespace RenderWorker;

class TexLoader
{
public:
    TexturePtr Load(TexMetadata const& metadata);

    static void GetImageInfo(TexMetadata const& metadata, Texture::TextureType& type, uint32_t& width, uint32_t& height,
        uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ElementFormat& format, uint32_t& row_pitch,
        uint32_t& slice_pitch);

    static bool IsSupported(std::string_view input_name);

private:
    bool Load();
    TexturePtr StoreToTexture();

private:
    TexMetadata metadata_;

    std::vector<std::vector<std::shared_ptr<ImagePlane>>> planes_;
    uint32_t width_;
    uint32_t height_;
    uint32_t array_size_;
    uint32_t num_mipmaps_;
    ElementFormat format_;
};

TexturePtr TexLoader::Load(TexMetadata const& metadata)
{
    TexturePtr ret;
    metadata_ = metadata;

    auto& res_loader = Context::Instance().ResLoaderInstance();
    auto in_folder = std::filesystem::path(res_loader.Locate(metadata_.PlaneFileName(0, 0))).parent_path().string();
    bool const in_path = res_loader.IsInPath(in_folder);
    if (!in_path)
    {
        res_loader.AddPath(in_folder);
    }

    if (this->Load())
    {
        ret = this->StoreToTexture();
    }

    if (!in_path)
    {
        res_loader.DelPath(in_folder);
    }

    return ret;
}

void TexLoader::GetImageInfo(TexMetadata const& metadata, Texture::TextureType& type, uint32_t& width, uint32_t& height,
        uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ElementFormat& format, uint32_t& row_pitch,
        uint32_t& slice_pitch)
{
    ImagePlane image;
    image.Load(metadata.PlaneFileName(0, 0), metadata);

    type = Texture::TT_2D;
    width = image.Width();
    height = image.Height();
    depth = 1;

    if (metadata.MipmapEnabled())
    {
        if (metadata.NumMipmaps() == 0)
        {
            num_mipmaps = 1;
            uint32_t w = width;
            uint32_t h = height;
            while ((w != 1) || (h != 1))
            {
                ++ num_mipmaps;

                w = std::max(1U, w / 2);
                h = std::max(1U, h / 2);
            }
        }
        else
        {
            num_mipmaps = metadata.NumMipmaps();
        }
    }
    else
    {
        num_mipmaps = 1;
    }

    array_size = metadata.ArraySize();

    if (image.CompressedTex())
    {
        format = image.CompressedTex()->Format();
    }
    else
    {
        format = image.UncompressedTex()->Format();
    }
    COMMON_ASSERT(format != EF_Unknown);
    ElementFormat metadata_format = metadata.PreferedFormat();
    if (metadata_format == EF_Unknown)
    {
        metadata_format = format;
    }

    if ((num_mipmaps > 1) && metadata.AutoGenMipmap())
    {
        format = image.UncompressedTex()->Format();
    }

    if (format != metadata_format)
    {
        format = metadata_format;

        uint32_t const block_width = BlockWidth(format);
        uint32_t const block_height = BlockHeight(format);
        uint32_t const block_bytes = BlockBytes(format);

        row_pitch = (width + block_width - 1) / block_width * block_bytes;
        slice_pitch = (height + block_height - 1) / block_height * row_pitch;
    }
    else if (IsCompressedFormat(format))
    {
        Texture::Mapper mapper(*image.CompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
            image.CompressedTex()->Width(0), image.CompressedTex()->Height(0));
        row_pitch = mapper.RowPitch();
        slice_pitch = mapper.SlicePitch();
    }
    else
    {
        Texture::Mapper mapper(*image.UncompressedTex(), 0, 0, TMA_Read_Only, 0, 0,
            image.UncompressedTex()->Width(0), image.UncompressedTex()->Height(0));
        row_pitch = mapper.RowPitch();
        slice_pitch = mapper.SlicePitch();
    }
}

bool TexLoader::IsSupported(std::string_view input_name)
{
    const std::string input_name_str = Context::Instance().ResLoaderInstance().Locate(input_name);
    if (input_name_str.empty())
    {
        //LogError() << "Could NOT find " << input_name << '.' << std::endl;
        return false;
    }

    const std::string ext_name = std::filesystem::path(input_name_str).extension().string();
    if (ext_name == ".dds")
    {
        return true;
    }
    else
    {
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(input_name_str.c_str(), 0);
        if (fif == FIF_UNKNOWN)
        {
            fif = FreeImage_GetFIFFromFilename(input_name_str.c_str());
        }
        return fif != FIF_UNKNOWN;
    }
}
}

namespace RenderWorker
{
TexturePtr TexConverter::Load(TexMetadata const& metadata)
{
    TexLoader tl;
    return tl.Load(metadata);
}

void TexConverter::GetImageInfo(TexMetadata const& metadata, Texture::TextureType& type, uint32_t& width, uint32_t& height,
    uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size, ElementFormat& format, uint32_t& row_pitch,
    uint32_t& slice_pitch)
{
    return TexLoader::GetImageInfo(metadata, type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
}

bool TexConverter::IsSupported(std::string_view input_name)
{
    return TexLoader::IsSupported(input_name);
}

}