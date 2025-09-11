#include <base/ZEngine.h>
#include <base/ResLoader.h>

#include <render/Texture.h>
#include <render/TexCompression.h>
#include <render/RenderFactory.h>

#include <fstream>
#include <filesystem>
namespace
{
	using namespace RenderWorker;
    enum
	{
		// The surface has alpha channel information in the pixel format.
		DDSPF_ALPHAPIXELS = 0x00000001,

		// The pixel format contains alpha only information
		DDSPF_ALPHA = 0x00000002,

		// The FourCC code is valid.
		DDSPF_FOURCC = 0x00000004,

		// The RGB data in the pixel format structure is valid.
		DDSPF_RGB = 0x00000040,

		// Luminance data in the pixel format is valid.
		// Use this flag for luminance-only or luminance+alpha surfaces,
		// the bit depth is then ddpf.dwLuminanceBitCount.
		DDSPF_LUMINANCE = 0x00020000,

		// Bump map dUdV data in the pixel format is valid.
		DDSPF_BUMPDUDV = 0x00080000
	};

	struct DDSPIXELFORMAT
	{
		uint32_t	size;				// size of structure
		uint32_t	flags;				// pixel format flags
		uint32_t	four_cc;			// (FOURCC code)
		uint32_t	rgb_bit_count;		// how many bits per pixel
		uint32_t	r_bit_mask;			// mask for red bit
		uint32_t	g_bit_mask;			// mask for green bits
		uint32_t	b_bit_mask;			// mask for blue bits
		uint32_t	rgb_alpha_bit_mask;	// mask for alpha channels
	};
	static_assert(sizeof(DDSPIXELFORMAT) == 32);

	enum
	{
		// Indicates a complex surface structure is being described.  A
		// complex surface structure results in the creation of more than
		// one surface.  The additional surfaces are attached to the root
		// surface.  The complex structure can only be destroyed by
		// destroying the root.
		DDSCAPS_COMPLEX		= 0x00000008,

		// Indicates that this surface can be used as a 3D texture.  It does not
		// indicate whether or not the surface is being used for that purpose.
		DDSCAPS_TEXTURE		= 0x00001000,

		// Indicates surface is one level of a mip-map. This surface will
		// be attached to other DDSCAPS_MIPMAP surfaces to form the mip-map.
		// This can be done explicitly, by creating a number of surfaces and
		// attaching them with AddAttachedSurface or by implicitly by CreateSurface.
		// If this bit is set then DDSCAPS_TEXTURE must also be set.
		DDSCAPS_MIPMAP		= 0x00400000,
	};

	enum
	{
		// This flag is used at CreateSurface time to indicate that this set of
		// surfaces is a cubic environment map
		DDSCAPS2_CUBEMAP	= 0x00000200,

		// These flags preform two functions:
		// - At CreateSurface time, they define which of the six cube faces are
		//   required by the application.
		// - After creation, each face in the cubemap will have exactly one of these
		//   bits set.
		DDSCAPS2_CUBEMAP_POSITIVEX	= 0x00000400,
		DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x00000800,
		DDSCAPS2_CUBEMAP_POSITIVEY	= 0x00001000,
		DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x00002000,
		DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x00004000,
		DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x00008000,

		// Indicates that the surface is a volume.
		// Can be combined with DDSCAPS_MIPMAP to indicate a multi-level volume
		DDSCAPS2_VOLUME		= 0x00200000,
	};

	struct DDSCAPS2
	{
		uint32_t	caps1;			// capabilities of surface wanted
		uint32_t	caps2;
		uint32_t	reserved[2];
	};
	static_assert(sizeof(DDSCAPS2) == 16);

	enum
	{
		DDSD_CAPS			= 0x00000001,	// default, dds_caps field is valid.
		DDSD_HEIGHT			= 0x00000002,	// height field is valid.
		DDSD_WIDTH			= 0x00000004,	// width field is valid.
		DDSD_PITCH			= 0x00000008,	// pitch is valid.
		DDSD_PIXELFORMAT	= 0x00001000,	// pixel_format is valid.
		DDSD_MIPMAPCOUNT	= 0x00020000,	// mip_map_count is valid.
		DDSD_LINEARSIZE		= 0x00080000,	// linear_size is valid
		DDSD_DEPTH			= 0x00800000,	// depth is valid
	};

	struct DDSSURFACEDESC2
	{
		uint32_t	size;					// size of the DDSURFACEDESC structure
		uint32_t	flags;					// determines what fields are valid
		uint32_t	height;					// height of surface to be created
		uint32_t	width;					// width of input surface
		union
		{
			int32_t		pitch;				// distance to start of next line (return value only)
			uint32_t	linear_size;		// Formless late-allocated optimized surface size
		};
		uint32_t		depth;				// the depth if this is a volume texture
		uint32_t		mip_map_count;		// number of mip-map levels requestde
		uint32_t		reserved1[11];		// reserved
		DDSPIXELFORMAT	pixel_format;		// pixel format description of the surface
		DDSCAPS2		dds_caps;			// direct draw surface capabilities
		uint32_t		reserved2;
	};
	static_assert(sizeof(DDSSURFACEDESC2) == 124);

	enum D3D_RESOURCE_DIMENSION
	{
		D3D_RESOURCE_DIMENSION_UNKNOWN = 0,
		D3D_RESOURCE_DIMENSION_BUFFER = 1,
		D3D_RESOURCE_DIMENSION_TEXTURE1D = 2,
		D3D_RESOURCE_DIMENSION_TEXTURE2D = 3,
		D3D_RESOURCE_DIMENSION_TEXTURE3D = 4,
	};

