#include "D3D12Util.h"
#include <render/RenderStateObject.h>

namespace RenderWorker
{

d3d12::TextureFormat D3D12Mapping::MappingFormat(ElementFormat format)
{
	switch (format)
	{
	case EF_R8:				return d3d12::TEXTUREFORMAT_R8_UNORM;
	case EF_GR8:			return d3d12::TEXTUREFORMAT_R8G8_UNORM;
	case EF_BGR8:			return d3d12::TEXTUREFORMAT_R8G8B8A8_UNORM; // padded/converted at upload
	case EF_ABGR8:			return d3d12::TEXTUREFORMAT_R8G8B8A8_UNORM;
	case EF_ARGB8:			return d3d12::TEXTUREFORMAT_B8G8R8A8_UNORM;
	case EF_ABGR8_SRGB:		return d3d12::TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB;
	case EF_ARGB8_SRGB:		return d3d12::TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB;
	case EF_A2BGR10:		return d3d12::TEXTUREFORMAT_R10G10B10A2_UNORM;
	case EF_R16F:			return d3d12::TEXTUREFORMAT_R16_FLOAT;
	case EF_GR16F:			return d3d12::TEXTUREFORMAT_R16G16_FLOAT;
	case EF_ABGR16F:		return d3d12::TEXTUREFORMAT_R16G16B16A16_FLOAT;
	case EF_R32F:			return d3d12::TEXTUREFORMAT_R32_FLOAT;
	case EF_GR32F:			return d3d12::TEXTUREFORMAT_R32G32_FLOAT;
	case EF_ABGR32F:		return d3d12::TEXTUREFORMAT_R32G32B32A32_FLOAT;
	case EF_BC1:			return d3d12::TEXTUREFORMAT_BC1_RGBA_UNORM;
	case EF_BC1_SRGB:		return d3d12::TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB;
	case EF_BC2:			return d3d12::TEXTUREFORMAT_BC2_RGBA_UNORM;
	case EF_BC2_SRGB:		return d3d12::TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB;
	case EF_BC3:			return d3d12::TEXTUREFORMAT_BC3_RGBA_UNORM;
	case EF_BC3_SRGB:		return d3d12::TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB;
	case EF_BC4:			return d3d12::TEXTUREFORMAT_BC4_R_UNORM;
	case EF_BC5:			return d3d12::TEXTUREFORMAT_BC5_RG_UNORM;
	case EF_D16:			return d3d12::TEXTUREFORMAT_D16_UNORM;
	case EF_D24S8:			return d3d12::TEXTUREFORMAT_D24_UNORM_S8_UINT;
	case EF_D32F:			return d3d12::TEXTUREFORMAT_D32_FLOAT;
	default:
		return d3d12::TEXTUREFORMAT_R8G8B8A8_UNORM;
	}
}

ElementFormat D3D12Mapping::MappingFormat(d3d12::TextureFormat fmt)
{
	switch (fmt)
	{
	case d3d12::TEXTUREFORMAT_R8_UNORM:				return EF_R8;
	case d3d12::TEXTUREFORMAT_R8G8_UNORM:				return EF_GR8;
	case d3d12::TEXTUREFORMAT_R8G8B8A8_UNORM:			return EF_ABGR8;
	case d3d12::TEXTUREFORMAT_B8G8R8A8_UNORM:			return EF_ARGB8;
		case d3d12::TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB:		return EF_ABGR8_SRGB;
		case d3d12::TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB:		return EF_ARGB8_SRGB;
		case d3d12::TEXTUREFORMAT_R10G10B10A2_UNORM:		return EF_A2BGR10;
	case d3d12::TEXTUREFORMAT_R16_FLOAT:				return EF_R16F;
	case d3d12::TEXTUREFORMAT_R16G16_FLOAT:			return EF_GR16F;
	case d3d12::TEXTUREFORMAT_R16G16B16A16_FLOAT:		return EF_ABGR16F;
	case d3d12::TEXTUREFORMAT_R32_FLOAT:				return EF_R32F;
	case d3d12::TEXTUREFORMAT_R32G32_FLOAT:			return EF_GR32F;
	case d3d12::TEXTUREFORMAT_R32G32B32A32_FLOAT:		return EF_ABGR32F;
	case d3d12::TEXTUREFORMAT_D16_UNORM:				return EF_D16;
	case d3d12::TEXTUREFORMAT_D24_UNORM_S8_UINT:		return EF_D24S8;
	case d3d12::TEXTUREFORMAT_D32_FLOAT:				return EF_D32F;
	default:
		return EF_ABGR8;
	}
}

d3d12::PrimitiveType D3D12Mapping::Mapping(RenderLayout::topology_type tt)
{
	switch (tt)
	{
	case RenderLayout::TT_PointList:		return d3d12::PRIMITIVETYPE_POINTLIST;
	case RenderLayout::TT_LineList:			return d3d12::PRIMITIVETYPE_LINELIST;
	case RenderLayout::TT_LineStrip:		return d3d12::PRIMITIVETYPE_LINESTRIP;
	case RenderLayout::TT_TriangleList:		return d3d12::PRIMITIVETYPE_TRIANGLELIST;
	case RenderLayout::TT_TriangleStrip:	return d3d12::PRIMITIVETYPE_TRIANGLESTRIP;
	default:
		return d3d12::PRIMITIVETYPE_TRIANGLELIST;
	}
}

d3d12::CullMode D3D12Mapping::Mapping(CullMode mode)
{
	switch (mode)
	{
	case CM_None:	return d3d12::CULLMODE_NONE;
	case CM_Front:	return d3d12::CULLMODE_FRONT;
	case CM_Back:	return d3d12::CULLMODE_BACK;
	default:		return d3d12::CULLMODE_BACK;
	}
}

d3d12::FillMode D3D12Mapping::Mapping(PolygonMode mode)
{
	switch (mode)
	{
	case PM_Line:	return d3d12::FILLMODE_LINE;
	case PM_Fill:	return d3d12::FILLMODE_FILL;
	default:		return d3d12::FILLMODE_FILL;
	}
}

d3d12::CompareOp D3D12Mapping::Mapping(CompareFunction func)
{
	switch (func)
	{
	case CF_AlwaysFail:		return d3d12::COMPAREOP_NEVER;
	case CF_AlwaysPass:		return d3d12::COMPAREOP_ALWAYS;
	case CF_Less:			return d3d12::COMPAREOP_LESS;
	case CF_LessEqual:		return d3d12::COMPAREOP_LESS_OR_EQUAL;
	case CF_Equal:			return d3d12::COMPAREOP_EQUAL;
	case CF_NotEqual:		return d3d12::COMPAREOP_NOT_EQUAL;
	case CF_GreaterEqual:	return d3d12::COMPAREOP_GREATER_OR_EQUAL;
	case CF_Greater:		return d3d12::COMPAREOP_GREATER;
	default:				return d3d12::COMPAREOP_LESS;
	}
}

d3d12::BlendFactor D3D12Mapping::Mapping(AlphaBlendFactor factor)
{
	switch (factor)
	{
	case ABF_Zero:				return d3d12::BLENDFACTOR_ZERO;
	case ABF_One:				return d3d12::BLENDFACTOR_ONE;
	case ABF_Src_Alpha:			return d3d12::BLENDFACTOR_SRC_ALPHA;
	case ABF_Dst_Alpha:			return d3d12::BLENDFACTOR_DST_ALPHA;
	case ABF_Inv_Src_Alpha:		return d3d12::BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	case ABF_Inv_Dst_Alpha:		return d3d12::BLENDFACTOR_ONE_MINUS_DST_ALPHA;
	case ABF_Src_Color:			return d3d12::BLENDFACTOR_SRC_COLOR;
	case ABF_Dst_Color:			return d3d12::BLENDFACTOR_DST_COLOR;
	case ABF_Inv_Src_Color:		return d3d12::BLENDFACTOR_ONE_MINUS_SRC_COLOR;
	case ABF_Inv_Dst_Color:		return d3d12::BLENDFACTOR_ONE_MINUS_DST_COLOR;
	case ABF_Src_Alpha_Sat:		return d3d12::BLENDFACTOR_SRC_ALPHA_SATURATE;
	case ABF_Blend_Factor:		return d3d12::BLENDFACTOR_CONSTANT_COLOR;
	case ABF_Inv_Blend_Factor:	return d3d12::BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR;
	default:					return d3d12::BLENDFACTOR_ONE;
	}
}

d3d12::BlendOp D3D12Mapping::Mapping(BlendOperation op)
{
	switch (op)
	{
	case BOP_Add:		return d3d12::BLENDOP_ADD;
	case BOP_Sub:		return d3d12::BLENDOP_SUBTRACT;
	case BOP_Rev_Sub:	return d3d12::BLENDOP_REVERSE_SUBTRACT;
	case BOP_Min:		return d3d12::BLENDOP_MIN;
	case BOP_Max:		return d3d12::BLENDOP_MAX;
	default:			return d3d12::BLENDOP_ADD;
	}
}

d3d12::Filter D3D12Mapping::MappingMinMagFilter(TexFilterOp filter, bool mag)
{
	uint32_t const bits = static_cast<uint32_t>(filter);
	bool const linear = mag ? ((bits & TFOE_Mag_Linear) != 0) : ((bits & TFOE_Min_Linear) != 0);
	if ((bits & TFOE_Anisotropic) != 0)
	{
		return d3d12::FILTER_LINEAR;
	}
	return linear ? d3d12::FILTER_LINEAR : d3d12::FILTER_NEAREST;
}

d3d12::SamplerMipmapMode D3D12Mapping::MappingMipMode(TexFilterOp filter)
{
	uint32_t const bits = static_cast<uint32_t>(filter);
	return ((bits & TFOE_Mip_Linear) != 0) ? d3d12::SAMPLERMIPMAPMODE_LINEAR : d3d12::SAMPLERMIPMAPMODE_NEAREST;
}

d3d12::SamplerAddressMode D3D12Mapping::Mapping(TexAddressingMode mode)
{
	switch (mode)
	{
	case TAM_Wrap:		return d3d12::SAMPLERADDRESSMODE_REPEAT;
	case TAM_Mirror:	return d3d12::SAMPLERADDRESSMODE_MIRRORED_REPEAT;
	case TAM_Clamp:		return d3d12::SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	case TAM_Border:	return d3d12::SAMPLERADDRESSMODE_CLAMP_TO_EDGE; // no border color in d3d12::
	default:			return d3d12::SAMPLERADDRESSMODE_REPEAT;
	}
}

} // namespace RenderWorker
