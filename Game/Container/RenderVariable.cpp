#include "RenderVariable.h"
#include "../System/ResLoader.h"
#include "../Tool/XMLDocument.h"
#include "../Util/UtilTool.h"
#include "Hash.h"
#include "DataBuffer.h"
#include "../Render/ITexture.h"
#include <string>
#include "../Container/C++17/filesystem.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#define STATIC_ASSERT(x) static_assert(x, #x)

ArrayRef<std::pair<char const *, size_t>> GetTypeDefines()
{
#define NAME_AND_HASH(name) std::make_pair(name, CT_HASH(name))
	static std::pair<char const *, size_t> const types[] =
	{
		NAME_AND_HASH("bool"),
		NAME_AND_HASH("string"),
		NAME_AND_HASH("texture1D"),
		NAME_AND_HASH("texture2D"),
		NAME_AND_HASH("texture2DMS"),
		NAME_AND_HASH("texture3D"),
		NAME_AND_HASH("textureCUBE"),
		NAME_AND_HASH("texture1DArray"),
		NAME_AND_HASH("texture2DArray"),
		NAME_AND_HASH("texture2DMSArray"),
		NAME_AND_HASH("texture3DArray"),
		NAME_AND_HASH("textureCUBEArray"),
		NAME_AND_HASH("sampler"),
		NAME_AND_HASH("shader"),
		NAME_AND_HASH("uint"),
		NAME_AND_HASH("uint2"),
		NAME_AND_HASH("uint3"),
		NAME_AND_HASH("uint4"),
		NAME_AND_HASH("int"),
		NAME_AND_HASH("int2"),
		NAME_AND_HASH("int3"),
		NAME_AND_HASH("int4"),
		NAME_AND_HASH("float"),
		NAME_AND_HASH("float2"),
		NAME_AND_HASH("float2x2"),
		NAME_AND_HASH("float2x3"),
		NAME_AND_HASH("float2x4"),
		NAME_AND_HASH("float3"),
		NAME_AND_HASH("float3x2"),
		NAME_AND_HASH("float3x3"),
		NAME_AND_HASH("float3x4"),
		NAME_AND_HASH("float4"),
		NAME_AND_HASH("float4x2"),
		NAME_AND_HASH("float4x3"),
		NAME_AND_HASH("float4x4"),
		NAME_AND_HASH("buffer"),
		NAME_AND_HASH("structured_buffer"),
		NAME_AND_HASH("byte_address_buffer"),
		NAME_AND_HASH("rw_buffer"),
		NAME_AND_HASH("rw_structured_buffer"),
		NAME_AND_HASH("rw_texture1D"),
		NAME_AND_HASH("rw_texture2D"),
		NAME_AND_HASH("rw_texture3D"),
		NAME_AND_HASH("rw_texture1DArray"),
		NAME_AND_HASH("rw_texture2DArray"),
		NAME_AND_HASH("rw_byte_address_buffer"),
		NAME_AND_HASH("append_structured_buffer"),
		NAME_AND_HASH("consume_structured_buffer"),
		NAME_AND_HASH("rasterizer_ordered_buffer"),
		NAME_AND_HASH("rasterizer_ordered_byte_address_buffer"),
		NAME_AND_HASH("rasterizer_ordered_structured_buffer"),
		NAME_AND_HASH("rasterizer_ordered_texture1D"),
		NAME_AND_HASH("rasterizer_ordered_texture1DArray"),
		NAME_AND_HASH("rasterizer_ordered_texture2D"),
		NAME_AND_HASH("rasterizer_ordered_texture2DArray"),
		NAME_AND_HASH("rasterizer_ordered_texture3D")
	};
#undef NAME_AND_HASH
	//STATIC_ASSERT(std::size(types) == CVAR_count);
	return types;
}

uint32_t GetFromNameType(std::string_view name)
{
	auto const types = GetTypeDefines();

	size_t const name_hash = HashRange(name.begin(), name.end());
	for (uint32_t i = 0; i < types.size(); ++i)
	{
		if (types[i].second == name_hash)
		{
			return i;
		}
	}

	UNREACHABLE_MSG("Invalid type name");
}