	enum D3D_RESOURCE_MISC_FLAG
	{
		D3D_RESOURCE_MISC_GENERATE_MIPS = 0x1L,
		D3D_RESOURCE_MISC_SHARED = 0x2L,
		D3D_RESOURCE_MISC_TEXTURECUBE = 0x4L,
		D3D_RESOURCE_MISC_SHARED_KEYEDMUTEX = 0x10L,
		D3D_RESOURCE_MISC_GDI_COMPATIBLE = 0x20L
	};

	struct DDS_HEADER_DXT10
	{
		uint32_t dxgi_format;
		D3D_RESOURCE_DIMENSION resource_dim;
		uint32_t misc_flag;
		uint32_t array_size;
		uint32_t reserved;
	};
	static_assert(sizeof(DDS_HEADER_DXT10) == 20);

#ifndef DXGI_FORMAT_DEFINED
	enum DXGI_FORMAT : uint32_t
	{
		DXGI_FORMAT_UNKNOWN	                    = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
		DXGI_FORMAT_R32G32B32A32_UINT           = 3,
		DXGI_FORMAT_R32G32B32A32_SINT           = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
		DXGI_FORMAT_R32G32B32_FLOAT             = 6,
		DXGI_FORMAT_R32G32B32_UINT              = 7,
		DXGI_FORMAT_R32G32B32_SINT              = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
		DXGI_FORMAT_R16G16B16A16_UINT           = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
		DXGI_FORMAT_R16G16B16A16_SINT           = 14,
		DXGI_FORMAT_R32G32_TYPELESS             = 15,
		DXGI_FORMAT_R32G32_FLOAT                = 16,
		DXGI_FORMAT_R32G32_UINT                 = 17,
		DXGI_FORMAT_R32G32_SINT                 = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
		DXGI_FORMAT_R10G10B10A2_UINT            = 25,
		DXGI_FORMAT_R11G11B10_FLOAT             = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
		DXGI_FORMAT_R8G8B8A8_UINT               = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
		DXGI_FORMAT_R8G8B8A8_SINT               = 32,
		DXGI_FORMAT_R16G16_TYPELESS             = 33,
		DXGI_FORMAT_R16G16_FLOAT                = 34,
		DXGI_FORMAT_R16G16_UNORM                = 35,
		DXGI_FORMAT_R16G16_UINT                 = 36,
		DXGI_FORMAT_R16G16_SNORM                = 37,
		DXGI_FORMAT_R16G16_SINT                 = 38,
		DXGI_FORMAT_R32_TYPELESS                = 39,
		DXGI_FORMAT_D32_FLOAT                   = 40,
		DXGI_FORMAT_R32_FLOAT                   = 41,
		DXGI_FORMAT_R32_UINT                    = 42,
		DXGI_FORMAT_R32_SINT                    = 43,
		DXGI_FORMAT_R24G8_TYPELESS              = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
		DXGI_FORMAT_R8G8_TYPELESS               = 48,
		DXGI_FORMAT_R8G8_UNORM                  = 49,
		DXGI_FORMAT_R8G8_UINT                   = 50,
		DXGI_FORMAT_R8G8_SNORM                  = 51,
		DXGI_FORMAT_R8G8_SINT                   = 52,
		DXGI_FORMAT_R16_TYPELESS                = 53,
		DXGI_FORMAT_R16_FLOAT                   = 54,
		DXGI_FORMAT_D16_UNORM                   = 55,
		DXGI_FORMAT_R16_UNORM                   = 56,
		DXGI_FORMAT_R16_UINT                    = 57,
		DXGI_FORMAT_R16_SNORM                   = 58,
		DXGI_FORMAT_R16_SINT                    = 59,
		DXGI_FORMAT_R8_TYPELESS                 = 60,
		DXGI_FORMAT_R8_UNORM                    = 61,
		DXGI_FORMAT_R8_UINT                     = 62,
		DXGI_FORMAT_R8_SNORM                    = 63,
		DXGI_FORMAT_R8_SINT                     = 64,
		DXGI_FORMAT_A8_UNORM                    = 65,
		DXGI_FORMAT_R1_UNORM                    = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
		DXGI_FORMAT_BC1_TYPELESS                = 70,
		DXGI_FORMAT_BC1_UNORM                   = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
		DXGI_FORMAT_BC2_TYPELESS                = 73,
		DXGI_FORMAT_BC2_UNORM                   = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
		DXGI_FORMAT_BC3_TYPELESS                = 76,
		DXGI_FORMAT_BC3_UNORM                   = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
		DXGI_FORMAT_BC4_TYPELESS                = 79,
		DXGI_FORMAT_BC4_UNORM                   = 80,
		DXGI_FORMAT_BC4_SNORM                   = 81,
		DXGI_FORMAT_BC5_TYPELESS                = 82,
		DXGI_FORMAT_BC5_UNORM                   = 83,
		DXGI_FORMAT_BC5_SNORM                   = 84,
		DXGI_FORMAT_B5G6R5_UNORM                = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
		DXGI_FORMAT_BC6H_TYPELESS               = 94,
		DXGI_FORMAT_BC6H_UF16                   = 95,
		DXGI_FORMAT_BC6H_SF16                   = 96,
		DXGI_FORMAT_BC7_TYPELESS                = 97,
		DXGI_FORMAT_BC7_UNORM                   = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
		DXGI_FORMAT_AYUV                        = 100,
		DXGI_FORMAT_Y410                        = 101,
		DXGI_FORMAT_Y416                        = 102,
		DXGI_FORMAT_NV12                        = 103,
		DXGI_FORMAT_P010                        = 104,
		DXGI_FORMAT_P016                        = 105,
		DXGI_FORMAT_420_OPAQUE                  = 106,
		DXGI_FORMAT_YUY2                        = 107,
		DXGI_FORMAT_Y210                        = 108,
		DXGI_FORMAT_Y216                        = 109,
		DXGI_FORMAT_NV11                        = 110,
		DXGI_FORMAT_AI44                        = 111,
		DXGI_FORMAT_IA44                        = 112,
		DXGI_FORMAT_P8                          = 113,
		DXGI_FORMAT_A8P8                        = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
		DXGI_FORMAT_FORCE_UINT                  = 0xffffffff
	};
#endif

