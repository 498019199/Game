#include "MetalUtil.h"
#include <render/RenderStateObject.h>

namespace RenderWorker
{

metal::TextureFormat MetalMapping::MappingFormat(ElementFormat format)
{
	switch (format)
	{
	case EF_R8:				return metal::TEXTUREFORMAT_R8_UNORM;
	case EF_GR8:			return metal::TEXTUREFORMAT_R8G8_UNORM;
	case EF_BGR8:			return metal::TEXTUREFORMAT_R8G8B8A8_UNORM; // padded/converted at upload
	case EF_ABGR8:			return metal::TEXTUREFORMAT_R8G8B8A8_UNORM;
	case EF_ARGB8:			return metal::TEXTUREFORMAT_B8G8R8A8_UNORM;
	case EF_ABGR8_SRGB:		return metal::TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB;
	case EF_ARGB8_SRGB:		return metal::TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB;
	case EF_A2BGR10:		return metal::TEXTUREFORMAT_R10G10B10A2_UNORM;
	case EF_R16F:			return metal::TEXTUREFORMAT_R16_FLOAT;
	case EF_GR16F:			return metal::TEXTUREFORMAT_R16G16_FLOAT;
	case EF_ABGR16F:		return metal::TEXTUREFORMAT_R16G16B16A16_FLOAT;
	case EF_R32F:			return metal::TEXTUREFORMAT_R32_FLOAT;
	case EF_GR32F:			return metal::TEXTUREFORMAT_R32G32_FLOAT;
	case EF_ABGR32F:		return metal::TEXTUREFORMAT_R32G32B32A32_FLOAT;
	case EF_BC1:			return metal::TEXTUREFORMAT_BC1_RGBA_UNORM;
	case EF_BC1_SRGB:		return metal::TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB;
	case EF_BC2:			return metal::TEXTUREFORMAT_BC2_RGBA_UNORM;
	case EF_BC2_SRGB:		return metal::TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB;
	case EF_BC3:			return metal::TEXTUREFORMAT_BC3_RGBA_UNORM;
	case EF_BC3_SRGB:		return metal::TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB;
	case EF_BC4:			return metal::TEXTUREFORMAT_BC4_R_UNORM;
	case EF_BC5:			return metal::TEXTUREFORMAT_BC5_RG_UNORM;
	case EF_D16:			return metal::TEXTUREFORMAT_D16_UNORM;
	case EF_D24S8:			return metal::TEXTUREFORMAT_D24_UNORM_S8_UINT;
	case EF_D32F:			return metal::TEXTUREFORMAT_D32_FLOAT;
	default:
		return metal::TEXTUREFORMAT_R8G8B8A8_UNORM;
	}
}

ElementFormat MetalMapping::MappingFormat(metal::TextureFormat fmt)
{
	switch (fmt)
	{
	case metal::TEXTUREFORMAT_R8_UNORM:				return EF_R8;
	case metal::TEXTUREFORMAT_R8G8_UNORM:				return EF_GR8;
	case metal::TEXTUREFORMAT_R8G8B8A8_UNORM:			return EF_ABGR8;
	case metal::TEXTUREFORMAT_B8G8R8A8_UNORM:			return EF_ARGB8;
		case metal::TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB:		return EF_ABGR8_SRGB;
		case metal::TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB:		return EF_ARGB8_SRGB;
		case metal::TEXTUREFORMAT_R10G10B10A2_UNORM:		return EF_A2BGR10;
	case metal::TEXTUREFORMAT_R16_FLOAT:				return EF_R16F;
	case metal::TEXTUREFORMAT_R16G16_FLOAT:			return EF_GR16F;
	case metal::TEXTUREFORMAT_R16G16B16A16_FLOAT:		return EF_ABGR16F;
	case metal::TEXTUREFORMAT_R32_FLOAT:				return EF_R32F;
	case metal::TEXTUREFORMAT_R32G32_FLOAT:			return EF_GR32F;
	case metal::TEXTUREFORMAT_R32G32B32A32_FLOAT:		return EF_ABGR32F;
	case metal::TEXTUREFORMAT_D16_UNORM:				return EF_D16;
	case metal::TEXTUREFORMAT_D24_UNORM_S8_UINT:		return EF_D24S8;
	case metal::TEXTUREFORMAT_D32_FLOAT:				return EF_D32F;
	default:
		return EF_ABGR8;
	}
}

metal::PrimitiveType MetalMapping::Mapping(RenderLayout::topology_type tt)
{
	switch (tt)
	{
	case RenderLayout::TT_PointList:		return metal::PRIMITIVETYPE_POINTLIST;
	case RenderLayout::TT_LineList:			return metal::PRIMITIVETYPE_LINELIST;
	case RenderLayout::TT_LineStrip:		return metal::PRIMITIVETYPE_LINESTRIP;
	case RenderLayout::TT_TriangleList:		return metal::PRIMITIVETYPE_TRIANGLELIST;
	case RenderLayout::TT_TriangleStrip:	return metal::PRIMITIVETYPE_TRIANGLESTRIP;
	default:
		return metal::PRIMITIVETYPE_TRIANGLELIST;
	}
}

metal::CullMode MetalMapping::Mapping(CullMode mode)
{
	switch (mode)
	{
	case CM_None:	return metal::CULLMODE_NONE;
	case CM_Front:	return metal::CULLMODE_FRONT;
	case CM_Back:	return metal::CULLMODE_BACK;
	default:		return metal::CULLMODE_BACK;
	}
}

metal::FillMode MetalMapping::Mapping(PolygonMode mode)
{
	switch (mode)
	{
	case PM_Line:	return metal::FILLMODE_LINE;
	case PM_Fill:	return metal::FILLMODE_FILL;
	default:		return metal::FILLMODE_FILL;
	}
}

metal::CompareOp MetalMapping::Mapping(CompareFunction func)
{
	switch (func)
	{
	case CF_AlwaysFail:		return metal::COMPAREOP_NEVER;
	case CF_AlwaysPass:		return metal::COMPAREOP_ALWAYS;
	case CF_Less:			return metal::COMPAREOP_LESS;
	case CF_LessEqual:		return metal::COMPAREOP_LESS_OR_EQUAL;
	case CF_Equal:			return metal::COMPAREOP_EQUAL;
	case CF_NotEqual:		return metal::COMPAREOP_NOT_EQUAL;
	case CF_GreaterEqual:	return metal::COMPAREOP_GREATER_OR_EQUAL;
	case CF_Greater:		return metal::COMPAREOP_GREATER;
	default:				return metal::COMPAREOP_LESS;
	}
}

metal::BlendFactor MetalMapping::Mapping(AlphaBlendFactor factor)
{
	switch (factor)
	{
	case ABF_Zero:				return metal::BLENDFACTOR_ZERO;
	case ABF_One:				return metal::BLENDFACTOR_ONE;
	case ABF_Src_Alpha:			return metal::BLENDFACTOR_SRC_ALPHA;
	case ABF_Dst_Alpha:			return metal::BLENDFACTOR_DST_ALPHA;
	case ABF_Inv_Src_Alpha:		return metal::BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	case ABF_Inv_Dst_Alpha:		return metal::BLENDFACTOR_ONE_MINUS_DST_ALPHA;
	case ABF_Src_Color:			return metal::BLENDFACTOR_SRC_COLOR;
	case ABF_Dst_Color:			return metal::BLENDFACTOR_DST_COLOR;
	case ABF_Inv_Src_Color:		return metal::BLENDFACTOR_ONE_MINUS_SRC_COLOR;
	case ABF_Inv_Dst_Color:		return metal::BLENDFACTOR_ONE_MINUS_DST_COLOR;
	case ABF_Src_Alpha_Sat:		return metal::BLENDFACTOR_SRC_ALPHA_SATURATE;
	case ABF_Blend_Factor:		return metal::BLENDFACTOR_CONSTANT_COLOR;
	case ABF_Inv_Blend_Factor:	return metal::BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR;
	default:					return metal::BLENDFACTOR_ONE;
	}
}

metal::BlendOp MetalMapping::Mapping(BlendOperation op)
{
	switch (op)
	{
	case BOP_Add:		return metal::BLENDOP_ADD;
	case BOP_Sub:		return metal::BLENDOP_SUBTRACT;
	case BOP_Rev_Sub:	return metal::BLENDOP_REVERSE_SUBTRACT;
	case BOP_Min:		return metal::BLENDOP_MIN;
	case BOP_Max:		return metal::BLENDOP_MAX;
	default:			return metal::BLENDOP_ADD;
	}
}

metal::Filter MetalMapping::MappingMinMagFilter(TexFilterOp filter, bool mag)
{
	uint32_t const bits = static_cast<uint32_t>(filter);
	bool const linear = mag ? ((bits & TFOE_Mag_Linear) != 0) : ((bits & TFOE_Min_Linear) != 0);
	if ((bits & TFOE_Anisotropic) != 0)
	{
		return metal::FILTER_LINEAR;
	}
	return linear ? metal::FILTER_LINEAR : metal::FILTER_NEAREST;
}

metal::SamplerMipmapMode MetalMapping::MappingMipMode(TexFilterOp filter)
{
	uint32_t const bits = static_cast<uint32_t>(filter);
	return ((bits & TFOE_Mip_Linear) != 0) ? metal::SAMPLERMIPMAPMODE_LINEAR : metal::SAMPLERMIPMAPMODE_NEAREST;
}

metal::SamplerAddressMode MetalMapping::Mapping(TexAddressingMode mode)
{
	switch (mode)
	{
	case TAM_Wrap:		return metal::SAMPLERADDRESSMODE_REPEAT;
	case TAM_Mirror:	return metal::SAMPLERADDRESSMODE_MIRRORED_REPEAT;
	case TAM_Clamp:		return metal::SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	case TAM_Border:	return metal::SAMPLERADDRESSMODE_CLAMP_TO_EDGE; // no border color in metal::
	default:			return metal::SAMPLERADDRESSMODE_REPEAT;
	}
}

} // namespace RenderWorker