bool BoolFromStr(std::string_view name)
{
	if (("true" == name) || ("1" == name))
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::unique_ptr<RenderVariable> ReadVar(XMLNode const & node, uint32_t nType, uint32_t nArraySize)
{
	std::unique_ptr<RenderVariable> var;
	XMLAttributePtr attr;
	switch (nType)
	{
	case CVAR_bool:
		if (0 == nArraySize)
		{
			attr = node.Attrib("value");
			bool tmp = false;
			if (attr)
			{
				tmp = BoolFromStr(attr->ValueString());
			}

			var = MakeUniquePtr<RenderVariableBool>();
			*var = tmp;
		}
		break;

	case CVAR_uint:
		if (0 == nArraySize)
		{
			attr = node.Attrib("value");
			uint32_t tmp = 0;
			if (attr)
			{
				tmp = attr->ValueInt();
			}

			var = MakeUniquePtr<RenderVariableUInt>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableUIntArray>();
			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					auto _1 = boost::is_any_of(std::string(","));
					boost::algorithm::split(strs, value_str, _1);
					std::vector<uint32_t> init_val(std::min(nArraySize, static_cast<uint32_t>(strs.size())), 0);
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						if (index < strs.size())
						{
							boost::algorithm::trim(strs[index]);
							init_val[index] = std::stoul(strs[index]);
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_int:
		if (0 == nArraySize)
		{
			attr = node.Attrib("value");
			int32_t tmp = 0;
			if (attr)
			{
				tmp = attr->ValueInt();
			}

			var = MakeUniquePtr<RenderVariableInt>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableIntArray>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<int32_t> init_val(std::min(nArraySize, static_cast<uint32_t>(strs.size())), 0);
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						if (index < strs.size())
						{
							boost::algorithm::trim(strs[index]);
							init_val[index] = std::stol(strs[index]);
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_string:
	{
		attr = node.Attrib("value");
		std::string tmp;
		if (attr)
		{
			tmp = std::string(attr->ValueString());
		}

		var = MakeUniquePtr<RenderVariableString>();
		*var = tmp;
	}
	break;

	case CVAR_texture1D:
	case CVAR_texture2D:
	case CVAR_texture3D:
	case CVAR_textureCUBE:
	case CVAR_texture1DArray:
	case CVAR_texture2DArray:
	case CVAR_texture3DArray:
	case CVAR_textureCUBEArray:
	case CVAR_rw_texture1D:
	case CVAR_rw_texture2D:
	case CVAR_rw_texture3D:
	case CVAR_rw_texture1DArray:
	case CVAR_rw_texture2DArray:
	case CVAR_rasterizer_ordered_texture1D:
	case CVAR_rasterizer_ordered_texture1DArray:
	case CVAR_rasterizer_ordered_texture2D:
	case CVAR_rasterizer_ordered_texture2DArray:
	case CVAR_rasterizer_ordered_texture3D:
		var = MakeUniquePtr<VariableTexture>();
		*var = TexturePtr();
		attr = node.Attrib("elem_type");
		if (attr)
		{
			*var = std::string(attr->ValueString());
		}
		else
		{
			*var = std::string("float4");
		}
		break;

	case CVAR_texture2DMS:
	case CVAR_texture2DMSArray:
	{
			var = MakeUniquePtr<VariableTexture>();
			*var = TexturePtr();

			std::string elem_type;
			attr = node.Attrib("elem_type");
			if (attr)
			{
				elem_type = std::string(attr->ValueString());
			}
			else
			{
				elem_type = "float4";
			}

			std::string sample_count;
			attr = node.Attrib("sample_count");
			if (attr)
			{
				sample_count = std::string(attr->ValueString());
				*var = elem_type + ", " + sample_count;
			}
			else
			{
				*var = elem_type;
			}
	}
	break;

	case CVAR_sampler:
	{
	//	SamplerStateDesc desc;

	//	for (XMLNodePtr state_node = node.FirstNode("state"); state_node; state_node = state_node->NextSibling("state"))
	//	{
	//		std::string_view const name = state_node->Attrib("name")->ValueString();
	//		size_t const name_hash = HashRange(name.begin(), name.end());

	//		XMLAttributePtr const value_attr = state_node->Attrib("value");
	//		std::string_view value_str;
	//		if (value_attr)
	//		{
	//			value_str = value_attr->ValueString();
	//		}

	//		if (CT_HASH("filtering") == name_hash)
	//		{
	//			desc.filter = TexFilterOpFromName(value_str);
	//		}
	//		else if (CT_HASH("address_u") == name_hash)
	//		{
	//			desc.addr_mode_u = TexAddressingModeFromName(value_str);
	//		}
	//		else if (CT_HASH("address_v") == name_hash)
	//		{
	//			desc.addr_mode_v = TexAddressingModeFromName(value_str);
	//		}
	//		else if (CT_HASH("address_w") == name_hash)
	//		{
	//			desc.addr_mode_w = TexAddressingModeFromName(value_str);
	//		}
	//		else if (CT_HASH("max_anisotropy") == name_hash)
	//		{
	//			desc.max_anisotropy = static_cast<uint8_t>(value_attr->ValueUInt());
	//		}
	//		else if (CT_HASH("min_lod") == name_hash)
	//		{
	//			desc.min_lod = value_attr->ValueFloat();
	//		}
	//		else if (CT_HASH("max_lod") == name_hash)
	//		{
	//			desc.max_lod = value_attr->ValueFloat();
	//		}
	//		else if (CT_HASH("mip_map_lod_bias") == name_hash)
	//		{
	//			desc.mip_map_lod_bias = value_attr->ValueFloat();
	//		}
	//		else if (CT_HASH("cmp_func") == name_hash)
	//		{
	//			desc.cmp_func = CompareFunctionFromName(value_str);
	//		}
	//		else if (CT_HASH("border_clr") == name_hash)
	//		{
	//			attr = state_node->Attrib("r");
	//			if (attr)
	//			{
	//				desc.border_clr.r() = attr->ValueFloat();
	//			}
	//			attr = state_node->Attrib("g");
	//			if (attr)
	//			{
	//				desc.border_clr.g() = attr->ValueFloat();
	//			}
	//			attr = state_node->Attrib("b");
	//			if (attr)
	//			{
	//				desc.border_clr.b() = attr->ValueFloat();
	//			}
	//			attr = state_node->Attrib("a");
	//			if (attr)
	//			{
	//				desc.border_clr.a() = attr->ValueFloat();
	//			}
	//		}
	//		else
	//		{
	//			KFL_UNREACHABLE("Invalid sampler state name");
	//		}
	//	}

	//	var = MakeUniquePtr<VariableSampler>();
	//	*var = Context::Instance()->RenderFactoryInstance().MakeSamplerStateObject(desc);
	}
	break;

	case CVAR_shader:
	{
	//	ShaderDesc desc;
	//	desc.profile = get_profile(node);
	//	desc.func_name = get_func_name(node);

	//	var = MakeUniquePtr<VariableShader>();
	//	*var = desc;
	}
	break;

	case CVAR_float:
		if (0 == nArraySize)
		{
			float tmp = 0;
			attr = node.Attrib("value");
			if (attr)
			{
				tmp = attr->ValueFloat();
			}

			var = MakeUniquePtr<RenderVariableFloat>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableFloatArray>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<float> init_val(std::min(nArraySize, static_cast<uint32_t>(strs.size())), 0.0f);
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						if (index < strs.size())
						{
							boost::algorithm::trim(strs[index]);
							init_val[index] = std::stof(strs[index]);
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_uint2:
		if (0 == nArraySize)
		{
			uint2 tmp(0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueUInt();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueUInt();
			}

			var = MakeUniquePtr<RenderVariableUInt2>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableInt2Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<uint2> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 1) / 2)), uint2(0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 2; ++j)
						{
							if (index * 2 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 2 + j]);
								init_val[index][j] = std::stoul(strs[index * 2 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_uint3:
		if (0 == nArraySize)
		{
			uint3 tmp(0, 0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueUInt();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueUInt();
			}
			attr = node.Attrib("z");
			if (attr)
			{
				tmp.z() = attr->ValueUInt();
			}

			var = MakeUniquePtr<RenderVariableUInt3>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableInt3Array>();
			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<uint3> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 2) / 3)), uint3(0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 3; ++j)
						{
							if (index * 3 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 3 + j]);
								init_val[index][j] = std::stoul(strs[index * 3 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_uint4:
		if (0 == nArraySize)
		{
			uint4 tmp(0, 0, 0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueUInt();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueUInt();
			}
			attr = node.Attrib("z");
			if (attr)
			{
				tmp.z() = attr->ValueUInt();
			}
			attr = node.Attrib("w");
			if (attr)
			{
				tmp.w() = attr->ValueUInt();
			}

			var = MakeUniquePtr<RenderVariableUInt4>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableInt4Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<int4> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 3) / 4)), int4(0, 0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 4; ++j)
						{
							if (index * 4 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 4 + j]);
								init_val[index][j] = std::stoul(strs[index * 4 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_int2:
		if (0 == nArraySize)
		{
			int2 tmp(0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueInt();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueInt();
			}

			var = MakeUniquePtr<RenderVariableInt2>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableInt2Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<int2> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 1) / 2)), int2(0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 2; ++j)
						{
							if (index * 2 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 2 + j]);
								init_val[index][j] = std::stol(strs[index * 2 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_int3:
		if (0 == nArraySize)
		{
			int3 tmp(0, 0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueInt();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueInt();
			}
			attr = node.Attrib("z");
			if (attr)
			{
				tmp.z() = attr->ValueInt();
			}

			var = MakeUniquePtr<RenderVariableInt3>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableInt3Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<int3> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 2) / 3)), int3(0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 3; ++j)
						{
							if (index * 3 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 3 + j]);
								init_val[index][j] = std::stol(strs[index * 3 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_int4:
		if (0 == nArraySize)
		{
			int4 tmp(0, 0, 0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueInt();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueInt();
			}
			attr = node.Attrib("z");
			if (attr)
			{
				tmp.z() = attr->ValueInt();
			}
			attr = node.Attrib("w");
			if (attr)
			{
				tmp.w() = attr->ValueInt();
			}

			var = MakeUniquePtr<RenderVariableInt4>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableInt4Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<int4> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 3) / 4)), int4(0, 0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 4; ++j)
						{
							if (index * 4 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 4 + j]);
								init_val[index][j] = std::stol(strs[index * 4 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_float2:
		if (0 == nArraySize)
		{
			float2 tmp(0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueFloat();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueFloat();
			}

			var = MakeUniquePtr<RenderVariableFloat2>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableFloat2Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<float2> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 1) / 2)), float2(0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 2; ++j)
						{
							if (index * 2 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 2 + j]);
								init_val[index][j] = std::stof(strs[index * 2 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_float3:
		if (0 == nArraySize)
		{
			float3 tmp(0, 0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueFloat();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueFloat();
			}
			attr = node.Attrib("z");
			if (attr)
			{
				tmp.z() = attr->ValueFloat();
			}

			var = MakeUniquePtr<RenderVariableFloat3>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableFloat3Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<float3> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 2) / 3)), float3(0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 3; ++j)
						{
							if (index * 3 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 3 + j]);
								init_val[index][j] = std::stof(strs[index * 3 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_float4:
		if (0 == nArraySize)
		{
			float4 tmp(0, 0, 0, 0);
			attr = node.Attrib("x");
			if (attr)
			{
				tmp.x() = attr->ValueFloat();
			}
			attr = node.Attrib("y");
			if (attr)
			{
				tmp.y() = attr->ValueFloat();
			}
			attr = node.Attrib("z");
			if (attr)
			{
				tmp.z() = attr->ValueFloat();
			}
			attr = node.Attrib("w");
			if (attr)
			{
				tmp.w() = attr->ValueFloat();
			}

			var = MakeUniquePtr<RenderVariableFloat4>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableFloat4Array>();
			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<float4> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 3) / 4)), float4(0, 0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 4; ++j)
						{
							if (index * 4 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 4 + j]);
								init_val[index][j] = std::stof(strs[index * 4 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_float4x4:
		if (0 == nArraySize)
		{
			float4x4 tmp(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			for (int y = 0; y < 4; ++y)
			{
				for (int x = 0; x < 4; ++x)
				{
					attr = node.Attrib(std::string("_")
						+ static_cast<char>('0' + y) + static_cast<char>('0' + x));
					if (attr)
					{
						tmp[y * 4 + x] = attr->ValueFloat();
					}
				}
			}

			var = MakeUniquePtr<RenderVariableFloat4x4>();
			*var = tmp;
		}
		else
		{
			var = MakeUniquePtr<RenderVariableFloat4x4Array>();

			XMLNodePtr value_node = node.FirstNode("value");
			if (value_node)
			{
				value_node = value_node->FirstNode();
				if (value_node && (XNT_CData == value_node->Type()))
				{
					std::string_view const value_str = value_node->ValueString();
					std::vector<std::string> strs;
					boost::algorithm::split(strs, value_str, boost::is_any_of(","));
					std::vector<float4x4> init_val(std::min(nArraySize, static_cast<uint32_t>((strs.size() + 15) / 16)),
						float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
					for (size_t index = 0; index < init_val.size(); ++index)
					{
						for (size_t j = 0; j < 16; ++j)
						{
							if (index * 16 + j < strs.size())
							{
								boost::algorithm::trim(strs[index * 16 + j]);
								init_val[index][j] = std::stof(strs[index * 16 + j]);
							}
						}
					}
					*var = init_val;
				}
			}
		}
		break;

	case CVAR_buffer:
	case CVAR_structured_buffer:
	case CVAR_rw_buffer:
	case CVAR_rw_structured_buffer:
	case CVAR_consume_structured_buffer:
	case CVAR_append_structured_buffer:
	case CVAR_rasterizer_ordered_buffer:
	case CVAR_rasterizer_ordered_structured_buffer:
		//var = MakeUniquePtr<VariableBuffer>();
		//*var = GraphicsBufferPtr();
		//attr = node.Attrib("elem_type");
		//if (attr)
		//{
		//	*var = std::string(attr->ValueString());
		//}
		//else
		//{
		//	*var = std::string("float4");
		//}
		break;

	case CVAR_byte_address_buffer:
	case CVAR_rw_byte_address_buffer:
	case CVAR_rasterizer_ordered_byte_address_buffer:
		//var = MakeUniquePtr<VariableByteAddressBuffer>();
		//*var = GraphicsBufferPtr();
		break;

	default:
		UNREACHABLE_MSG("Invalid type");
	}

	return var;
}

class EffectLoadingDesc : public ResLoadingDesc
{
private:
	struct EffectDesc
	{
		std::vector<std::string> NameVec;
		RenderCVarlistPtr ptrCVList;
	};

public:
	explicit EffectLoadingDesc(ArrayRef<std::string> name)
	{
		m_DataDesc.NameVec = std::vector<std::string>(name.begin(), name.end());
	}

	uint64_t Type() const override
	{
		static uint64_t const type = CT_HASH("EffectLoadingDesc");
		return type;
	}

	bool StateLess() const override
	{
		return false;
	}

	void SubThreadStage() override
	{
	}

	void MainThreadStage() override
	{
		m_DataDesc.ptrCVList = MakeSharedPtr<RenderCVarlist>();
		m_DataDesc.ptrCVList->Load(m_DataDesc.NameVec);
	}

	bool HasSubThreadStage() const override
	{
		return false;
	}

	bool Match(ResLoadingDesc const & rhs) const override
	{
		if (this->Type() == rhs.Type())
		{
			EffectLoadingDesc const & eld = static_cast<EffectLoadingDesc const &>(rhs);
			return (m_DataDesc.NameVec == eld.m_DataDesc.NameVec);
		}
		return false;
	}

	void CopyDataFrom(ResLoadingDesc const & rhs) override
	{
		BOOST_ASSERT(this->Type() == rhs.Type());

		EffectLoadingDesc const & eld = static_cast<EffectLoadingDesc const &>(rhs);
		m_DataDesc.NameVec = eld.m_DataDesc.NameVec;
	}

	std::shared_ptr<void> CloneResourceFrom(std::shared_ptr<void> const & resource) override
	{
		m_DataDesc.ptrCVList = std::static_pointer_cast<RenderCVarlist>(resource)->Clone();
		return std::static_pointer_cast<void>(m_DataDesc.ptrCVList);
	}

	std::shared_ptr<void> Resource() const override
	{
		return m_DataDesc.ptrCVList;
	}

private:
	EffectDesc m_DataDesc;
};

RenderVariable::RenderVariable()
{
}

RenderVariable::~RenderVariable()
{
}

RenderVariable& RenderVariable::operator=(const bool &)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const uint32_t&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const int32_t &)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const float&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const uint2&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const uint3&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const uint4&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const int2&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const int3&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const int4&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const float2&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const float3&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const float4&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const float4x4&)
{
	UNREACHABLE_MSG("Can't be called");
}

//Variable& Variable::operator=(SamplerStateObjectPtr const &)
//{
//	UNREACHABLE_MSG("Can't be called");
//}
//
//Variable& Variable::operator=(GraphicsBufferPtr const &)
//{
//	UNREACHABLE_MSG("Can't be called");
//}

RenderVariable& RenderVariable::operator=(const std::string &)
{
	UNREACHABLE_MSG("Can't be called");
}

//Variable& Variable::operator=(ShaderDesc const &)
//{
//	UNREACHABLE_MSG("Can't be called");
//}

RenderVariable& RenderVariable::operator=(const std::vector<bool>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<uint32_t>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<int32_t>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<float>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<uint2>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<uint3>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<uint4>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<int2>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<int3>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<int4>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<float2>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<float3>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<float4>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::vector<float4x4>&)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const std::wstring& value)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const int64_t& value)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const PERSISTID& value)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const TextureSubresource& value)
{
	UNREACHABLE_MSG("Can't be called");
}

RenderVariable& RenderVariable::operator=(const TexturePtr& value)
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(bool&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(uint32_t&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(int32_t&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(float&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(uint2&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(uint3&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(uint4&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(int2&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(int3&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(int4&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(float2&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(float3&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(float4&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(float4x4&) const
{
	UNREACHABLE_MSG("Can't be called");
}

//void Variable::Value(TexturePtr&) const
//{
//	UNREACHABLE_MSG("Can't be called");
//}

//void Variable::Value(TextureSubresource&) const
//{
//	UNREACHABLE_MSG("Can't be called");
//}

//void Variable::Value(SamplerStateObjectPtr&) const
//{
//	UNREACHABLE_MSG("Can't be called");
//}
//
//void Variable::Value(GraphicsBufferPtr&) const
//{
//	UNREACHABLE_MSG("Can't be called");
//}

void RenderVariable::Value(std::string&) const
{
	UNREACHABLE_MSG("Can't be called");
}

//void Variable::Value(ShaderDesc&) const
//{
//	UNREACHABLE_MSG("Can't be called");
//}

void RenderVariable::Value(std::vector<bool>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<uint32_t>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<int32_t>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<float>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<uint2>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<uint3>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<uint4>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<int2>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<int3>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<int4>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<float2>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<float3>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<float4>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::vector<float4x4>&) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(std::wstring& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(int64_t& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(PERSISTID& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(TexturePtr& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::Value(TextureSubresource& val) const
{
	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::BindToCBuffer(RenderConstantBuffer& cbuff, uint32_t offset, uint32_t stride)
{
	UNUSED(cbuff);
	UNUSED(offset);
	UNUSED(stride);

	UNREACHABLE_MSG("Can't be called");
}

void RenderVariable::RebindToCBuffer(RenderConstantBuffer& cbuff)
{
	UNUSED(cbuff);

	UNREACHABLE_MSG("Can't be called");
}

RenderVariableFloat4x4::RenderVariableFloat4x4(bool in_cbuff)
	: RenderVariableConcrete<float4x4>(in_cbuff)
{
}

RenderVariableFloat4x4::RenderVariableFloat4x4()
	: RenderVariableConcrete<float4x4>()
{
}

std::unique_ptr<RenderVariable> RenderVariableFloat4x4::Clone()
{
	auto ret = MakeUniquePtr<RenderVariableFloat4x4>(m_bIsBuff);
	if (m_bIsBuff)
	{
		ret->m_Data = m_Data;

		float4x4 val;
		RenderVariableConcrete<float4x4>::Value(val);
		RenderVariableConcrete<float4x4>::operator=(val);
	}
	else
	{
		ret->RetriveT() = this->RetriveT();
	}
	return std::move(ret);
}

void RenderVariableFloat4x4::Value(float4x4& val) const
{
	RenderVariableConcrete<float4x4>::Value(val);
	val = MathLib::Transpose(val);
}

RenderVariable& RenderVariableFloat4x4::operator=(float4x4 const & value)
{
	return RenderVariableConcrete<float4x4>::operator=(MathLib::Transpose(value));
}

void RenderConstantBuffer::Load(std::string const & name)
{
	m_HashName = MakeSharedPtr<std::remove_reference<decltype(*m_HashName)>::type>();
	m_HashName->first = name;
	m_HashName->second = HashRange(m_HashName->first.begin(), m_HashName->first.end());
	m_ParamIndexVec = MakeSharedPtr<std::remove_reference<decltype(*m_ParamIndexVec)>::type>();
}

std::unique_ptr<RenderConstantBuffer> RenderConstantBuffer::Clone(RenderCVarlist& src_list, RenderCVarlist& dst_list)
{
	auto ret = MakeUniquePtr<RenderConstantBuffer>();
	ret->m_HashName = m_HashName;
	ret->m_ParamIndexVec = m_ParamIndexVec;
	ret->m_buff = m_buff;
	ret->Resize(static_cast<uint32_t>(m_buff.size()));

	for (size_t i = 0; i < m_ParamIndexVec->size(); ++i)
	{
		RenderCVarParameter* src_param = src_list.QueryByIndex((*m_ParamIndexVec)[i]);
		if (src_param->InCBuffer())
		{
			RenderCVarParameter* dst_param = dst_list.QueryByIndex((*m_ParamIndexVec)[i]);
			dst_param->RebindToCBuffer(*ret);
		}
	}

	return ret;
}

void RenderConstantBuffer::AddParameter(uint32_t index)
{
	m_ParamIndexVec->push_back(index);
}

void RenderConstantBuffer::Resize(uint32_t size)
{
	m_buff.resize(size);
	if (size > 0)
	{
		if (!m_DataBuffPtr->IsNull() || (size > m_DataBuffPtr->GetSize()))
		{
			m_DataBuffPtr->Create(size);
		}
	}

	m_bDirty = true;
}

void RenderConstantBuffer::Update()
{
	//
}

void RenderConstantBuffer::BindHWBuff(const DataBufferPtr& buff)
{
	m_DataBuffPtr = buff;
	m_buff.resize(m_DataBuffPtr->GetSize());
}

RenderVariableFloat4x4Array::RenderVariableFloat4x4Array(bool in_cbuff)
	: RenderVariableConcrete<std::vector<float4x4>>(in_cbuff)
{

}

RenderVariableFloat4x4Array::RenderVariableFloat4x4Array()
	: RenderVariableConcrete<std::vector<float4x4>>()
{

}

std::unique_ptr<RenderVariable> RenderVariableFloat4x4Array::Clone()
{
	auto ret = MakeUniquePtr<RenderVariableFloat4x4Array>(m_bIsBuff);
	if (m_bIsBuff)
	{
		ret->m_Data = m_Data;
		ret->m_Size = m_Size;

		auto const & src_cbuff_desc = this->RetriveCBufferDesc();
		uint8_t const * src = src_cbuff_desc.pBuff->VariableInBuff<uint8_t>(src_cbuff_desc.nOffset);

		auto const & dst_cbuff_desc = ret->RetriveCBufferDesc();
		uint8_t* dst = dst_cbuff_desc.pBuff->VariableInBuff<uint8_t>(dst_cbuff_desc.nOffset);

		memcpy(dst, src, m_Size * sizeof(float4x4));

		dst_cbuff_desc.pBuff->Dirty(true);
	}
	else
	{
		ret->RetriveT() = this->RetriveT();
	}
	return std::move(ret);
}

void RenderVariableFloat4x4Array::Value(std::vector<float4x4>& val) const
{
	if (m_bIsBuff)
	{
		auto const & cbuff_desc = this->RetriveCBufferDesc();
		float4x4 const * src = cbuff_desc.pBuff->VariableInBuff<float4x4>(cbuff_desc.nOffset);

		val.resize(m_Size);
		float4x4* dst = val.data();
		for (size_t i = 0; i < m_Size; ++i)
		{
			*dst = MathLib::Transpose(*src);
			++src;
			++dst;
		}
	}
	else
	{
		val = this->RetriveT();
	}
}

RenderVariable& RenderVariableFloat4x4Array::operator=(const std::vector<float4x4>& value)
{
	if (m_bIsBuff)
	{
		float4x4 const * src = value.data();
		auto& cbuff_desc = this->RetriveCBufferDesc();
		float4x4* dst = cbuff_desc.pBuff->VariableInBuff<float4x4>(cbuff_desc.nOffset);

		m_Size = static_cast<uint32_t>(value.size());
		for (size_t i = 0; i < value.size(); ++i)
		{
			*dst = MathLib::Transpose(*src);
			++src;
			++dst;
		}

		cbuff_desc.pBuff->Dirty(true);
	}
	else
	{
		this->RetriveT() = value;
	}
	return *this;
}

std::unique_ptr<RenderVariable> VariableTexture::Clone()
{
	auto ret = MakeUniquePtr<VariableTexture>();
	TexturePtr val;
	this->Value(val);
	*ret = val;
	std::string elem_type;
	this->Value(elem_type);
	*ret = elem_type;
	return std::move(ret);
}

void VariableTexture::Value(TexturePtr& val) const
{
	if (val_.tex)
	{
		val_.num_items = val_.tex->ArraySize();
		val_.num_levels = val_.tex->NumMipMaps();
	}
	val = val_.tex;
}

void VariableTexture::Value(TextureSubresource& val) const
{
	if (val_.tex)
	{
		val_.num_items = val_.tex->ArraySize();
		val_.num_levels = val_.tex->NumMipMaps();
	}
	val = val_;
}

void VariableTexture::Value(std::string& val) const
{
	val = elem_type_;
}

RenderVariable& VariableTexture::operator=(const std::string  & value)
{
	elem_type_ = value;
	return *this;
}

RenderVariable& VariableTexture::operator=(const TextureSubresource & value)
{
	val_ = value;
	return *this;
}

RenderVariable& VariableTexture::operator=(TexturePtr const & value)
{
	uint32_t array_size = 1;
	uint32_t mipmap = 1;
	if (value)
	{
		array_size = value->ArraySize();
		mipmap = value->NumMipMaps();
	}
	val_ = TextureSubresource(value, 0, array_size, 0, mipmap);
	return *this;
}

std::unique_ptr<RenderVariable> RenderVariableByteAddressBuffer::Clone()
{
	auto ret = MakeUniquePtr<RenderVariableByteAddressBuffer>();
	DataBufferPtr val;
	this->Value(val);
	*ret = val;
	std::string elem_type;
	this->Value(elem_type);
	*ret = elem_type;
	return std::move(ret);
}

void RenderVariableByteAddressBuffer::Value(DataBufferPtr& val) const
{
	val = m_Val;
}

void RenderVariableByteAddressBuffer::Value(std::string& val) const
{
	val = m_Type;
}

RenderVariable& RenderVariableByteAddressBuffer::operator=(std::string const & value)
{
	m_Type = value;
	return *this;
}

RenderVariable& RenderVariableByteAddressBuffer::operator=(DataBufferPtr const & value)
{
	m_Val = value;
	return *this;
}

void RenderVariableAnnotation::Load(XMLNodePtr const & node)
{
	m_nType = GetFromNameType(node->Attrib("type")->ValueString());
	m_strName = std::string(node->Attrib("name")->ValueString());
	m_ImageData = ReadVar(*node, m_nType, 0);
}

void RenderVariableAnnotation::StreamIn(ResIdentifierPtr const & res)
{

}

void RenderVariableAnnotation::StreamOut(std::ostream& os) const
{

}

void RenderCVarParameter::Load(XMLNodePtr const & node)
{
	m_Type = GetFromNameType(node->Attrib("type")->ValueString());
	m_NameHash = MakeSharedPtr<std::remove_reference<decltype(*m_NameHash)>::type>();
	m_NameHash->first = std::string(node->Attrib("name")->ValueString());
	m_NameHash->second = HashRange(m_NameHash->first.begin(), m_NameHash->first.end());

	XMLAttributePtr attr = node->Attrib("semantic");
	if (attr)
	{
		m_SemanticHash = MakeSharedPtr<std::remove_reference<decltype(*m_SemanticHash)>::type>();
		m_SemanticHash->first = std::string(attr->ValueString());
		m_SemanticHash->second = HashRange(m_SemanticHash->first.begin(), m_SemanticHash->first.end());
	}

	uint32_t as;
	attr = node->Attrib("array_size");
	if (attr)
	{
		m_ArraySize = MakeSharedPtr<std::string>(attr->ValueString());

		if (!attr->TryConvert(as))
		{
			as = 1;  // dummy array size
		}
	}
	else
	{
		as = 0;
	}
	m_Var = ReadVar(*node, m_Type, as);

	{
		XMLNodePtr anno_node = node->FirstNode("annotation");
		if (anno_node)
		{
			m_ImageListData = MakeSharedPtr<std::remove_reference<decltype(*m_ImageListData)>::type>();
			for (; anno_node; anno_node = anno_node->NextSibling("annotation"))
			{
				m_ImageListData->push_back(MakeUniquePtr<RenderVariableAnnotation>());
				m_ImageListData->back()->Load(anno_node);
			}
		}
	}

	if (m_ImageListData && ((CVAR_texture1D == m_Type) || (CVAR_texture2D == m_Type) || (CVAR_texture2DMS == m_Type)
		|| (CVAR_texture3D == m_Type) || (CVAR_textureCUBE == m_Type)
		|| (CVAR_texture1DArray == m_Type) || (CVAR_texture2DArray == m_Type) || (CVAR_texture2DMSArray == m_Type)
		|| (CVAR_texture3DArray == m_Type) || (CVAR_textureCUBEArray == m_Type)))
	{
		for (size_t i = 0; i < m_ImageListData->size(); ++i)
		{
			if (CVAR_string == (*m_ImageListData)[i]->Type())
			{
				if ("SasResourceAddress" == (*m_ImageListData)[i]->Name())
				{
					std::string val;
					(*m_ImageListData)[i]->Value(val);
					if (ResLoader::Instance()->Locate(val).empty())
					{
						//LogError() << val << " NOT found" << std::endl;
					}
					else
					{
						*m_Var = SyncLoadTexture(val, 0);
					}
				}
			}
		}
	}
}

std::unique_ptr<RenderCVarParameter> RenderCVarParameter::Clone()
{
	std::unique_ptr<RenderCVarParameter> ret = MakeUniquePtr<RenderCVarParameter>();

	ret->m_NameHash = m_NameHash;
	ret->m_SemanticHash = m_SemanticHash;
	ret->m_Type = m_Type;
	ret->m_Var = m_Var->Clone();
	ret->m_ArraySize = m_ArraySize;
	ret->m_ImageListData = m_ImageListData;

	return ret;
}

std::string const & RenderCVarParameter::Semantic() const
{
	if (this->HasSemantic())
	{
		return m_SemanticHash->first;
	}
	else
	{
		static std::string empty("");
		return empty;
	}
}

size_t RenderCVarParameter::SemanticHash() const
{
	return this->HasSemantic() ? m_SemanticHash->second : 0;
}

uint32_t RenderCVarParameter::NumAnnotations() const
{
	return m_ImageListData ? static_cast<uint32_t>(m_ImageListData->size()) : 0;
}

RenderVariableAnnotation const & RenderCVarParameter::Annotation(uint32_t n) const
{
	BOOST_ASSERT(n < this->NumAnnotations());
	return *(*m_ImageListData)[n];
}

void RenderCVarParameter::BindToCBuffer(RenderConstantBuffer& cbuff, uint32_t offset, uint32_t stride)
{
	m_pBuff = &cbuff;
	m_Var->BindToCBuffer(cbuff, offset, stride);
}

void RenderCVarParameter::RebindToCBuffer(RenderConstantBuffer& cbuff)
{
	m_pBuff = &cbuff;
	m_Var->RebindToCBuffer(cbuff);
}

void RenderCVarlist::Load(ArrayRef<std::string> names)
{
	m_DataTemplate = MakeSharedPtr<RenderCVarTemplate>();
	m_DataTemplate->Load(names, *this);
}

RenderCVarlistPtr RenderCVarlist::Clone()
{
	RenderCVarlistPtr ret = MakeSharedPtr<RenderCVarlist>();
	ret->m_DataTemplate = m_DataTemplate;
	ret->m_ParamVar.resize(m_ParamVar.size());
	for (size_t i = 0; i < m_ParamVar.size(); ++i)
	{
		ret->m_ParamVar[i] = m_ParamVar[i]->Clone();
	}

	ret->m_Buffer.resize(m_Buffer.size());
	for (size_t i = 0; i < m_Buffer.size(); ++i)
	{
		ret->m_Buffer[i] = m_Buffer[i]->Clone(*this, *ret);
	}

	return ret;
}

std::string const & RenderCVarlist::ResName() const
{
	return m_DataTemplate->ResName();
}

size_t RenderCVarlist::ResNameHash() const
{
	return m_DataTemplate->ResNameHash();
}

uint32_t RenderCVarlist::NumParameters() const
{
	return static_cast<uint32_t>(m_ParamVar.size());
}

RenderCVarParameter* RenderCVarlist::QueryBySemantic(std::string_view semantic) const
{
	size_t const semantic_hash = HashRange(semantic.begin(), semantic.end());
	for (auto const & param : m_ParamVar)
	{
		if (semantic_hash == param->SemanticHash())
		{
			return param.get();
		}
	}
	return nullptr;
}

RenderCVarParameter* RenderCVarlist::QueryByName(std::string_view name) const
{
	size_t const name_hash = HashRange(name.begin(), name.end());
	for (auto const & param : m_ParamVar)
	{
		if (name_hash == param->NameHash())
		{
			return param.get();
		}
	}
	return nullptr;
}

RenderCVarParameter* RenderCVarlist::QueryByIndex(uint32_t n) const
{
	BOOST_ASSERT(n < this->NumParameters());
	return m_ParamVar[n].get();
}

uint32_t RenderCVarlist::NumCBuffers() const
{
	return static_cast<uint32_t>(m_Buffer.size());
}

RenderConstantBuffer* RenderCVarlist::CBufferByName(std::string_view name) const
{
	size_t const name_hash = HashRange(name.begin(), name.end());
	for (auto const & cbuffer : m_Buffer)
	{
		if (name_hash == cbuffer->NameHash())
		{
			return cbuffer.get();
		}
	}
	return nullptr;
}

RenderConstantBuffer* RenderCVarlist::CBufferByIndex(uint32_t n) const
{
	BOOST_ASSERT(n < this->NumCBuffers());
	return m_Buffer[n].get();
}

uint32_t RenderCVarlist::NumMacros() const
{
	return m_DataTemplate->NumMacros();
}

std::pair<std::string, std::string> const & RenderCVarlist::MacroByIndex(uint32_t n) const
{
	return m_DataTemplate->MacroByIndex(n);
}

void RenderCVarTemplate::PreprocessIncludes(XMLDocument& doc, XMLNode& root, std::vector<std::unique_ptr<XMLDocument>>& include_docs)
{

}

void RenderCVarTemplate::RecursiveIncludeNode(XMLNode const & root, std::vector<std::string>& FileNames) const
{
	for (XMLNodePtr node = root.FirstNode("include"); node; node = node->NextSibling("include"))
	{
		XMLAttributePtr attr = node->Attrib("name");
		BOOST_ASSERT(attr);

		std::string strFileName = std::string(attr->ValueString());

		auto source = ResLoader::Instance()->Open(strFileName);
		if (nullptr != source)
		{
			XMLDocument include_doc;
			XMLNodePtr include_root = include_doc.Parse(source);
			this->RecursiveIncludeNode(*include_root, FileNames);
		}

		bool found = false;
		for (size_t i = 0; i < FileNames.size(); ++i)
		{
			if (strFileName == FileNames[i])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			FileNames.push_back(strFileName);
		}
	}
}

void RenderCVarTemplate::InsertIncludeNodes(XMLDocument& target_doc, XMLNode& target_root, XMLNodePtr const & target_place, XMLNode const & include_root) const
{
	for (XMLNodePtr child_node = include_root.FirstNode(); child_node; child_node = child_node->NextSibling())
	{
		if ((XNT_Element == child_node->Type()) && (child_node->Name() != "include"))
		{
			target_root.InsertNode(target_place, target_doc.CloneNode(child_node));
		}
	}
}

XMLNodePtr RenderCVarTemplate::ResolveInheritTechNode(XMLDocument& doc, XMLNode& root, XMLNodePtr const & tech_node)
{
	auto inherit_attr = tech_node->Attrib("inherit");
	if (!inherit_attr)
	{
		return tech_node;
	}

	auto const tech_name = tech_node->Attrib("name")->ValueString();

	auto const inherit_name = inherit_attr->ValueString();
	BOOST_ASSERT(inherit_name != tech_name);

	XMLNodePtr new_tech_node;
	for (auto node = root.FirstNode("technique"); node; node = node->NextSibling("technique"))
	{
		auto const parent_tech_name = node->Attrib("name")->ValueString();
		if (parent_tech_name == inherit_name)
		{
			auto parent_node = this->ResolveInheritTechNode(doc, root, node);
			new_tech_node = doc.CloneNode(parent_node);

			for (auto tech_anno_node = tech_node->FirstNode("annotation"); tech_anno_node;
				tech_anno_node = tech_anno_node->NextSibling("annotation"))
			{
				new_tech_node->AppendNode(doc.CloneNode(tech_anno_node));
			}
			for (auto tech_macro_node = tech_node->FirstNode("macro"); tech_macro_node;
				tech_macro_node = tech_macro_node->NextSibling("macro"))
			{
				new_tech_node->AppendNode(doc.CloneNode(tech_macro_node));
			}
			for (auto pass_node = tech_node->FirstNode("pass"); pass_node; pass_node = pass_node->NextSibling("pass"))
			{
				auto const pass_name = pass_node->Attrib("name")->ValueString();

				bool found_pass = false;
				for (auto new_pass_node = new_tech_node->FirstNode("pass"); new_pass_node;
					new_pass_node = new_pass_node->NextSibling("pass"))
				{
					auto const parent_pass_name = new_pass_node->Attrib("name")->ValueString();

					if (pass_name == parent_pass_name)
					{
						for (auto pass_anno_node = pass_node->FirstNode("annotation"); pass_anno_node;
							pass_anno_node = pass_anno_node->NextSibling("annotation"))
						{
							new_pass_node->AppendNode(doc.CloneNode(pass_anno_node));
						}
						for (auto pass_macro_node = pass_node->FirstNode("macro"); pass_macro_node;
							pass_macro_node = pass_macro_node->NextSibling("macro"))
						{
							new_pass_node->AppendNode(doc.CloneNode(pass_macro_node));
						}
						for (auto pass_state_node = pass_node->FirstNode("state"); pass_state_node;
							pass_state_node = pass_state_node->NextSibling("state"))
						{
							new_pass_node->AppendNode(doc.CloneNode(pass_state_node));
						}

						found_pass = true;
						break;
					}
				}

				if (!found_pass)
				{
					new_tech_node->AppendNode(doc.CloneNode(pass_node));
				}
			}

			new_tech_node->RemoveAttrib(new_tech_node->Attrib("name"));
			new_tech_node->AppendAttrib(doc.AllocAttribString("name", tech_name));

			break;
		}
	}
	BOOST_ASSERT(new_tech_node);

	return new_tech_node;
}

void RenderCVarTemplate::ResolveOverrideTechs(XMLDocument& doc, XMLNode& root)
{
	std::vector<XMLNodePtr> tech_nodes;
	for (XMLNodePtr node = root.FirstNode("technique"); node; node = node->NextSibling("technique"))
	{
		tech_nodes.push_back(node);
	}

	for (auto const & node : tech_nodes)
	{
		auto override_attr = node->Attrib("override");
		if (override_attr)
		{
			auto override_tech_name = override_attr->ValueString();
			for (auto& overrided_node : tech_nodes)
			{
				auto name = overrided_node->Attrib("name")->ValueString();
				if (override_tech_name == name)
				{
					auto new_node = doc.CloneNode(this->ResolveInheritTechNode(doc, root, node));
					new_node->RemoveAttrib(new_node->Attrib("name"));
					new_node->AppendAttrib(doc.AllocAttribString("name", name));
					auto attr = new_node->Attrib("override");
					if (attr)
					{
						new_node->RemoveAttrib(attr);
					}

					root.InsertNode(overrided_node, new_node);
					root.RemoveNode(overrided_node);
					overrided_node = new_node;

					break;
				}
			}
		}
	}
}

void RenderCVarTemplate::Load(ArrayRef<std::string> names, RenderCVarlist& cvlist)
{
	std::filesystem::path last_fxml_path(names.back());
	std::filesystem::path last_fxml_directory = last_fxml_path.parent_path();
	std::string connected_name;
	for (size_t i = 0; i < names.size(); ++i)
	{
		connected_name += std::filesystem::path(names[i]).stem().string();
		if (i != names.size() - 1)
		{
			connected_name += '+';
		}
	}

	m_strNameRes = (last_fxml_directory / (connected_name + ".fxml")).string();
	m_NameHash = HashRange(m_strNameRes.begin(), m_strNameRes.end());
	for (auto const & name : names)
	{
		m_nTimeMap = 0;
		ResIdentifierPtr source = ResLoader::Instance()->Open(name);
		if (source)
		{
			m_nTimeMap = std::max(m_nTimeMap, source->Timestamp());
			std::unique_ptr<XMLDocument> doc = MakeUniquePtr<XMLDocument>();

			XMLNodePtr root = doc->Parse(source);
			std::vector<std::string> FileNames;
			this->RecursiveIncludeNode(*root, FileNames);
			for (auto const & strFileName : FileNames)
			{
				ResIdentifierPtr include_source = ResLoader::Instance()->Open(strFileName);
				if (include_source)
				{
					m_nTimeMap = std::max(m_nTimeMap, include_source->Timestamp());
				}
			}
		}
	}

	std::vector<std::unique_ptr<XMLDocument>> include_docs;
	std::vector<std::unique_ptr<XMLDocument>> frag_docs(names.size());
	ResIdentifierPtr main_source = ResLoader::Instance()->Open(names[0]);
	if (main_source)
	{
		frag_docs[0] = MakeUniquePtr<XMLDocument>();
		XMLNodePtr root = frag_docs[0]->Parse(main_source);
		//this->ResolveOverrideTechs(*frag_docs[0], *root);
		this->Load(*root, cvlist);
	}
}

void RenderCVarTemplate::Load(XMLNode const & root, RenderCVarlist& cvlist)
{
	{
		XMLNodePtr macro_node = root.FirstNode("macro");
		for (; macro_node; macro_node = macro_node->NextSibling("macro"))
		{
			m_Typs.emplace_back(std::make_pair(macro_node->Attrib("name")->ValueString(), macro_node->Attrib("value")->ValueString()), true);
		}
	}

	std::vector<XMLNodePtr> parameter_nodes;
	for (XMLNodePtr node = root.FirstNode(); node; node = node->NextSibling())
	{
		if ("parameter" == node->Name())
		{
			parameter_nodes.push_back(node);
		}
		else if ("cbuffer" == node->Name())
		{
			for (XMLNodePtr sub_node = node->FirstNode("parameter"); sub_node; sub_node = sub_node->NextSibling("parameter"))
			{
				parameter_nodes.push_back(sub_node);
			}
		}
	}

	for (uint32_t param_index = 0; param_index < parameter_nodes.size(); ++param_index)
	{
		XMLNodePtr const & node = parameter_nodes[param_index];
		uint32_t type = GetFromNameType(node->Attrib("type")->ValueString());
		if ((type != CVAR_sampler)
			&& (type != CVAR_texture1D) && (type != CVAR_texture2D) && (type != CVAR_texture2DMS) && (type != CVAR_texture3D)
			&& (type != CVAR_textureCUBE)
			&& (type != CVAR_texture1DArray) && (type != CVAR_texture2DArray) && (type != CVAR_texture2DMSArray)
			&& (type != CVAR_texture3DArray) && (type != CVAR_textureCUBEArray)
			&& (type != CVAR_buffer) && (type != CVAR_structured_buffer)
			&& (type != CVAR_byte_address_buffer) && (type != CVAR_rw_buffer)
			&& (type != CVAR_rw_structured_buffer) && (type != CVAR_rw_texture1D)
			&& (type != CVAR_rw_texture2D) && (type != CVAR_rw_texture3D)
			&& (type != CVAR_rw_texture1DArray) && (type != CVAR_rw_texture2DArray)
			&& (type != CVAR_rw_byte_address_buffer) && (type != CVAR_append_structured_buffer)
			&& (type != CVAR_consume_structured_buffer)
			&& (type != CVAR_rasterizer_ordered_buffer) && (type != CVAR_rasterizer_ordered_byte_address_buffer)
			&& (type != CVAR_rasterizer_ordered_structured_buffer)
			&& (type != CVAR_rasterizer_ordered_texture1D) && (type != CVAR_rasterizer_ordered_texture1DArray)
			&& (type != CVAR_rasterizer_ordered_texture2D) && (type != CVAR_rasterizer_ordered_texture2DArray)
			&& (type != CVAR_rasterizer_ordered_texture3D))
		{
			RenderConstantBuffer* cbuff = nullptr;
			XMLNodePtr parent_node = node->Parent();
			std::string const cbuff_name = std::string(parent_node->AttribString("name", "global_cb"));
			size_t const cbuff_name_hash = RT_HASH(cbuff_name.c_str());

			bool found = false;
			for (size_t i = 0; i < cvlist.m_Buffer.size(); ++i)
			{
				if (cvlist.m_Buffer[i]->NameHash() == cbuff_name_hash)
				{
					cbuff = cvlist.m_Buffer[i].get();
					found = true;
					break;
				}
			}
			if (!found)
			{
				cvlist.m_Buffer.push_back(MakeUniquePtr<RenderConstantBuffer>());
				cbuff = cvlist.m_Buffer.back().get();
				cbuff->Load(cbuff_name);
			}
			BOOST_ASSERT(cbuff);

			cbuff->AddParameter(param_index);
		}

		cvlist.m_ParamVar.push_back(MakeUniquePtr<RenderCVarParameter>());
		cvlist.m_ParamVar.back()->Load(node);
	}
}

bool RenderCVarTemplate::StreamIn(ResIdentifierPtr const & source, RenderCVarlist& effect)
{
	return true;
}

std::pair<std::string, std::string> const & RenderCVarTemplate::MacroByIndex(uint32_t n) const
{
	BOOST_ASSERT(n < this->NumMacros());
	return m_Typs[n].first;
}


RenderCVarlistPtr SyncLoadRenderEffect(std::string const & file_name)
{
	return ResLoader::Instance()->SyncQueryT<RenderCVarlist>(MakeSharedPtr<EffectLoadingDesc>(file_name));
}

RenderCVarlistPtr SyncLoadRenderEffects(ArrayRef<std::string> file_names)
{
	return ResLoader::Instance()->SyncQueryT<RenderCVarlist>(MakeSharedPtr<EffectLoadingDesc>(file_names));
}

RenderCVarlistPtr ASyncLoadRenderEffect(std::string const & file_name)
{
	return ResLoader::Instance()->SyncQueryT<RenderCVarlist>(MakeSharedPtr<EffectLoadingDesc>(file_name));
}

RenderCVarlistPtr ASyncLoadRenderEffects(ArrayRef<std::string> file_names)
{
	return ResLoader::Instance()->SyncQueryT<RenderCVarlist>(MakeSharedPtr<EffectLoadingDesc>(file_names));
}