	ElementFormat FromDXGIFormat(uint32_t format)
	{
		switch (format)
		{
		case DXGI_FORMAT_A8_UNORM:
			return EF_A8;

		case DXGI_FORMAT_B5G6R5_UNORM:
			return EF_R5G6B5;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return EF_A1RGB5;

		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return EF_ARGB4;

		case DXGI_FORMAT_R8_UNORM:
			return EF_R8;

		case DXGI_FORMAT_R8_SNORM:
			return EF_SIGNED_R8;

		case DXGI_FORMAT_R8G8_UNORM:
			return EF_GR8;

		case DXGI_FORMAT_R8G8_SNORM:
			return EF_SIGNED_GR8;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return EF_ARGB8;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return EF_ABGR8;

		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return EF_SIGNED_ABGR8;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return EF_A2BGR10;

		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			return EF_SIGNED_A2BGR10;

		case DXGI_FORMAT_R8_UINT:
			return EF_R8UI;

		case DXGI_FORMAT_R8_SINT:
			return EF_R8I;

		case DXGI_FORMAT_R8G8_UINT:
			return EF_GR8UI;

		case DXGI_FORMAT_R8G8_SINT:
			return EF_GR8I;

		case DXGI_FORMAT_R8G8B8A8_UINT:
			return EF_ABGR8UI;

		case DXGI_FORMAT_R8G8B8A8_SINT:
			return EF_ABGR8I;

		case DXGI_FORMAT_R10G10B10A2_UINT:
			return EF_A2BGR10UI;

		case DXGI_FORMAT_R16_UNORM:
			return EF_R16;

		case DXGI_FORMAT_R16_SNORM:
			return EF_SIGNED_R16;

		case DXGI_FORMAT_R16G16_UNORM:
			return EF_GR16;

		case DXGI_FORMAT_R16G16_SNORM:
			return EF_SIGNED_GR16;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return EF_ABGR16;

		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return EF_SIGNED_ABGR16;

		case DXGI_FORMAT_R16_UINT:
			return EF_R16UI;

		case DXGI_FORMAT_R16_SINT:
			return EF_R16I;

		case DXGI_FORMAT_R16G16_UINT:
			return EF_GR16UI;

		case DXGI_FORMAT_R16G16_SINT:
			return EF_GR16I;

		case DXGI_FORMAT_R16G16B16A16_UINT:
			return EF_ABGR16UI;

		case DXGI_FORMAT_R16G16B16A16_SINT:
			return EF_ABGR16I;

		case DXGI_FORMAT_R32_UINT:
			return EF_R32UI;

		case DXGI_FORMAT_R32_SINT:
			return EF_R32I;

		case DXGI_FORMAT_R32G32_UINT:
			return EF_GR32UI;

		case DXGI_FORMAT_R32G32_SINT:
			return EF_GR32I;

		case DXGI_FORMAT_R32G32B32_UINT:
			return EF_BGR32UI;

		case DXGI_FORMAT_R32G32B32_SINT:
			return EF_BGR32I;

		case DXGI_FORMAT_R32G32B32A32_UINT:
			return EF_ABGR32UI;

		case DXGI_FORMAT_R32G32B32A32_SINT:
			return EF_ABGR32I;

		case DXGI_FORMAT_R16_FLOAT:
			return EF_R16F;

		case DXGI_FORMAT_R16G16_FLOAT:
			return EF_GR16F;

		case DXGI_FORMAT_R11G11B10_FLOAT:
			return EF_B10G11R11F;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return EF_ABGR16F;

		case DXGI_FORMAT_R32_FLOAT:
			return EF_R32F;

		case DXGI_FORMAT_R32G32_FLOAT:
			return EF_GR32F;

		case DXGI_FORMAT_R32G32B32_FLOAT:
			return EF_BGR32F;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return EF_ABGR32F;

		case DXGI_FORMAT_BC1_UNORM:
			return EF_BC1;

		case DXGI_FORMAT_BC2_UNORM:
			return EF_BC2;

		case DXGI_FORMAT_BC3_UNORM:
			return EF_BC3;

		case DXGI_FORMAT_BC4_UNORM:
			return EF_BC4;

		case DXGI_FORMAT_BC4_SNORM:
			return EF_SIGNED_BC4;

		case DXGI_FORMAT_BC5_UNORM:
			return EF_BC5;

		case DXGI_FORMAT_BC5_SNORM:
			return EF_SIGNED_BC5;

		case DXGI_FORMAT_BC6H_UF16:
			return EF_BC6;

		case DXGI_FORMAT_BC6H_SF16:
			return EF_SIGNED_BC6;

		case DXGI_FORMAT_BC7_UNORM:
			return EF_BC7;

		case DXGI_FORMAT_D16_UNORM:
			return EF_D16;

		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return EF_D24S8;

		case DXGI_FORMAT_D32_FLOAT:
			return EF_D32F;

		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return EF_ABGR8_SRGB;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return EF_BC1_SRGB;

		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return EF_BC2_SRGB;

		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return EF_BC3_SRGB;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return EF_BC7_SRGB;

			// My extensions for ETC

		case 0x80000000UL:
			return EF_ETC1;

		case 0x80000001UL:
			return EF_ETC2_R11;

		case 0x80000002UL:
			return EF_SIGNED_ETC2_R11;

		case 0x80000003UL:
			return EF_ETC2_GR11;

		case 0x80000004UL:
			return EF_SIGNED_ETC2_GR11;

		case 0x80000005UL:
			return EF_ETC2_BGR8;

		case 0x80000006UL:
			return EF_ETC2_BGR8_SRGB;

		case 0x80000007UL:
			return EF_ETC2_A1BGR8;

		case 0x80000008UL:
			return EF_ETC2_A1BGR8_SRGB;

		case 0x80000009UL:
			return EF_ETC2_ABGR8;

		case 0x8000000AUL:
			return EF_ETC2_ABGR8_SRGB;

		default:
			ZENGINE_UNREACHABLE("Invalid format");
		}
	}

