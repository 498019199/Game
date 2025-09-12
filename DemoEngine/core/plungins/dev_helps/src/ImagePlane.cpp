#include "ImagePlane.h"
#include <dev_helps/TexMetadata.h>

#include <base/ZEngine.h>
#include <math/math.h>
#include <render/RenderEngine.h>
#include <render/TexCompression.h>
#include <render/Texture.h>
#include <FreeImage.h>
#include <filesystem>

namespace RenderWorker
{
using namespace CommonWorker;
using namespace MathWorker;

bool ImagePlane::Load(std::string_view name, TexMetadata const & metadata)
{
    const std::string name_str = Context::Instance().ResLoaderInstance().Locate(name);
    if (name_str.empty())
    {
        return false;
    }

    const std::string ext_name = std::filesystem::path(name).extension().string();

    if (ext_name == ".dds")
    {
        TexturePtr in_tex = LoadVirtualTexture(name_str);
        auto const type = in_tex->Type();
        auto const depth = in_tex->Depth(0);
        auto const num_mipmaps = in_tex->MipMapsNum();
        auto const array_size = in_tex->ArraySize();
        auto const format = in_tex->Format();

        if (type != Texture::TT_2D)
        {
            //LogWarn() << "Only 2D texture are supported." << std::endl;
        }
        if (depth != 1)
        {
            //LogWarn() << "Only first slice in the 3D texture is used." << std::endl;
        }
        if (num_mipmaps != 1)
        {
            //LogWarn() << "Only first mip level in the texture is used." << std::endl;
        }
        if (array_size != 1)
        {
            //LogWarn() << "Only first slice in the texture array is used." << std::endl;
        }

        if (IsCompressedFormat(format))
        {
            ElementFormat uncompressed_format;
            if ((format == EF_BC6) || (format == EF_SIGNED_BC6))
            {
                uncompressed_format = EF_ABGR16F;
            }
            else if (IsSigned(format))
            {
                uncompressed_format = EF_SIGNED_ABGR8;
            }
            else if (IsSRGB(format))
            {
                uncompressed_format = EF_ARGB8_SRGB;
            }
            else
            {
                uncompressed_format = EF_ARGB8;
            }
        }
        else
        {
            uncompressed_tex_ = in_tex;
            compressed_tex_.reset();
        }
    }
    else
    {
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(name_str.c_str(), 0);
        if (fif == FIF_UNKNOWN)
        {
            fif = FreeImage_GetFIFFromFilename(name_str.c_str());
        }
        if (fif == FIF_UNKNOWN)
        {
            return false;
        }

        auto fi_bitmap_deleter = [](FIBITMAP* dib)
        {
            FreeImage_Unload(dib);
        };

        std::unique_ptr<FIBITMAP, decltype(fi_bitmap_deleter)> dib(nullptr, fi_bitmap_deleter);
        if (FreeImage_FIFSupportsReading(fif))
        {
            int flag = 0;
            if (fif == FIF_JPEG)
            {
                flag = JPEG_ACCURATE;
            }

            dib.reset(FreeImage_Load(fif, name_str.c_str(), flag));
        }
        if (!dib)
        {
            return false;
        }

        uint32_t const width = FreeImage_GetWidth(dib.get());
        uint32_t const height = FreeImage_GetHeight(dib.get());
        if ((width == 0) || (height == 0))
        {
            return false;
        }

        FreeImage_FlipVertical(dib.get());

        ElementFormat uncompressed_format = EF_ABGR8;
        FREE_IMAGE_TYPE const image_type = FreeImage_GetImageType(dib.get());
        switch (image_type)
        {
        case FIT_BITMAP:
            {
                uint32_t const bpp = FreeImage_GetBPP(dib.get());
                uint32_t const r_mask = FreeImage_GetRedMask(dib.get());
                uint32_t const g_mask = FreeImage_GetGreenMask(dib.get());
                uint32_t const b_mask = FreeImage_GetBlueMask(dib.get());
                switch (bpp)
                {
                case 1:
                case 4:
                case 8:
                case 24:
                    if (bpp == 24)
                    {
                        if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
                        {
                            uncompressed_format = EF_ARGB8;
                        }
                        else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
                        {
                            uncompressed_format = EF_ABGR8;
                        }
                    }
                    else
                    {
                        uncompressed_format = EF_ARGB8;
                    }
                    dib.reset(FreeImage_ConvertTo32Bits(dib.get()));
                    break;

                case 16:
                    if ((r_mask == (0x1F << 10)) && (g_mask == (0x1F << 5)) && (b_mask == 0x1F))
                    {
                        uncompressed_format = EF_A1RGB5;
                    }
                    else if ((r_mask == (0x1F << 11)) && (g_mask == (0x3F << 5)) && (b_mask == 0x1F))
                    {
                        uncompressed_format = EF_R5G6B5;
                    }
                    break;

                case 32:
                    if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
                    {
                        uncompressed_format = EF_ARGB8;
                    }
                    else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
                    {
                        uncompressed_format = EF_ABGR8;
                    }
                    break;

                default:
                    ZENGINE_UNREACHABLE("Unsupported bpp.");
                }
            }
            break;

        case FIT_UINT16:
            uncompressed_format = EF_R16UI;
            break;

        case FIT_INT16:
            uncompressed_format = EF_R16I;
            break;

        case FIT_UINT32:
            uncompressed_format = EF_R32UI;
            break;

        case FIT_INT32:
            uncompressed_format = EF_R32I;
            break;

        case FIT_FLOAT:
            uncompressed_format = EF_R32F;
            break;

        case FIT_COMPLEX:
            uncompressed_format = EF_GR32F;
            break;

        case FIT_RGB16:
            uncompressed_format = EF_ABGR16;
            dib.reset(FreeImage_ConvertToRGBA16(dib.get()));
            break;

        case FIT_RGBA16:
            uncompressed_format = EF_ABGR16;
            break;

        case FIT_RGBF:
            uncompressed_format = EF_ABGR32F;
            dib.reset(FreeImage_ConvertToRGBAF(dib.get()));
            break;

        case FIT_RGBAF:
            uncompressed_format = EF_ABGR32F;
            break;

        default:
            ZENGINE_UNREACHABLE("Unsupported image type.");
        }

        uint8_t const * src = FreeImage_GetBits(dib.get());
        if (src == nullptr)
        {
            return false;
        }

        ElementInitData uncompressed_init_data;
        uncompressed_init_data.data = src;
        uncompressed_init_data.row_pitch = FreeImage_GetPitch(dib.get());
        uncompressed_init_data.slice_pitch = uncompressed_init_data.row_pitch * height;

        uncompressed_tex_ = MakeSharedPtr<VirtualTexture>(Texture::TT_2D, width, height,
            1, 1, 1, uncompressed_format, false);
        uncompressed_tex_->CreateHWResource(MakeSpan<1>(uncompressed_init_data), nullptr);
    }

    if (metadata.ForceSRGB())
    {
        if (!IsSRGB(uncompressed_tex_->Format()))
        {
            uint32_t const width = uncompressed_tex_->Width(0);
            uint32_t const height = uncompressed_tex_->Height(0);

            TexturePtr srgb_uncompressed_tex = MakeSharedPtr<VirtualTexture>(Texture::TT_2D,
                width, height, 1, 1, 1, MakeSRGB(uncompressed_tex_->Format()), false);
            {
                Texture::Mapper ori_mapper(*uncompressed_tex_, 0, 0, TMA_Read_Only, 0, 0, width, height);

                ElementInitData init_data;
                init_data.data = ori_mapper.Pointer<void>();
                init_data.row_pitch = ori_mapper.RowPitch();
                init_data.slice_pitch = ori_mapper.SlicePitch();

                srgb_uncompressed_tex->CreateHWResource(MakeSpan<1>(init_data), nullptr);
            }

            uncompressed_tex_ = srgb_uncompressed_tex;
        }

        if (compressed_tex_ && !IsSRGB(compressed_tex_->Format()))
        {
            uint32_t const width = compressed_tex_->Width(0);
            uint32_t const height = compressed_tex_->Height(0);

            TexturePtr srgb_compressed_tex = MakeSharedPtr<VirtualTexture>(Texture::TT_2D,
                width, height, 1, 1, 1, MakeSRGB(compressed_tex_->Format()), false);
            {
                Texture::Mapper ori_mapper(*compressed_tex_, 0, 0, TMA_Read_Only, 0, 0, width, height);

                ElementInitData init_data;
                init_data.data = ori_mapper.Pointer<void>();
                init_data.row_pitch = ori_mapper.RowPitch();
                init_data.slice_pitch = ori_mapper.SlicePitch();

                srgb_compressed_tex->CreateHWResource(MakeSpan<1>(init_data), nullptr);
            }

            compressed_tex_ = srgb_compressed_tex;
        }
    }

    uint32_t const num_channels = NumComponents(uncompressed_tex_->Format());
    int32_t channel_mapping[4];
    for (uint32_t ch = 0; ch < num_channels; ++ ch)
    {
        channel_mapping[ch] = metadata.ChannelMapping(ch);
    }

    bool need_swizzle = false;
    for (uint32_t ch = 0; ch < num_channels; ++ ch)
    {
        if (channel_mapping[ch] != static_cast<int32_t>(ch))
        {
            need_swizzle = true;
            break;
        }
    }
    if (need_swizzle)
    {
        compressed_tex_.reset();

        uint32_t const width = uncompressed_tex_->Width(0);
        uint32_t const height = uncompressed_tex_->Height(0);
        ElementFormat const format = uncompressed_tex_->Format();

        Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Write, 0, 0,
            uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
        uint8_t* ptr = mapper.Pointer<uint8_t>();
        std::vector<Color> line_32f(width);
        for (uint32_t y = 0; y < height; ++ y)
        {
            ConvertToABGR32F(format, ptr, width, line_32f.data());

            Color original_clr;
            Color swizzled_clr(0, 0, 0, 0);
            for (uint32_t x = 0; x < width; ++ x)
            {
                original_clr = line_32f[x];
                for (uint32_t ch = 0; ch < num_channels; ++ ch)
                {
                    if (channel_mapping[ch] >= 0)
                    {
                        swizzled_clr[ch] = original_clr[channel_mapping[ch]];
                    }
                    else
                    {
                        swizzled_clr[ch] = 0;
                    }
                }
                line_32f[x] = swizzled_clr;
            }

            ConvertFromABGR32F(format, line_32f.data(), width, ptr);

            ptr += mapper.RowPitch();
        }
    }

    auto const preferred_fmt = metadata.PreferedFormat();
    if ((preferred_fmt != EF_Unknown) && IsCompressedFormat(preferred_fmt))
    {
        uint32_t const block_width = BlockWidth(preferred_fmt);
        uint32_t const block_height = BlockHeight(preferred_fmt);

        uint32_t const width = uncompressed_tex_->Width(0);
        uint32_t const height = uncompressed_tex_->Height(0);

        uint32_t const aligned_width = (width + block_width - 1) & ~(block_width - 1);
        uint32_t const aligned_height = (height + block_height - 1) & ~(block_height - 1);

        if ((width != aligned_width) || (height != aligned_height))
        {
            *this = this->ResizeTo(aligned_width, aligned_height, true);
        }
    }
    return false;
}

void ImagePlane::RgbToLum()
{
    
}

void ImagePlane::AlphaToLum()
{
    
}

void ImagePlane::BumpToNormal(float scale, float amplitude)
{
    
}

void ImagePlane::NormalToHeight(float min_z)
{
    
}

void ImagePlane::PrepareNormalCompression(ElementFormat normal_compression_format)
{
    
}

void ImagePlane::FormatConversion(ElementFormat format)
{
    
}

ImagePlane ImagePlane::ResizeTo(uint32_t width, uint32_t height, bool linear)
{
    COMMON_ASSERT(uncompressed_tex_);

    compressed_tex_.reset();

    auto const format = uncompressed_tex_->Format();

    ImagePlane target;
    target.uncompressed_tex_ = MakeSharedPtr<VirtualTexture>(Texture::TT_2D, width, height, 1, 1, 1,
        format, false);

    ElementInitData target_init_data;
    target_init_data.row_pitch = width * NumFormatBytes(uncompressed_tex_->Format());
    target_init_data.slice_pitch = target_init_data.row_pitch * height;
    std::vector<uint8_t> target_data(target_init_data.slice_pitch);
    target_init_data.data = target_data.data();

    {
        Texture::Mapper mapper(*uncompressed_tex_, 0, 0, TMA_Read_Only, 0, 0,
            uncompressed_tex_->Width(0), uncompressed_tex_->Height(0));
        ResizeTexture(target_data.data(), target_init_data.row_pitch, target_init_data.slice_pitch,
            format, width, height, 1,
            mapper.Pointer<void>(), mapper.RowPitch(), mapper.SlicePitch(), format,
            uncompressed_tex_->Width(0), uncompressed_tex_->Height(0), 1,
            linear ? TextureFilter::Linear : TextureFilter::Point);
    }

    target.uncompressed_tex_->CreateHWResource(MakeSpan<1>(target_init_data), nullptr);

    return target;
}

}