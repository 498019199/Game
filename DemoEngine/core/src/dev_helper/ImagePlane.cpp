#include <dev_helper/ImagePlane.h>
#include <render/RenderEngine.h>

//#include <FreeImage.h>

#include <filesystem>
namespace RenderWorker
{

bool ImagePlane::Load(std::string_view name)
{
    // const std::string name_str = name;
    // std::string const ext_name = std::filesystem::path(name).extension().string();

    // if (ext_name == ".dds")
    // {
    //     auto const format = ;
    //     if (IsCompressedFormat(format))
    //     {
    //         ElementFormat uncompressed_format;
    //         if ((format == EF_BC6) || (format == EF_SIGNED_BC6))
    //         {
    //             uncompressed_format = EF_ABGR16F;
    //         }
    //         else if (IsSigned(format))
    //         {
    //             uncompressed_format = EF_SIGNED_ABGR8;
    //         }
    //         else if (IsSRGB(format))
    //         {
    //             uncompressed_format = EF_ARGB8_SRGB;
    //         }
    //         else
    //         {
    //             uncompressed_format = EF_ARGB8;
    //         }
    //     }
    // }
    // else
    // {
    //     FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(name_str.c_str(), 0);
    //     if (fif == FIF_UNKNOWN)
    //     {
    //         fif = FreeImage_GetFIFFromFilename(name_str.c_str());
    //     }
    //     if (fif == FIF_UNKNOWN)
    //     {
    //         return false;
    //     }

    //     auto fi_bitmap_deleter = [](FIBITMAP* dib)
    //     {
    //         FreeImage_Unload(dib);
    //     };

    //     std::unique_ptr<FIBITMAP, decltype(fi_bitmap_deleter)> dib(nullptr, fi_bitmap_deleter);
    //     if (FreeImage_FIFSupportsReading(fif))
    //     {
    //         int flag = 0;
    //         if (fif == FIF_JPEG)
    //         {
    //             flag = JPEG_ACCURATE;
    //         }

    //         dib.reset(FreeImage_Load(fif, name_str.c_str(), flag));
    //     }
    //     if (!dib)
    //     {
    //         return false;
    //     }

    //     uint32_t const width = FreeImage_GetWidth(dib.get());
    //     uint32_t const height = FreeImage_GetHeight(dib.get());
    //     if ((width == 0) || (height == 0))
    //     {
    //         return false;
    //     }

    //     FreeImage_FlipVertical(dib.get());

    //     ElementFormat uncompressed_format = EF_ABGR8;
    //     FREE_IMAGE_TYPE const image_type = FreeImage_GetImageType(dib.get());
    //     switch (image_type)
    //     {
    //     case FIT_BITMAP:
    //         {
    //             uint32_t const bpp = FreeImage_GetBPP(dib.get());
    //             uint32_t const r_mask = FreeImage_GetRedMask(dib.get());
    //             uint32_t const g_mask = FreeImage_GetGreenMask(dib.get());
    //             uint32_t const b_mask = FreeImage_GetBlueMask(dib.get());
    //             switch (bpp)
    //             {
    //             case 1:
    //             case 4:
    //             case 8:
    //             case 24:
    //                 if (bpp == 24)
    //                 {
    //                     if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
    //                     {
    //                         uncompressed_format = EF_ARGB8;
    //                     }
    //                     else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
    //                     {
    //                         uncompressed_format = EF_ABGR8;
    //                     }
    //                 }
    //                 else
    //                 {
    //                     uncompressed_format = EF_ARGB8;
    //                 }
    //                 dib.reset(FreeImage_ConvertTo32Bits(dib.get()));
    //                 break;

    //             case 16:
    //                 if ((r_mask == (0x1F << 10)) && (g_mask == (0x1F << 5)) && (b_mask == 0x1F))
    //                 {
    //                     uncompressed_format = EF_A1RGB5;
    //                 }
    //                 else if ((r_mask == (0x1F << 11)) && (g_mask == (0x3F << 5)) && (b_mask == 0x1F))
    //                 {
    //                     uncompressed_format = EF_R5G6B5;
    //                 }
    //                 break;

    //             case 32:
    //                 if ((r_mask == 0xFF0000) && (g_mask == 0xFF00) && (b_mask == 0xFF))
    //                 {
    //                     uncompressed_format = EF_ARGB8;
    //                 }
    //                 else if ((r_mask == 0xFF) && (g_mask == 0xFF00) && (b_mask == 0xFF0000))
    //                 {
    //                     uncompressed_format = EF_ABGR8;
    //                 }
    //                 break;

    //             //default:
    //                 //KFL_UNREACHABLE("Unsupported bpp.");
    //             }
    //         }
    //         break;

    //     case FIT_UINT16:
    //         uncompressed_format = EF_R16UI;
    //         break;

    //     case FIT_INT16:
    //         uncompressed_format = EF_R16I;
    //         break;

    //     case FIT_UINT32:
    //         uncompressed_format = EF_R32UI;
    //         break;

    //     case FIT_INT32:
    //         uncompressed_format = EF_R32I;
    //         break;

    //     case FIT_FLOAT:
    //         uncompressed_format = EF_R32F;
    //         break;

    //     case FIT_COMPLEX:
    //         uncompressed_format = EF_GR32F;
    //         break;

    //     case FIT_RGB16:
    //         uncompressed_format = EF_ABGR16;
    //         dib.reset(FreeImage_ConvertToRGBA16(dib.get()));
    //         break;

    //     case FIT_RGBA16:
    //         uncompressed_format = EF_ABGR16;
    //         break;

    //     case FIT_RGBF:
    //         uncompressed_format = EF_ABGR32F;
    //         dib.reset(FreeImage_ConvertToRGBAF(dib.get()));
    //         break;

    //     case FIT_RGBAF:
    //         uncompressed_format = EF_ABGR32F;
    //         break;

    //     //default:
    //         //KFL_UNREACHABLE("Unsupported image type.");
    //     }

    //     uint8_t const * src = FreeImage_GetBits(dib.get());
    //     if (src == nullptr)
    //     {
    //         return false;
    //     }

    //     ElementInitData uncompressed_init_data;
    //     uncompressed_init_data.data = src;
    //     uncompressed_init_data.row_pitch = FreeImage_GetPitch(dib.get());
    //     uncompressed_init_data.slice_pitch = uncompressed_init_data.row_pitch * height;
    // }
    return false;
}

}