	DXGI_FORMAT ToDXGIFormat(ElementFormat format)
	{
		switch (format)
		{
		case EF_A8:
			return DXGI_FORMAT_A8_UNORM;

		case EF_R5G6B5:
			return DXGI_FORMAT_B5G6R5_UNORM;

		case EF_A1RGB5:
			return DXGI_FORMAT_B5G5R5A1_UNORM;

		case EF_ARGB4:
			return DXGI_FORMAT_B4G4R4A4_UNORM;

		case EF_R8:
			return DXGI_FORMAT_R8_UNORM;

		case EF_SIGNED_R8:
			return DXGI_FORMAT_R8_SNORM;

		case EF_GR8:
			return DXGI_FORMAT_R8G8_UNORM;

		case EF_SIGNED_GR8:
			return DXGI_FORMAT_R8G8_SNORM;

		case EF_ARGB8:
		case EF_ARGB8_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case EF_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case EF_SIGNED_ABGR8:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

		case EF_A2BGR10:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case EF_SIGNED_A2BGR10:
			return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

		case EF_R8UI:
			return DXGI_FORMAT_R8_UINT;

		case EF_R8I:
			return DXGI_FORMAT_R8_SINT;

		case EF_GR8UI:
			return DXGI_FORMAT_R8G8_UINT;

		case EF_GR8I:
			return DXGI_FORMAT_R8G8_SINT;

		case EF_ABGR8UI:
			return DXGI_FORMAT_R8G8B8A8_UINT;

		case EF_ABGR8I:
			return DXGI_FORMAT_R8G8B8A8_SINT;

		case EF_A2BGR10UI:
			return DXGI_FORMAT_R10G10B10A2_UINT;

		case EF_R16:
			return DXGI_FORMAT_R16_UNORM;

		case EF_SIGNED_R16:
			return DXGI_FORMAT_R16_SNORM;

		case EF_GR16:
			return DXGI_FORMAT_R16G16_UNORM;

		case EF_SIGNED_GR16:
			return DXGI_FORMAT_R16G16_SNORM;

		case EF_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

		case EF_SIGNED_ABGR16:
			return DXGI_FORMAT_R16G16B16A16_SNORM;

		case EF_R16UI:
			return DXGI_FORMAT_R16_UINT;

		case EF_R16I:
			return DXGI_FORMAT_R16_SINT;

		case EF_GR16UI:
			return DXGI_FORMAT_R16G16_UINT;

		case EF_GR16I:
			return DXGI_FORMAT_R16G16_SINT;

		case EF_ABGR16UI:
			return DXGI_FORMAT_R16G16B16A16_UINT;

		case EF_ABGR16I:
			return DXGI_FORMAT_R16G16B16A16_SINT;

		case EF_R32UI:
			return DXGI_FORMAT_R32_UINT;

		case EF_R32I:
			return DXGI_FORMAT_R32_SINT;

		case EF_GR32UI:
			return DXGI_FORMAT_R32G32_UINT;

		case EF_GR32I:
			return DXGI_FORMAT_R32G32_SINT;

		case EF_BGR32UI:
			return DXGI_FORMAT_R32G32B32_UINT;

		case EF_BGR32I:
			return DXGI_FORMAT_R32G32B32_SINT;

		case EF_ABGR32UI:
			return DXGI_FORMAT_R32G32B32A32_UINT;

		case EF_ABGR32I:
			return DXGI_FORMAT_R32G32B32A32_SINT;

		case EF_R16F:
			return DXGI_FORMAT_R16_FLOAT;

		case EF_GR16F:
			return DXGI_FORMAT_R16G16_FLOAT;

		case EF_B10G11R11F:
			return DXGI_FORMAT_R11G11B10_FLOAT;

		case EF_ABGR16F:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case EF_R32F:
			return DXGI_FORMAT_R32_FLOAT;

		case EF_GR32F:
			return DXGI_FORMAT_R32G32_FLOAT;

		case EF_BGR32F:
			return DXGI_FORMAT_R32G32B32_FLOAT;

		case EF_ABGR32F:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case EF_BC1:
			return DXGI_FORMAT_BC1_UNORM;

		case EF_BC2:
			return DXGI_FORMAT_BC2_UNORM;

		case EF_BC3:
			return DXGI_FORMAT_BC3_UNORM;

		case EF_BC4:
			return DXGI_FORMAT_BC4_UNORM;

		case EF_SIGNED_BC4:
			return DXGI_FORMAT_BC4_SNORM;

		case EF_BC5:
			return DXGI_FORMAT_BC5_UNORM;

		case EF_SIGNED_BC5:
			return DXGI_FORMAT_BC5_SNORM;

		case EF_BC6:
			return DXGI_FORMAT_BC6H_UF16;

		case EF_SIGNED_BC6:
			return DXGI_FORMAT_BC6H_SF16;

		case EF_BC7:
			return DXGI_FORMAT_BC7_UNORM;

		case EF_D16:
			return DXGI_FORMAT_D16_UNORM;

		case EF_D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case EF_D32F:
			return DXGI_FORMAT_D32_FLOAT;

		case EF_ABGR8_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case EF_BC1_SRGB:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case EF_BC2_SRGB:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case EF_BC3_SRGB:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case EF_BC7_SRGB:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

			// My extensions for ETC

		case EF_ETC1:
			return static_cast<DXGI_FORMAT>(0x80000000UL);

		case EF_ETC2_R11:
			return static_cast<DXGI_FORMAT>(0x80000001UL);

		case EF_SIGNED_ETC2_R11:
			return static_cast<DXGI_FORMAT>(0x80000002UL);

		case EF_ETC2_GR11:
			return static_cast<DXGI_FORMAT>(0x80000003UL);

		case EF_SIGNED_ETC2_GR11:
			return static_cast<DXGI_FORMAT>(0x80000004UL);

		case EF_ETC2_BGR8:
			return static_cast<DXGI_FORMAT>(0x80000005UL);

		case EF_ETC2_BGR8_SRGB:
			return static_cast<DXGI_FORMAT>(0x80000006UL);

		case EF_ETC2_A1BGR8:
			return static_cast<DXGI_FORMAT>(0x80000007UL);

		case EF_ETC2_A1BGR8_SRGB:
			return static_cast<DXGI_FORMAT>(0x80000008UL);

		case EF_ETC2_ABGR8:
			return static_cast<DXGI_FORMAT>(0x80000009UL);

		case EF_ETC2_ABGR8_SRGB:
			return static_cast<DXGI_FORMAT>(0x8000000AUL);

		default:
			ZENGINE_UNREACHABLE("Invalid format");
		}
	}

