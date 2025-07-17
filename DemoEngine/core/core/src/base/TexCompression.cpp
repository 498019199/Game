#include <render/TexCompression.h>

namespace RenderWorker
{

uint32_t BlockWidth(ElementFormat format)
{
    switch (format)
    {
    case EF_BC1:
    case EF_SIGNED_BC1:
    case EF_BC1_SRGB:
    case EF_BC2:
    case EF_SIGNED_BC2:
    case EF_BC2_SRGB:
    case EF_BC3:
    case EF_SIGNED_BC3:
    case EF_BC3_SRGB:
    case EF_BC4:
    case EF_SIGNED_BC4:
    case EF_BC4_SRGB:
    case EF_BC5:
    case EF_SIGNED_BC5:
    case EF_BC5_SRGB:
    case EF_BC6:
    case EF_SIGNED_BC6:
    case EF_BC7:
    case EF_BC7_SRGB:
    case EF_ETC1:
    case EF_ETC2_BGR8:
    case EF_ETC2_BGR8_SRGB:
    case EF_ETC2_A1BGR8:
    case EF_ETC2_A1BGR8_SRGB:
    case EF_ETC2_ABGR8:
    case EF_ETC2_ABGR8_SRGB:
    case EF_ETC2_R11:
    case EF_SIGNED_ETC2_R11:
    case EF_ETC2_GR11:
    case EF_SIGNED_ETC2_GR11:
        return 4;

    default:
        COMMON_ASSERT(!IsCompressedFormat(format));
        return 1;
    }
}

uint32_t BlockHeight(ElementFormat format)
{
    switch (format)
    {
    case EF_BC1:
    case EF_SIGNED_BC1:
    case EF_BC1_SRGB:
    case EF_BC2:
    case EF_SIGNED_BC2:
    case EF_BC2_SRGB:
    case EF_BC3:
    case EF_SIGNED_BC3:
    case EF_BC3_SRGB:
    case EF_BC4:
    case EF_SIGNED_BC4:
    case EF_BC4_SRGB:
    case EF_BC5:
    case EF_SIGNED_BC5:
    case EF_BC5_SRGB:
    case EF_BC6:
    case EF_SIGNED_BC6:
    case EF_BC7:
    case EF_BC7_SRGB:
    case EF_ETC1:
    case EF_ETC2_BGR8:
    case EF_ETC2_BGR8_SRGB:
    case EF_ETC2_A1BGR8:
    case EF_ETC2_A1BGR8_SRGB:
    case EF_ETC2_ABGR8:
    case EF_ETC2_ABGR8_SRGB:
    case EF_ETC2_R11:
    case EF_SIGNED_ETC2_R11:
    case EF_ETC2_GR11:
    case EF_SIGNED_ETC2_GR11:
        return 4;

    default:
        COMMON_ASSERT(!IsCompressedFormat(format));
        return 1;
    }
}

uint32_t BlockDepth(ElementFormat format)
{
    switch (format)
    {
    case EF_BC1:
    case EF_SIGNED_BC1:
    case EF_BC1_SRGB:
    case EF_BC2:
    case EF_SIGNED_BC2:
    case EF_BC2_SRGB:
    case EF_BC3:
    case EF_SIGNED_BC3:
    case EF_BC3_SRGB:
    case EF_BC4:
    case EF_SIGNED_BC4:
    case EF_BC4_SRGB:
    case EF_BC5:
    case EF_SIGNED_BC5:
    case EF_BC5_SRGB:
    case EF_BC6:
    case EF_SIGNED_BC6:
    case EF_BC7:
    case EF_BC7_SRGB:
    case EF_ETC1:
    case EF_ETC2_BGR8:
    case EF_ETC2_BGR8_SRGB:
    case EF_ETC2_A1BGR8:
    case EF_ETC2_A1BGR8_SRGB:
    case EF_ETC2_ABGR8:
    case EF_ETC2_ABGR8_SRGB:
    case EF_ETC2_R11:
    case EF_SIGNED_ETC2_R11:
    case EF_ETC2_GR11:
    case EF_SIGNED_ETC2_GR11:
        return 1;

    default:
        COMMON_ASSERT(!IsCompressedFormat(format));
        return 1;
    }
}

uint32_t BlockBytes(ElementFormat format)
{
    switch (format)
    {
    case EF_BC1:
    case EF_SIGNED_BC1:
    case EF_BC1_SRGB:
    case EF_BC2:
    case EF_SIGNED_BC2:
    case EF_BC2_SRGB:
    case EF_BC3:
    case EF_SIGNED_BC3:
    case EF_BC3_SRGB:
    case EF_BC4:
    case EF_SIGNED_BC4:
    case EF_BC4_SRGB:
    case EF_BC5:
    case EF_SIGNED_BC5:
    case EF_BC5_SRGB:
    case EF_BC6:
    case EF_SIGNED_BC6:
    case EF_BC7:
    case EF_BC7_SRGB:
    case EF_ETC1:
    case EF_ETC2_BGR8:
    case EF_ETC2_BGR8_SRGB:
    case EF_ETC2_A1BGR8:
    case EF_ETC2_A1BGR8_SRGB:
    case EF_ETC2_ABGR8:
    case EF_ETC2_ABGR8_SRGB:
    case EF_ETC2_R11:
    case EF_SIGNED_ETC2_R11:
    case EF_ETC2_GR11:
    case EF_SIGNED_ETC2_GR11:
        return NumFormatBytes(format) * 4;

    default:
        COMMON_ASSERT(!IsCompressedFormat(format));
        return NumFormatBytes(format);
    }
}

ElementFormat DecodedFormat(ElementFormat format)
{
	switch (format)
    {
    case EF_BC1:
    case EF_SIGNED_BC1:
    case EF_BC1_SRGB:
        return EF_ARGB8;

    case EF_BC2:
    case EF_SIGNED_BC2:
    case EF_BC2_SRGB:
        return EF_ARGB8;

    case EF_BC3:
    case EF_SIGNED_BC3:
    case EF_BC3_SRGB:
        return EF_ARGB8;

    case EF_BC4:
    case EF_SIGNED_BC4:
    case EF_BC4_SRGB:
        return EF_R8;

    case EF_BC5:
    case EF_SIGNED_BC5:
    case EF_BC5_SRGB:
        return EF_GR8;

    case EF_BC6:
    case EF_SIGNED_BC6:
        return EF_ABGR16F;

    case EF_BC7:
    case EF_BC7_SRGB:
        return EF_ARGB8;

    case EF_ETC1:
        return EF_ARGB8;

    case EF_ETC2_BGR8:
    case EF_ETC2_BGR8_SRGB:
        return EF_ARGB8;

    case EF_ETC2_A1BGR8:
    case EF_ETC2_A1BGR8_SRGB:
        return EF_ARGB8;

    case EF_ETC2_ABGR8:
    case EF_ETC2_ABGR8_SRGB:
        return EF_ARGB8;

    case EF_ETC2_R11:
    case EF_SIGNED_ETC2_R11:
        return EF_R8;

    case EF_ETC2_GR11:
    case EF_SIGNED_ETC2_GR11:
        return EF_GR8;

    default:
        COMMON_ASSERT(!IsCompressedFormat(format));
        return format;
    }
}
}