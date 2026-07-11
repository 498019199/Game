#include "SDL3Util.h"
#include <render/RenderStateObject.h>

namespace RenderWorker
{

SDL_GPUTextureFormat SDL3Mapping::MappingFormat(ElementFormat format)
{
	switch (format)
	{
	case EF_R8:				return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	case EF_GR8:			return SDL_GPU_TEXTUREFORMAT_R8G8_UNORM;
	case EF_ABGR8:			return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	case EF_ARGB8:			return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	case EF_A2BGR10:		return SDL_GPU_TEXTUREFORMAT_A2R10G10B10_UNORM;
	case EF_R16F:			return SDL_GPU_TEXTUREFORMAT_R16_FLOAT;
	case EF_GR16F:			return SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT;
	case EF_ABGR16F:		return SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT;
	case EF_R32F:			return SDL_GPU_TEXTUREFORMAT_R32_FLOAT;
	case EF_GR32F:			return SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT;
	case EF_ABGR32F:		return SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT;
	case EF_BC1:			return SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM;
	case EF_BC2:			return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
	case EF_BC3:			return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
	case EF_BC4:			return SDL_GPU_TEXTUREFORMAT_BC4_R_UNORM;
	case EF_BC5:			return SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM;
	case EF_D16:			return SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	case EF_D24S8:			return SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
	case EF_D32F:			return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
	default:
		return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	}
}

ElementFormat SDL3Mapping::MappingFormat(SDL_GPUTextureFormat fmt)
{
	switch (fmt)
	{
	case SDL_GPU_TEXTUREFORMAT_R8_UNORM:				return EF_R8;
	case SDL_GPU_TEXTUREFORMAT_R8G8_UNORM:				return EF_GR8;
	case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM:			return EF_ABGR8;
	case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM:			return EF_ARGB8;
	case SDL_GPU_TEXTUREFORMAT_R16_FLOAT:				return EF_R16F;
	case SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT:			return EF_GR16F;
	case SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT:		return EF_ABGR16F;
	case SDL_GPU_TEXTUREFORMAT_R32_FLOAT:				return EF_R32F;
	case SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT:			return EF_GR32F;
	case SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT:		return EF_ABGR32F;
	case SDL_GPU_TEXTUREFORMAT_D16_UNORM:				return EF_D16;
	case SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT:		return EF_D24S8;
	case SDL_GPU_TEXTUREFORMAT_D32_FLOAT:				return EF_D32F;
	default:
		return EF_ABGR8;
	}
}

SDL_GPUPrimitiveType SDL3Mapping::Mapping(RenderLayout::topology_type tt)
{
	switch (tt)
	{
	case RenderLayout::TT_PointList:		return SDL_GPU_PRIMITIVETYPE_POINTLIST;
	case RenderLayout::TT_LineList:			return SDL_GPU_PRIMITIVETYPE_LINELIST;
	case RenderLayout::TT_LineStrip:		return SDL_GPU_PRIMITIVETYPE_LINESTRIP;
	case RenderLayout::TT_TriangleList:		return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	case RenderLayout::TT_TriangleStrip:	return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
	default:
		return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	}
}

SDL_GPUCullMode SDL3Mapping::Mapping(CullMode mode)
{
	switch (mode)
	{
	case CM_None:	return SDL_GPU_CULLMODE_NONE;
	case CM_Front:	return SDL_GPU_CULLMODE_FRONT;
	case CM_Back:	return SDL_GPU_CULLMODE_BACK;
	default:		return SDL_GPU_CULLMODE_BACK;
	}
}

SDL_GPUFillMode SDL3Mapping::Mapping(PolygonMode mode)
{
	switch (mode)
	{
	case PM_Line:	return SDL_GPU_FILLMODE_LINE;
	case PM_Fill:	return SDL_GPU_FILLMODE_FILL;
	default:		return SDL_GPU_FILLMODE_FILL;
	}
}

SDL_GPUCompareOp SDL3Mapping::Mapping(CompareFunction func)
{
	switch (func)
	{
	case CF_AlwaysFail:		return SDL_GPU_COMPAREOP_NEVER;
	case CF_AlwaysPass:		return SDL_GPU_COMPAREOP_ALWAYS;
	case CF_Less:			return SDL_GPU_COMPAREOP_LESS;
	case CF_LessEqual:		return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
	case CF_Equal:			return SDL_GPU_COMPAREOP_EQUAL;
	case CF_NotEqual:		return SDL_GPU_COMPAREOP_NOT_EQUAL;
	case CF_GreaterEqual:	return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
	case CF_Greater:		return SDL_GPU_COMPAREOP_GREATER;
	default:				return SDL_GPU_COMPAREOP_LESS;
	}
}

SDL_GPUBlendFactor SDL3Mapping::Mapping(AlphaBlendFactor factor)
{
	switch (factor)
	{
	case ABF_Zero:				return SDL_GPU_BLENDFACTOR_ZERO;
	case ABF_One:				return SDL_GPU_BLENDFACTOR_ONE;
	case ABF_Src_Alpha:			return SDL_GPU_BLENDFACTOR_SRC_ALPHA;
	case ABF_Dst_Alpha:			return SDL_GPU_BLENDFACTOR_DST_ALPHA;
	case ABF_Inv_Src_Alpha:		return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	case ABF_Inv_Dst_Alpha:		return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
	case ABF_Src_Color:			return SDL_GPU_BLENDFACTOR_SRC_COLOR;
	case ABF_Dst_Color:			return SDL_GPU_BLENDFACTOR_DST_COLOR;
	case ABF_Inv_Src_Color:		return SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
	case ABF_Inv_Dst_Color:		return SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR;
	case ABF_Src_Alpha_Sat:		return SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE;
	case ABF_Blend_Factor:		return SDL_GPU_BLENDFACTOR_CONSTANT_COLOR;
	case ABF_Inv_Blend_Factor:	return SDL_GPU_BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR;
	default:					return SDL_GPU_BLENDFACTOR_ONE;
	}
}

SDL_GPUBlendOp SDL3Mapping::Mapping(BlendOperation op)
{
	switch (op)
	{
	case BOP_Add:		return SDL_GPU_BLENDOP_ADD;
	case BOP_Sub:		return SDL_GPU_BLENDOP_SUBTRACT;
	case BOP_Rev_Sub:	return SDL_GPU_BLENDOP_REVERSE_SUBTRACT;
	case BOP_Min:		return SDL_GPU_BLENDOP_MIN;
	case BOP_Max:		return SDL_GPU_BLENDOP_MAX;
	default:			return SDL_GPU_BLENDOP_ADD;
	}
}

} // namespace RenderWorker