    void ReadDdsFileHeader(const ResIdentifierPtr& tex_res, Texture::TextureType& type,
        uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
        ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch)
    {
        uint32_t magic;
        tex_res->read(&magic, sizeof(magic));
        magic = LE2Native(magic);
        COMMON_ASSERT((MakeFourCC<'D', 'D', 'S', ' '>::value) == magic);
    
        DDSSURFACEDESC2 desc;
        tex_res->read(&desc, sizeof(desc));
        desc.size = LE2Native(desc.size);
        desc.flags = LE2Native(desc.flags);
        desc.height = LE2Native(desc.height);
        desc.width = LE2Native(desc.width);
        desc.pitch = LE2Native(desc.pitch);
        desc.depth = LE2Native(desc.depth);
        desc.mip_map_count = LE2Native(desc.mip_map_count);
        for (uint32_t i = 0; i < std::size(desc.reserved1); ++ i)
        {
            desc.reserved1[i] = LE2Native(desc.reserved1[i]);
        }
        desc.pixel_format.size = LE2Native(desc.pixel_format.size);
        desc.pixel_format.flags = LE2Native(desc.pixel_format.flags);
        desc.pixel_format.four_cc = LE2Native(desc.pixel_format.four_cc);
        desc.pixel_format.rgb_bit_count = LE2Native(desc.pixel_format.rgb_bit_count);
        desc.pixel_format.r_bit_mask = LE2Native(desc.pixel_format.r_bit_mask);
        desc.pixel_format.g_bit_mask = LE2Native(desc.pixel_format.g_bit_mask);
        desc.pixel_format.b_bit_mask = LE2Native(desc.pixel_format.b_bit_mask);
        desc.pixel_format.rgb_alpha_bit_mask = LE2Native(desc.pixel_format.rgb_alpha_bit_mask);
        desc.dds_caps.caps1 = LE2Native(desc.dds_caps.caps1);
        desc.dds_caps.caps2 = LE2Native(desc.dds_caps.caps2);
        for (uint32_t i = 0; i < std::size(desc.dds_caps.reserved); ++ i)
        {
            desc.dds_caps.reserved[i] = LE2Native(desc.dds_caps.reserved[i]);
        }
        desc.reserved2 = LE2Native(desc.reserved2);

        DDS_HEADER_DXT10 desc10;
		if (MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc)
		{
			tex_res->read(&desc10, sizeof(desc10));
			desc10.dxgi_format = LE2Native(desc10.dxgi_format);
			desc10.resource_dim = LE2Native(desc10.resource_dim);
			desc10.misc_flag = LE2Native(desc10.misc_flag);
			desc10.array_size = LE2Native(desc10.array_size);
			desc10.reserved = LE2Native(desc10.reserved);
			desc10.array_size = LE2Native(desc10.array_size);
			array_size = desc10.array_size;
		}
		else
		{
			std::memset(&desc10, 0, sizeof(desc10));
			array_size = 1;

			COMMON_ASSERT((desc.flags & DDSD_CAPS) != 0);
			COMMON_ASSERT((desc.flags & DDSD_PIXELFORMAT) != 0);
		}

        COMMON_ASSERT((desc.flags & DDSD_WIDTH) != 0);
		COMMON_ASSERT((desc.flags & DDSD_HEIGHT) != 0);

		if (0 == (desc.flags & DDSD_MIPMAPCOUNT))
		{
			desc.mip_map_count = 1;
		}

		format = EF_ARGB8;
		if ((desc.pixel_format.flags & DDSPF_FOURCC) != 0)
		{
			// From "Programming Guide for DDS", http://msdn.microsoft.com/en-us/library/bb943991.aspx
			switch (desc.pixel_format.four_cc)
			{
			case 36:
				format = EF_ABGR16;
				break;

			case 110:
				format = EF_SIGNED_ABGR16;
				break;

			case 111:
				format = EF_R16F;
				break;

			case 112:
				format = EF_GR16F;
				break;

			case 113:
				format = EF_ABGR16F;
				break;

			case 114:
				format = EF_R32F;
				break;

			case 115:
				format = EF_GR32F;
				break;

			case 116:
				format = EF_ABGR32F;
				break;

			case MakeFourCC<'D', 'X', 'T', '1'>::value:
				format = EF_BC1;
				break;

			case MakeFourCC<'D', 'X', 'T', '3'>::value:
				format = EF_BC2;
				break;

			case MakeFourCC<'D', 'X', 'T', '5'>::value:
				format = EF_BC3;
				break;

			case MakeFourCC<'B', 'C', '4', 'U'>::value:
			case MakeFourCC<'A', 'T', 'I', '1'>::value:
				format = EF_BC4;
				break;

			case MakeFourCC<'B', 'C', '4', 'S'>::value:
				format = EF_SIGNED_BC4;
				break;

			case MakeFourCC<'B', 'C', '5', 'U'>::value:
			case MakeFourCC<'A', 'T', 'I', '2'>::value:
				format = EF_BC5;
				break;

			case MakeFourCC<'B', 'C', '5', 'S'>::value:
				format = EF_SIGNED_BC5;
				break;

			case MakeFourCC<'D', 'X', '1', '0'>::value:
				format = FromDXGIFormat(desc10.dxgi_format);
				break;
			}
		}
		else
		{
			if ((desc.pixel_format.flags & DDSPF_RGB) != 0)
			{
				switch (desc.pixel_format.rgb_bit_count)
				{
				case 16:
					if ((0xF000 == desc.pixel_format.rgb_alpha_bit_mask)
						&& (0x0F00 == desc.pixel_format.r_bit_mask)
						&& (0x00F0 == desc.pixel_format.g_bit_mask)
						&& (0x000F == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB4;
					}
					else
					{
						ZENGINE_UNREACHABLE("Invalid format");
					}
					break;

				case 32:
					if ((0x00FF0000 == desc.pixel_format.r_bit_mask)
						&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
						&& (0x000000FF == desc.pixel_format.b_bit_mask))
					{
						format = EF_ARGB8;
					}
					else
					{
						if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
							&& (0x000003FF == desc.pixel_format.r_bit_mask)
							&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
							&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
						{
							format = EF_A2BGR10;
						}
						else
						{
							if ((0xFF000000 == desc.pixel_format.rgb_alpha_bit_mask)
								&& (0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_ABGR8;
							}
							else
							{
								if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
									&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
									&& (0x00000000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_GR16;
								}
								else
								{
									ZENGINE_UNREACHABLE("Invalid format");
								}
							}
						}
					}
					break;

				default:
					ZENGINE_UNREACHABLE("Invalid rgb bit count");
				}
			}
			else
			{
				if ((desc.pixel_format.flags & DDSPF_LUMINANCE) != 0)
				{
					switch (desc.pixel_format.rgb_bit_count)
					{
					case 8:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R8;
						}
						else
						{
							ZENGINE_UNREACHABLE("Invalid format");
						}
						break;

					case 16:
						if (0 == (desc.pixel_format.flags & DDSPF_ALPHAPIXELS))
						{
							format = EF_R16;
						}
						else
						{
							ZENGINE_UNREACHABLE("Invalid format");
						}
						break;

					default:
						ZENGINE_UNREACHABLE("Invalid rgb bit count");
					}
				}
				else
				{
					if ((desc.pixel_format.flags & DDSPF_BUMPDUDV) != 0)
					{
						switch (desc.pixel_format.rgb_bit_count)
						{
						case 16:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask))
							{
								format = EF_SIGNED_GR8;
							}
							else
							{
								if (0x0000FFFF == desc.pixel_format.r_bit_mask)
								{
									format = EF_SIGNED_R16;
								}
								else
								{
									ZENGINE_UNREACHABLE("Invalid format");
								}
							}
							break;

						case 32:
							if ((0x000000FF == desc.pixel_format.r_bit_mask)
								&& (0x0000FF00 == desc.pixel_format.g_bit_mask)
								&& (0x00FF0000 == desc.pixel_format.b_bit_mask))
							{
								format = EF_SIGNED_ABGR8;
							}
							else
							{
								if ((0xC0000000 == desc.pixel_format.rgb_alpha_bit_mask)
									&& (0x000003FF == desc.pixel_format.r_bit_mask)
									&& (0x000FFC00 == desc.pixel_format.g_bit_mask)
									&& (0x3FF00000 == desc.pixel_format.b_bit_mask))
								{
									format = EF_SIGNED_A2BGR10;
								}
								else
								{
									if ((0x00000000 == desc.pixel_format.rgb_alpha_bit_mask)
										&& (0x0000FFFF == desc.pixel_format.r_bit_mask)
										&& (0xFFFF0000 == desc.pixel_format.g_bit_mask)
										&& (0x00000000 == desc.pixel_format.b_bit_mask))
									{
										format = EF_SIGNED_GR16;
									}
									else
									{
										ZENGINE_UNREACHABLE("Invalid format");
									}
								}
							}
							break;

						default:
							ZENGINE_UNREACHABLE("Invalid rgb bit count");
						}
					}
					else
					{
						if ((desc.pixel_format.flags & DDSPF_ALPHA) != 0)
						{
							format = EF_A8;
						}
						else
						{
							ZENGINE_UNREACHABLE("Invalid alpha format");
						}
					}
				}
			}
		}

		if ((desc.flags & DDSD_PITCH) != 0)
		{
			row_pitch = desc.pitch;
		}
		else
		{
			if ((desc.flags & desc.pixel_format.flags & 0x00000040) != 0)
			{
				row_pitch = desc.width * desc.pixel_format.rgb_bit_count / 8;
			}
			else
			{
				row_pitch = desc.width * NumFormatBytes(format);
			}
		}
		slice_pitch = row_pitch * desc.height;

		if (desc.reserved1[0] != 0)
		{
			format = MakeSRGB(format);
		}

		width = desc.width;
		num_mipmaps = desc.mip_map_count;

		if ((MakeFourCC<'D', 'X', '1', '0'>::value == desc.pixel_format.four_cc))
		{
			if (D3D_RESOURCE_MISC_TEXTURECUBE == desc10.misc_flag)
			{
				type = Texture::TT_Cube;
				array_size /= 6;
				height = desc.width;
				depth = 1;
			}
			else
			{
				switch (desc10.resource_dim)
				{
				case D3D_RESOURCE_DIMENSION_TEXTURE1D:
					type = Texture::TT_1D;
					height = 1;
					depth = 1;
					break;

				case D3D_RESOURCE_DIMENSION_TEXTURE2D:
					type = Texture::TT_2D;
					height = desc.height;
					depth = 1;
					break;

				case D3D_RESOURCE_DIMENSION_TEXTURE3D:
					type = Texture::TT_3D;
					height = desc.height;
					depth = desc.depth;
					break;

				default:
					ZENGINE_UNREACHABLE("Invalid resource dimension");
				}
			}	
		}
		else
		{
			if ((desc.dds_caps.caps2 & DDSCAPS2_CUBEMAP) != 0)
			{
				type = Texture::TT_Cube;
				height = desc.width;
				depth = 1;
			}
			else
			{
				if ((desc.dds_caps.caps2 & DDSCAPS2_VOLUME) != 0)
				{
					type = Texture::TT_3D;
					height = desc.height;
					depth = desc.depth;
				}
				else
				{
					type = Texture::TT_2D;
					height = desc.height;
					depth = 1;
				}
			}
		}
    }
}// namespace

namespace RenderWorker
{
	
Texture::Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
    : type_(type), sample_count_(sample_count), sample_quality_(sample_quality), access_hint_(access_hint)
{
    
}

Texture::~Texture() noexcept = default;

Texture::TextureType Texture::Type() const
{
    return type_;
}

uint32_t Texture::SampleCount() const
{
    return sample_count_;
}

uint32_t Texture::SampleQuality() const
{
    return sample_quality_;
}

uint32_t Texture::AccessHint() const
{
    return access_hint_;
}

uint32_t Texture::MipMapsNum() const
{
    return mip_maps_num_;
}

uint32_t Texture::ArraySize() const
{
    return array_size_;
}

ElementFormat Texture::Format() const
{
    return format_;
}












VirtualTexture::VirtualTexture(TextureType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mipmaps, uint32_t array_size,
    ElementFormat format, bool ref_only)
    : Texture(type, 1, 0, EAH_CPU_Read | EAH_CPU_Write), 
        ref_only_(ref_only), width_(width), height_(height), depth_(depth)
{
    if (0 == num_mipmaps)
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

    mip_maps_num_ = num_mipmaps;
    array_size_ = array_size;
    format_ = format;
}

uint32_t VirtualTexture::Width(uint32_t level) const 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, width_ >> level);
}

uint32_t VirtualTexture::Height(uint32_t level) const 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, height_ >> level);
}

uint32_t VirtualTexture::Depth(uint32_t level) const 
{
    COMMON_ASSERT(level < mip_maps_num_);
    return std::max(1U, depth_ >> level);
}

void VirtualTexture::CreateHWResource(std::span<ElementInitData const> init_data, [[maybe_unused]] const float4* clear_value_hint)
{
    uint32_t const num_faces = (type_ == TT_Cube) ? 6 : 1;
    uint32_t const num_subres = mip_maps_num_ * array_size_ * num_faces;

    if (init_data.empty())
    {
        COMMON_ASSERT(!ref_only_);

        subres_data_.resize(num_subres);

        uint32_t const block_width = BlockWidth(format_);
        uint32_t const block_height = BlockHeight(format_);
        uint32_t const block_bytes = BlockBytes(format_);
        for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
        {
            for (uint32_t face = 0; face < num_faces; ++ face)
            {
                for (uint32_t level = 0; level < mip_maps_num_; ++ level)
                {
                    size_t const subres = (array_index * num_faces + face) * mip_maps_num_ + level;

                    subres_data_[subres].row_pitch = (this->Width(level) + block_width - 1) / block_width * block_bytes;
                    subres_data_[subres].slice_pitch
                        = (this->Height(level) + block_height - 1) / block_height * subres_data_[subres].row_pitch;
                }
            }
        }
    }
    else
    {
        COMMON_ASSERT(init_data.size() >= num_subres);

        subres_data_.assign(init_data.begin(), init_data.begin() + num_subres);
    }
    mapped_.resize(num_subres);

    for (auto& item : mapped_)
    {
        item = MakeUniquePtr<std::atomic<bool>>(false);
    }

    data_block_.clear();
    if (!ref_only_)
    {
        uint32_t size = 0;
        for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
        {
            for (uint32_t face = 0; face < num_faces; ++ face)
            {
                for (uint32_t level = 0; level < mip_maps_num_; ++ level)
                {
                    size_t const subres = (array_index * num_faces + face) * mip_maps_num_ + level;
                    size += subres_data_[subres].slice_pitch * this->Depth(level);
                }
            }
        }

        data_block_.reserve(size);
        for (uint32_t array_index = 0; array_index < array_size_; ++ array_index)
        {
            for (uint32_t face = 0; face < num_faces; ++ face)
            {
                for (uint32_t level = 0; level < mip_maps_num_; ++ level)
                {
                    size_t const subres = (array_index * num_faces + face) * mip_maps_num_ + level;

                    subres_data_[subres].data = data_block_.data() + data_block_.size();
                    uint32_t const subres_size = subres_data_[subres].slice_pitch * this->Depth(level);
                    if (init_data.empty())
                    {
                        data_block_.resize(data_block_.size() + subres_size);
                    }
                    else
                    {
                        uint8_t const * p = static_cast<uint8_t const *>(init_data[subres].data);
                        data_block_.insert(data_block_.end(), p, p + subres_size);
                    }
                }
            }
        }

        if (init_data.empty())
        {
            std::memset(data_block_.data(), 0, data_block_.size());
        }
    }
}

void VirtualTexture::DeleteHWResource()
{
    subres_data_.clear();
    data_block_.clear();
    mapped_.clear();
}

bool VirtualTexture::HWResourceReady() const 
{
    return !subres_data_.empty();
}










void GetImageInfo(std::string_view tex_name, Texture::TextureType& type,
    uint32_t& width, uint32_t& height, uint32_t& depth, uint32_t& num_mipmaps, uint32_t& array_size,
    ElementFormat& format, uint32_t& row_pitch, uint32_t& slice_pitch)
{
    std::string path_file = tex_name.data();
    size_t lastIndex = path_file.rfind("\\");
    std::string package_path = path_file.substr(0, lastIndex);
    std::string name = path_file.substr(lastIndex + 1);

    uint64_t const timestamp = std::filesystem::last_write_time(package_path).time_since_epoch().count();
    ResIdentifierPtr tex_res = MakeSharedPtr<ResIdentifier>(
        name, timestamp, MakeSharedPtr<std::ifstream>(path_file.c_str(), std::ios_base::binary));
    
    std::filesystem::path res_path(tex_name);
    if (res_path.extension().string() != ".dds")
    {
        // TexConverter::GetImageInfo(tex_res, metadata_name, nullptr,
		// 		type, width, height, depth, num_mipmaps, array_size, format, row_pitch, slice_pitch);
    }
    else
    {
        ReadDdsFileHeader(tex_res, type, width, height, depth, num_mipmaps, array_size, format,
            row_pitch, slice_pitch);
    }
}

TexturePtr SyncLoadTexture(std::string_view tex_name, uint32_t access_hint)
{
	TexturePtr tex = LoadVirtualTexture(tex_name);

	auto& sw_tex = checked_cast<VirtualTexture&>(*tex);
	std::vector<ElementInitData> init_data;
	init_data = sw_tex.SubresourceData();
	auto& rf = Context::Instance().RenderFactoryInstance();

	TexturePtr texture;
	switch (tex->Type())
	{
	case Texture::TT_1D:
		break;

	case Texture::TT_2D:
		texture = rf.MakeTexture2D(tex->Width(0), tex->Height(0), tex->MipMapsNum(), 
		tex->ArraySize(), tex->Format(), 1, 0, access_hint);
		break;

	case Texture::TT_3D:
		break;

	case Texture::TT_Cube:
		break;

	default:
		ZENGINE_UNREACHABLE("Invalid texture type");
	}
	texture->CreateHWResource(init_data, nullptr);
	return texture;
}

TexturePtr LoadVirtualTexture(std::string_view tex_name)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();
    ResIdentifierPtr tex_res = res_loader.Open(tex_name);

    Texture::TextureType type;
    uint32_t width, height, depth;
    uint32_t num_mipmaps;
    uint32_t array_size;
    ElementFormat format;
    std::vector<ElementInitData> init_data;
    std::vector<uint8_t> data_block;
    uint32_t row_pitch, slice_pitch;
    ReadDdsFileHeader(tex_res, type, width, height, depth, num_mipmaps, array_size, format,
        row_pitch, slice_pitch);

    uint32_t const fmt_size = NumFormatBytes(format);
    bool padding = false;
    if (!IsCompressedFormat(format))
    {
        if (row_pitch != width * fmt_size)
        {
            COMMON_ASSERT(row_pitch == ((width + 3) & ~3) * fmt_size);
            padding = true;
        }
    }

    std::vector<size_t> base;
    switch (type)
    {
    case Texture::TT_1D:
        {
        }
        break;

    case Texture::TT_2D:
        {
            init_data.resize(array_size * num_mipmaps);
            base.resize(array_size * num_mipmaps);
            for (uint32_t array_index = 0; array_index < array_size; ++ array_index)
            {
                uint32_t the_width = width;
                uint32_t the_height = height;
                for (uint32_t level = 0; level < num_mipmaps; ++ level)
                {
                    size_t const index = array_index * num_mipmaps + level;
                    if (IsCompressedFormat(format))
                    {
                        uint32_t const block_size = NumFormatBytes(format) * 4;
                        uint32_t image_size = ((the_width + 3) / 4) * ((the_height + 3) / 4) * block_size;

                        base[index] = data_block.size();
                        data_block.resize(base[index] + image_size);
                        init_data[index].row_pitch = (the_width + 3) / 4 * block_size;
                        init_data[index].slice_pitch = image_size;

                        tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(image_size));
                        COMMON_ASSERT(tex_res->gcount() == static_cast<int>(image_size));
                    }
                    else
                    {
                        init_data[index].row_pitch = (padding ? ((the_width + 3) & ~3) : the_width) * fmt_size;
                        init_data[index].slice_pitch = init_data[index].row_pitch * the_height;
                        base[index] = data_block.size();
                        data_block.resize(base[index] + init_data[index].slice_pitch);

                        tex_res->read(&data_block[base[index]], static_cast<std::streamsize>(init_data[index].slice_pitch));
                        COMMON_ASSERT(tex_res->gcount() == static_cast<int>(init_data[index].slice_pitch));
                    }

                    the_width = std::max(the_width / 2, 1U);
                    the_height = std::max(the_height / 2, 1U);
                }
            }
        }
        break;

    case Texture::TT_3D:
        {
        }
        break;    

    case Texture::TT_Cube:
        {
        }
        break;
    }

    for (size_t i = 0; i < base.size(); ++ i)
    {
        init_data[i].data = &data_block[base[i]];
    }
    auto ret = MakeSharedPtr<VirtualTexture>(type, width, height, depth, num_mipmaps, array_size, format, false);
    ret->CreateHWResource(init_data, nullptr);
    return ret;
}

}