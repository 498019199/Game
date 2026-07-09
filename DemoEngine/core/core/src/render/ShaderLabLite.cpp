#include <render/ShaderLabLite.h>

#include <common/ResIdentifier.h>
#include <common/Util.h>
#include <common/XMLDom.h>
#include <base/ZEngine.h>

#include <algorithm>
#include <cctype>
#include <istream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace RenderWorker
{
using namespace CommonWorker;

namespace
{
struct PassState
{
	std::string name;
	std::string value;
	int index = -1;
};

struct PropDesc
{
	std::string name;
	std::string display_name;
	std::string type; // Color, Float, Range, Vector, 2D, Int, Float2/3/4, Matrix, Sampler, ...
	std::string default_value;
	float range_min = 0.f;
	float range_max = 1.f;
	std::string cbuffer_name; // empty → top-level / global_cb; set by CBuffer block
	std::string semantic;
	std::string array_size;
	std::string custom_type; // struct type name when type is StructRef
	std::vector<PassState> sampler_states;
	float xyzw[4] = {0, 0, 0, 0};
	bool has_xyzw = false;
	bool has_value = false;
	float value_f = 0.f;
};

struct StructMemberDesc
{
	std::string type;
	std::string name;
	std::string array_size;
};

struct StructDesc
{
	std::string name;
	std::vector<StructMemberDesc> members;
};

struct PassDesc
{
	std::string name;
	std::string tech_name; // technique name (from Pass Name); empty → Main / pN
	std::vector<PassState> states;
	std::vector<std::pair<std::string, std::string>> macros;
	std::string hlsl;
	std::string vs_entry = "vert";
	std::string ps_entry = "frag";
};

void SkipWs(std::string_view s, size_t& i)
{
	while (i < s.size())
	{
		if (std::isspace(static_cast<unsigned char>(s[i])))
		{
			++i;
			continue;
		}
		// line comment
		if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '/')
		{
			i += 2;
			while (i < s.size() && s[i] != '\n')
			{
				++i;
			}
			continue;
		}
		// block comment
		if (s[i] == '/' && i + 1 < s.size() && s[i + 1] == '*')
		{
			i += 2;
			while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/'))
			{
				++i;
			}
			if (i + 1 < s.size())
			{
				i += 2;
			}
			continue;
		}
		break;
	}
}

bool Match(std::string_view s, size_t& i, std::string_view kw)
{
	SkipWs(s, i);
	if (i + kw.size() > s.size())
	{
		return false;
	}
	if (s.substr(i, kw.size()) != kw)
	{
		return false;
	}
	// keyword boundary
	if (!kw.empty() && std::isalpha(static_cast<unsigned char>(kw.back())))
	{
		size_t end = i + kw.size();
		if (end < s.size() && (std::isalnum(static_cast<unsigned char>(s[end])) || s[end] == '_'))
		{
			return false;
		}
	}
	i += kw.size();
	return true;
}

bool Expect(std::string_view s, size_t& i, std::string_view kw, std::string* err, char const* what)
{
	if (!Match(s, i, kw))
	{
		if (err)
		{
			*err = std::string("ShaderLabLite: expected ") + what + " near offset " + std::to_string(i);
		}
		return false;
	}
	return true;
}

bool ParseQuoted(std::string_view s, size_t& i, std::string& out, std::string* err)
{
	SkipWs(s, i);
	if (i >= s.size() || s[i] != '"')
	{
		if (err)
		{
			*err = "ShaderLabLite: expected quoted string near offset " + std::to_string(i);
		}
		return false;
	}
	++i;
	size_t begin = i;
	while (i < s.size() && s[i] != '"')
	{
		++i;
	}
	if (i >= s.size())
	{
		if (err)
		{
			*err = "ShaderLabLite: unterminated string";
		}
		return false;
	}
	out.assign(s.data() + begin, i - begin);
	++i;
	return true;
}

bool ParseIdent(std::string_view s, size_t& i, std::string& out, std::string* err)
{
	SkipWs(s, i);
	if (i >= s.size() || !(std::isalpha(static_cast<unsigned char>(s[i])) || s[i] == '_'))
	{
		if (err)
		{
			*err = "ShaderLabLite: expected identifier near offset " + std::to_string(i);
		}
		return false;
	}
	size_t begin = i;
	++i;
	while (i < s.size() && (std::isalnum(static_cast<unsigned char>(s[i])) || s[i] == '_'))
	{
		++i;
	}
	out.assign(s.data() + begin, i - begin);
	return true;
}

// Property types may start with a digit (Unity: 2D / 3D).
bool ParsePropType(std::string_view s, size_t& i, std::string& out, std::string* err)
{
	SkipWs(s, i);
	if (i >= s.size() || !(std::isalnum(static_cast<unsigned char>(s[i])) || s[i] == '_'))
	{
		if (err)
		{
			*err = "ShaderLabLite: expected property type near offset " + std::to_string(i);
		}
		return false;
	}
	size_t begin = i;
	++i;
	while (i < s.size() && (std::isalnum(static_cast<unsigned char>(s[i])) || s[i] == '_'))
	{
		++i;
	}
	out.assign(s.data() + begin, i - begin);
	return true;
}

bool FindMatchingBrace(std::string_view s, size_t open_brace, size_t& close_out, std::string* err)
{
	if (open_brace >= s.size() || s[open_brace] != '{')
	{
		if (err)
		{
			*err = "ShaderLabLite: expected '{'";
		}
		return false;
	}
	int depth = 0;
	bool in_str = false;
	for (size_t i = open_brace; i < s.size(); ++i)
	{
		char c = s[i];
		if (in_str)
		{
			if (c == '\\' && i + 1 < s.size())
			{
				++i;
				continue;
			}
			if (c == '"')
			{
				in_str = false;
			}
			continue;
		}
		if (c == '"')
		{
			in_str = true;
			continue;
		}
		if (c == '/' && i + 1 < s.size() && s[i + 1] == '/')
		{
			i += 2;
			while (i < s.size() && s[i] != '\n')
			{
				++i;
			}
			continue;
		}
		if (c == '/' && i + 1 < s.size() && s[i + 1] == '*')
		{
			i += 2;
			while (i + 1 < s.size() && !(s[i] == '*' && s[i + 1] == '/'))
			{
				++i;
			}
			if (i + 1 < s.size())
			{
				++i;
			}
			continue;
		}
		if (c == '{')
		{
			++depth;
		}
		else if (c == '}')
		{
			--depth;
			if (depth == 0)
			{
				close_out = i;
				return true;
			}
		}
	}
	if (err)
	{
		*err = "ShaderLabLite: unmatched '{'";
	}
	return false;
}

std::string_view Slice(std::string_view s, size_t begin, size_t end)
{
	if (end < begin || begin > s.size())
	{
		return {};
	}
	end = std::min(end, s.size());
	return s.substr(begin, end - begin);
}

bool ParseFloatList(std::string_view text, std::vector<float>& out)
{
	out.clear();
	size_t i = 0;
	while (i < text.size())
	{
		while (i < text.size() && (std::isspace(static_cast<unsigned char>(text[i])) || text[i] == ',' || text[i] == '(' || text[i] == ')'))
		{
			++i;
		}
		if (i >= text.size())
		{
			break;
		}
		size_t begin = i;
		if (text[i] == '-' || text[i] == '+')
		{
			++i;
		}
		while (i < text.size() && (std::isdigit(static_cast<unsigned char>(text[i])) || text[i] == '.' || text[i] == 'e' || text[i] == 'E' ||
								   text[i] == '+' || text[i] == '-'))
		{
			// allow scientific notation digits; stop on clear separators
			if ((text[i] == '+' || text[i] == '-') && i > begin && text[i - 1] != 'e' && text[i - 1] != 'E')
			{
				break;
			}
			++i;
		}
		if (begin == i)
		{
			return false;
		}
		try
		{
			out.push_back(std::stof(std::string(text.substr(begin, i - begin))));
		}
		catch (...)
		{
			return false;
		}
	}
	return !out.empty();
}

char const* MapCull(std::string_view v)
{
	if (v == "Off" || v == "off" || v == "None" || v == "none")
	{
		return "none";
	}
	if (v == "Front" || v == "front")
	{
		return "front";
	}
	return "back";
}

char const* MapZTest(std::string_view v)
{
	if (v == "Less" || v == "less")
	{
		return "less";
	}
	if (v == "Greater" || v == "greater")
	{
		return "greater";
	}
	if (v == "LEqual" || v == "LessEqual" || v == "lequal")
	{
		return "less_equal";
	}
	if (v == "GEqual" || v == "GreaterEqual" || v == "gequal")
	{
		return "greater_equal";
	}
	if (v == "Equal" || v == "Equal")
	{
		return "equal";
	}
	if (v == "NotEqual" || v == "notequal")
	{
		return "not_equal";
	}
	if (v == "Always" || v == "always")
	{
		return "always_pass";
	}
	if (v == "Never" || v == "never")
	{
		return "always_fail";
	}
	return "less_equal";
}

char const* MapBlendFactor(std::string_view v)
{
	if (v == "One")
	{
		return "one";
	}
	if (v == "Zero")
	{
		return "zero";
	}
	if (v == "SrcAlpha")
	{
		return "src_alpha";
	}
	if (v == "OneMinusSrcAlpha")
	{
		return "inv_src_alpha";
	}
	if (v == "DstAlpha")
	{
		return "dst_alpha";
	}
	if (v == "OneMinusDstAlpha")
	{
		return "inv_dst_alpha";
	}
	if (v == "SrcColor")
	{
		return "src_color";
	}
	if (v == "OneMinusSrcColor")
	{
		return "inv_src_color";
	}
	if (v == "DstColor")
	{
		return "dst_color";
	}
	if (v == "OneMinusDstColor")
	{
		return "inv_dst_color";
	}
	return "one";
}

bool ParseProperties(std::string_view body, std::vector<PropDesc>& props, std::string* err)
{
	size_t i = 0;
	while (true)
	{
		SkipWs(body, i);
		if (i >= body.size())
		{
			break;
		}
		PropDesc p;
		if (!ParseIdent(body, i, p.name, err))
		{
			return false;
		}
		SkipWs(body, i);
		if (!Expect(body, i, "(", err, "'('"))
		{
			return false;
		}
		if (!ParseQuoted(body, i, p.display_name, err))
		{
			return false;
		}
		SkipWs(body, i);
		if (!Expect(body, i, ",", err, "','"))
		{
			return false;
		}
		SkipWs(body, i);
		if (!ParsePropType(body, i, p.type, err))
		{
			return false;
		}
		if (p.type == "Range")
		{
			SkipWs(body, i);
			if (!Expect(body, i, "(", err, "Range '('"))
			{
				return false;
			}
			size_t range_begin = i;
			while (i < body.size() && body[i] != ')')
			{
				++i;
			}
			if (i >= body.size())
			{
				if (err)
				{
					*err = "ShaderLabLite: Range missing ')'";
				}
				return false;
			}
			std::vector<float> nums;
			if (!ParseFloatList(Slice(body, range_begin, i), nums) || nums.size() < 2)
			{
				if (err)
				{
					*err = "ShaderLabLite: invalid Range bounds";
				}
				return false;
			}
			p.range_min = nums[0];
			p.range_max = nums[1];
			++i; // )
		}
		SkipWs(body, i);
		if (!Expect(body, i, ")", err, "')'"))
		{
			return false;
		}
		SkipWs(body, i);
		if (!Expect(body, i, "=", err, "'='"))
		{
			return false;
		}
		SkipWs(body, i);
		size_t val_begin = i;
		if (i < body.size() && body[i] == '(')
		{
			int depth = 0;
			do
			{
				if (body[i] == '(')
				{
					++depth;
				}
				else if (body[i] == ')')
				{
					--depth;
				}
				++i;
			} while (i < body.size() && depth > 0);
		}
		else if (i < body.size() && body[i] == '"')
		{
			++i;
			while (i < body.size() && body[i] != '"')
			{
				++i;
			}
			if (i < body.size())
			{
				++i;
			}
		}
		else if (i < body.size() && body[i] == '{')
		{
			// Matrix / empty object default: {}
			size_t close = 0;
			if (!FindMatchingBrace(body, i, close, err))
			{
				return false;
			}
			i = close + 1;
		}
		else
		{
			while (i < body.size() && body[i] != '\n' && body[i] != '}')
			{
				++i;
			}
		}
		p.default_value = std::string(Slice(body, val_begin, i));
		// trim
		while (!p.default_value.empty() && std::isspace(static_cast<unsigned char>(p.default_value.back())))
		{
			p.default_value.pop_back();
		}
		// Unity texture defaults often end with optional {}
		SkipWs(body, i);
		if (i < body.size() && body[i] == '{')
		{
			size_t close = 0;
			if (!FindMatchingBrace(body, i, close, err))
			{
				return false;
			}
			i = close + 1;
		}
		props.push_back(std::move(p));
	}
	return true;
}

void ExtractPragmasAndStrip(std::string& hlsl, std::string& vs, std::string& ps)
{
	std::istringstream iss(hlsl);
	std::ostringstream oss;
	std::string line;
	while (std::getline(iss, line))
	{
		size_t pos = line.find("#pragma");
		if (pos != std::string::npos)
		{
			std::istringstream ls(line.substr(pos + 7));
			std::string cmd;
			std::string name;
			ls >> cmd >> name;
			if (cmd == "vertex" && !name.empty())
			{
				vs = name;
			}
			else if ((cmd == "fragment" || cmd == "pixel") && !name.empty())
			{
				ps = name;
			}
			continue; // do not emit Unity pragmas into FXC input
		}
		oss << line << '\n';
	}
	hlsl = oss.str();
}

bool ParsePassBody(std::string_view body, PassDesc& pass, std::string* err)
{
	size_t i = 0;
	while (true)
	{
		SkipWs(body, i);
		if (i >= body.size())
		{
			break;
		}

		// HLSLPROGRAM ... ENDHLSL  or CGPROGRAM ... ENDCG
		if (Match(body, i, "HLSLPROGRAM") || Match(body, i, "CGPROGRAM"))
		{
			size_t prog_begin = i;
			size_t end_pos = body.find("ENDHLSL", prog_begin);
			size_t end_len = 7;
			if (end_pos == std::string_view::npos)
			{
				end_pos = body.find("ENDCG", prog_begin);
				end_len = 5;
			}
			if (end_pos == std::string_view::npos)
			{
				if (err)
				{
					*err = "ShaderLabLite: missing ENDHLSL/ENDCG";
				}
				return false;
			}
			pass.hlsl = std::string(Slice(body, prog_begin, end_pos));
			ExtractPragmasAndStrip(pass.hlsl, pass.vs_entry, pass.ps_entry);
			i = end_pos + end_len;
			continue;
		}

		std::string cmd;
		if (!ParseIdent(body, i, cmd, err))
		{
			return false;
		}

		if (cmd == "Name")
		{
			std::string n;
			if (!ParseQuoted(body, i, n, err))
			{
				return false;
			}
			pass.tech_name = n;
			if (pass.name.empty() || pass.name[0] == 'p')
			{
				pass.name = "p0";
			}
			continue;
		}
		if (cmd == "Macro")
		{
			std::string name;
			if (!ParseIdent(body, i, name, err))
			{
				return false;
			}
			SkipWs(body, i);
			if (!Expect(body, i, "=", err, "'='"))
			{
				return false;
			}
			SkipWs(body, i);
			size_t begin = i;
			while (i < body.size() && body[i] != '\n')
			{
				++i;
			}
			std::string value(Slice(body, begin, i));
			while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
			{
				value.pop_back();
			}
			pass.macros.emplace_back(std::move(name), std::move(value));
			continue;
		}
		if (cmd == "State")
		{
			std::string st_name;
			if (!ParseIdent(body, i, st_name, err))
			{
				return false;
			}
			int index = -1;
			SkipWs(body, i);
			if (i < body.size() && body[i] == '[')
			{
				++i;
				size_t begin = i;
				while (i < body.size() && body[i] != ']')
				{
					++i;
				}
				index = std::stoi(std::string(Slice(body, begin, i)));
				if (i < body.size())
				{
					++i;
				}
			}
			SkipWs(body, i);
			if (!Expect(body, i, "=", err, "'='"))
			{
				return false;
			}
			SkipWs(body, i);
			size_t begin = i;
			while (i < body.size() && body[i] != '\n')
			{
				++i;
			}
			std::string value(Slice(body, begin, i));
			while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
			{
				value.pop_back();
			}
			pass.states.push_back({st_name, value, index});
			continue;
		}
		if (cmd == "Tags")
		{
			SkipWs(body, i);
			if (i >= body.size() || body[i] != '{')
			{
				if (err)
				{
					*err = "ShaderLabLite: Tags expects '{'";
				}
				return false;
			}
			size_t close = 0;
			if (!FindMatchingBrace(body, i, close, err))
			{
				return false;
			}
			i = close + 1;
			continue;
		}
		if (cmd == "Cull")
		{
			std::string v;
			if (!ParseIdent(body, i, v, err))
			{
				return false;
			}
			pass.states.push_back({"cull_mode", MapCull(v), -1});
			continue;
		}
		if (cmd == "ZWrite")
		{
			std::string v;
			if (!ParseIdent(body, i, v, err))
			{
				return false;
			}
			bool on = (v == "On" || v == "on" || v == "True" || v == "true" || v == "1");
			pass.states.push_back({"depth_write_mask", on ? "true" : "false", -1});
			continue;
		}
		if (cmd == "ZTest")
		{
			std::string v;
			if (!ParseIdent(body, i, v, err))
			{
				return false;
			}
			pass.states.push_back({"depth_func", MapZTest(v), -1});
			continue;
		}
		if (cmd == "Blend")
		{
			SkipWs(body, i);
			std::string a;
			if (!ParseIdent(body, i, a, err))
			{
				return false;
			}
			if (a == "Off" || a == "off")
			{
				pass.states.push_back({"blend_enable", "false", 0});
				continue;
			}
			std::string b;
			if (!ParseIdent(body, i, b, err))
			{
				return false;
			}
			pass.states.push_back({"blend_enable", "true", 0});
			pass.states.push_back({"src_blend", MapBlendFactor(a), 0});
			pass.states.push_back({"dest_blend", MapBlendFactor(b), 0});
			pass.states.push_back({"src_blend_alpha", MapBlendFactor(a), 0});
			pass.states.push_back({"dest_blend_alpha", MapBlendFactor(b), 0});
			continue;
		}

		// Unknown command: skip until end of line / next token block
		while (i < body.size() && body[i] != '\n' && body[i] != '{')
		{
			++i;
		}
		if (i < body.size() && body[i] == '{')
		{
			size_t close = 0;
			if (!FindMatchingBrace(body, i, close, err))
			{
				return false;
			}
			i = close + 1;
		}
	}
	return true;
}

XMLNode MakeState(std::string_view name, std::string_view value, int index = -1)
{
	XMLNode st(XMLNodeType::Element, "state");
	st.AppendAttrib(XMLAttribute("name", name));
	st.AppendAttrib(XMLAttribute("value", value));
	if (index >= 0)
	{
		st.AppendAttrib(XMLAttribute("index", index));
	}
	return st;
}

void AppendSamplerStates(XMLNode& samp, bool cube_like)
{
	{
		XMLNode st(XMLNodeType::Element, "state");
		st.AppendAttrib(XMLAttribute("name", std::string_view("filtering")));
		st.AppendAttrib(XMLAttribute("value",
			std::string_view(cube_like ? "min_mag_linear_mip_point" : "min_mag_mip_linear")));
		samp.AppendNode(std::move(st));
	}
	char const* addr = cube_like ? "clamp" : "wrap";
	char const* axes[] = {"address_u", "address_v", "address_w"};
	size_t const axis_count = cube_like ? 3 : 2;
	for (size_t ai = 0; ai < axis_count; ++ai)
	{
		XMLNode st(XMLNodeType::Element, "state");
		st.AppendAttrib(XMLAttribute("name", std::string_view(axes[ai])));
		st.AppendAttrib(XMLAttribute("value", std::string_view(addr)));
		samp.AppendNode(std::move(st));
	}
}

char const* MapScalarType(std::string const& type)
{
	if (type == "Color" || type == "Vector" || type == "Float4")
	{
		return "float4";
	}
	if (type == "Float3")
	{
		return "float3";
	}
	if (type == "Float2")
	{
		return "float2";
	}
	if (type == "Float" || type == "Range")
	{
		return "float";
	}
	if (type == "Int")
	{
		return "int";
	}
	if (type == "Int2")
	{
		return "int2";
	}
	if (type == "Int3")
	{
		return "int3";
	}
	if (type == "Int4")
	{
		return "int4";
	}
	if (type == "UInt" || type == "Uint")
	{
		return "uint";
	}
	if (type == "UInt2")
	{
		return "uint2";
	}
	if (type == "UInt3")
	{
		return "uint3";
	}
	if (type == "UInt4")
	{
		return "uint4";
	}
	if (type == "Bool")
	{
		return "bool";
	}
	if (type == "Matrix" || type == "Float4x4" || type == "Matrix4x4")
	{
		return "float4x4";
	}
	return nullptr;
}

bool ParseAttrAssignments(std::string_view s, size_t& i, PropDesc& p)
{
	// optional: semantic="X" array_size="N" x=1 y=2 z=3 w=4 value=1
	while (true)
	{
		SkipWs(s, i);
		if (i >= s.size() || s[i] == '\n' || s[i] == '{' || s[i] == '}')
		{
			break;
		}
		std::string key;
		size_t save = i;
		if (!ParseIdent(s, i, key, nullptr))
		{
			i = save;
			break;
		}
		SkipWs(s, i);
		if (i >= s.size() || s[i] != '=')
		{
			i = save;
			break;
		}
		++i;
		SkipWs(s, i);
		std::string val;
		if (i < s.size() && s[i] == '"')
		{
			if (!ParseQuoted(s, i, val, nullptr))
			{
				return false;
			}
		}
		else
		{
			size_t begin = i;
			while (i < s.size() && !std::isspace(static_cast<unsigned char>(s[i])) && s[i] != '{' && s[i] != '}')
			{
				++i;
			}
			val = std::string(Slice(s, begin, i));
		}
		if (key == "semantic")
		{
			p.semantic = val;
		}
		else if (key == "array_size")
		{
			p.array_size = val;
		}
		else if (key == "x" || key == "y" || key == "z" || key == "w")
		{
			p.has_xyzw = true;
			float f = 0.f;
			try
			{
				f = std::stof(val);
			}
			catch (...)
			{
			}
			if (key == "x")
			{
				p.xyzw[0] = f;
			}
			else if (key == "y")
			{
				p.xyzw[1] = f;
			}
			else if (key == "z")
			{
				p.xyzw[2] = f;
			}
			else
			{
				p.xyzw[3] = f;
			}
		}
		else if (key == "value")
		{
			p.has_value = true;
			try
			{
				p.value_f = std::stof(val);
			}
			catch (...)
			{
				p.default_value = val;
			}
		}
	}
	return true;
}

void AppendParamNode(XMLNode& host, PropDesc const& p, bool& emitted_shared_cube_sampler)
{
	if (p.type == "Sampler")
	{
		XMLNode samp(XMLNodeType::Element, "parameter");
		samp.AppendAttrib(XMLAttribute("type", std::string_view("sampler")));
		samp.AppendAttrib(XMLAttribute("name", p.name));
		if (!p.sampler_states.empty())
		{
			for (auto const& st : p.sampler_states)
			{
				samp.AppendNode(MakeState(st.name, st.value, st.index));
			}
		}
		else
		{
			AppendSamplerStates(samp, false);
		}
		host.AppendNode(std::move(samp));
		return;
	}

	if (p.type == "2D" || p.type == "Cube" || p.type == "3D" || p.type == "Texture2D" || p.type == "TextureCube" ||
		p.type == "Texture3D")
	{
		char const* tex_type = "texture2D";
		bool const is_cube = (p.type == "Cube" || p.type == "TextureCube");
		if (is_cube)
		{
			tex_type = "textureCUBE";
		}
		else if (p.type == "3D" || p.type == "Texture3D")
		{
			tex_type = "texture3D";
		}

		XMLNode tex(XMLNodeType::Element, "parameter");
		tex.AppendAttrib(XMLAttribute("type", std::string_view(tex_type)));
		tex.AppendAttrib(XMLAttribute("name", p.name));
		tex.AppendAttrib(XMLAttribute("elem_type", std::string_view("float4")));
		host.AppendNode(std::move(tex));

		// Cube maps in this engine often share one sampler (e.g. skybox_sampler).
		if (is_cube)
		{
			if (!emitted_shared_cube_sampler)
			{
				XMLNode samp(XMLNodeType::Element, "parameter");
				samp.AppendAttrib(XMLAttribute("type", std::string_view("sampler")));
				samp.AppendAttrib(XMLAttribute("name", std::string_view("skybox_sampler")));
				AppendSamplerStates(samp, true);
				host.AppendNode(std::move(samp));
				emitted_shared_cube_sampler = true;
			}
			return;
		}

		// Library Texture2D declarations (Material.shader) do not auto-add samplers.
		if (p.type == "Texture2D" || p.type == "Texture3D")
		{
			return;
		}

		XMLNode samp(XMLNodeType::Element, "parameter");
		samp.AppendAttrib(XMLAttribute("type", std::string_view("sampler")));
		// Unity-style: Texture _MainTex + SamplerState sampler_MainTex
		samp.AppendAttrib(XMLAttribute("name", "sampler" + p.name));
		AppendSamplerStates(samp, false);
		host.AppendNode(std::move(samp));
		return;
	}

	XMLNode node(XMLNodeType::Element, "parameter");
	char const* mapped = MapScalarType(p.type);
	if (!p.custom_type.empty())
	{
		node.AppendAttrib(XMLAttribute("type", p.custom_type));
	}
	else if (mapped)
	{
		node.AppendAttrib(XMLAttribute("type", std::string_view(mapped)));
	}
	else if (p.type == "Color" || p.type == "Vector")
	{
		node.AppendAttrib(XMLAttribute("type", std::string_view("float4")));
	}
	else
	{
		// Assume custom struct type name (e.g. KlayGECameraInfo)
		node.AppendAttrib(XMLAttribute("type", p.type));
	}
	node.AppendAttrib(XMLAttribute("name", p.name));
	if (!p.semantic.empty())
	{
		node.AppendAttrib(XMLAttribute("semantic", p.semantic));
	}
	if (!p.array_size.empty())
	{
		node.AppendAttrib(XMLAttribute("array_size", p.array_size));
	}

	if (p.has_xyzw)
	{
		node.AppendAttrib(XMLAttribute("x", p.xyzw[0]));
		node.AppendAttrib(XMLAttribute("y", p.xyzw[1]));
		node.AppendAttrib(XMLAttribute("z", p.xyzw[2]));
		node.AppendAttrib(XMLAttribute("w", p.xyzw[3]));
	}
	else if (p.has_value)
	{
		if (p.type == "Int" || p.type == "UInt" || p.type == "Uint" || p.type == "Bool")
		{
			node.AppendAttrib(XMLAttribute("value", static_cast<int32_t>(p.value_f)));
		}
		else
		{
			node.AppendAttrib(XMLAttribute("value", p.value_f));
		}
	}
	else if (!p.default_value.empty())
	{
		std::vector<float> nums;
		if (ParseFloatList(p.default_value, nums))
		{
			if (nums.size() >= 2)
			{
				node.AppendAttrib(XMLAttribute("x", nums[0]));
				node.AppendAttrib(XMLAttribute("y", nums.size() > 1 ? nums[1] : 0.f));
				if (nums.size() > 2)
				{
					node.AppendAttrib(XMLAttribute("z", nums[2]));
				}
				if (nums.size() > 3)
				{
					node.AppendAttrib(XMLAttribute("w", nums[3]));
				}
			}
			else
			{
				node.AppendAttrib(XMLAttribute("value", nums[0]));
			}
		}
	}
	host.AppendNode(std::move(node));
}

bool ParseCBufferMembers(std::string_view body, std::string const& cb_name, std::vector<PropDesc>& props, std::string* err)
{
	size_t i = 0;
	while (true)
	{
		SkipWs(body, i);
		if (i >= body.size())
		{
			break;
		}
		std::string type;
		if (!ParsePropType(body, i, type, err))
		{
			return false;
		}
		std::string name;
		if (!ParseIdent(body, i, name, err))
		{
			return false;
		}
		PropDesc p;
		p.name = std::move(name);
		p.display_name = p.name;
		p.type = std::move(type);
		p.cbuffer_name = cb_name;
		if (!ParseAttrAssignments(body, i, p))
		{
			return false;
		}
		props.push_back(std::move(p));
	}
	return true;
}

bool ParseTopLevelTypedParam(std::string_view s, size_t& i, std::vector<PropDesc>& props, std::string* err)
{
	size_t save = i;
	std::string kw;
	if (!ParseIdent(s, i, kw, err))
	{
		return false;
	}
	bool const is_tex = (kw == "Texture2D" || kw == "TextureCube" || kw == "Texture3D");
	bool const is_samp = (kw == "Sampler");
	bool const is_scalar = (MapScalarType(kw) != nullptr) || (kw == "Color") || (kw == "Vector") || (kw == "Range");
	if (!is_tex && !is_samp && !is_scalar)
	{
		// custom struct-typed parameter: TypeName name array_size="N"
		// Heuristic: next token is ident and looks like a param (not Keywords)
		std::string name;
		size_t after_type = i;
		if (!ParseIdent(s, i, name, nullptr))
		{
			i = save;
			if (err)
			{
				err->clear();
			}
			return false;
		}
		PropDesc p;
		p.name = std::move(name);
		p.display_name = p.name;
		p.type = kw;
		p.custom_type = kw;
		if (!ParseAttrAssignments(s, i, p))
		{
			i = save;
			return false;
		}
		// If nothing consumed after name and no attrs, still accept
		(void)after_type;
		props.push_back(std::move(p));
		return true;
	}

	std::string name;
	if (!ParseIdent(s, i, name, err))
	{
		i = save;
		return false;
	}
	PropDesc p;
	p.name = std::move(name);
	p.display_name = p.name;
	p.type = std::move(kw);
	if (!ParseAttrAssignments(s, i, p))
	{
		return false;
	}
	if (is_samp)
	{
		SkipWs(s, i);
		if (i < s.size() && s[i] == '{')
		{
			size_t close = 0;
			if (!FindMatchingBrace(s, i, close, err))
			{
				return false;
			}
			std::string_view body = Slice(s, i + 1, close);
			size_t si = 0;
			while (true)
			{
				SkipWs(body, si);
				if (si >= body.size())
				{
					break;
				}
				std::string st_kw;
				if (!ParseIdent(body, si, st_kw, err))
				{
					return false;
				}
				if (st_kw != "State")
				{
					*err = "ShaderLabLite: Sampler body expects State";
					return false;
				}
				std::string st_name;
				if (!ParseIdent(body, si, st_name, err))
				{
					return false;
				}
				int index = -1;
				SkipWs(body, si);
				if (si < body.size() && body[si] == '[')
				{
					++si;
					size_t begin = si;
					while (si < body.size() && body[si] != ']')
					{
						++si;
					}
					index = std::stoi(std::string(Slice(body, begin, si)));
					if (si < body.size())
					{
						++si;
					}
				}
				SkipWs(body, si);
				if (!Expect(body, si, "=", err, "'='"))
				{
					return false;
				}
				SkipWs(body, si);
				size_t vbegin = si;
				while (si < body.size() && body[si] != '\n')
				{
					++si;
				}
				std::string val(Slice(body, vbegin, si));
				while (!val.empty() && std::isspace(static_cast<unsigned char>(val.back())))
				{
					val.pop_back();
				}
				p.sampler_states.push_back({st_name, val, index});
			}
			i = close + 1;
		}
	}
	props.push_back(std::move(p));
	return true;
}

} // namespace — helpers above; ParseShaderLabLite below uses anonymous helpers

namespace
{
bool LoadEffectXmlFromString(std::string const& xml_text, XMLNode& out_root, std::string* err)
{
	auto buf = MakeSharedPtr<std::string>(xml_text);
	auto stream = MakeSharedPtr<std::istringstream>(*buf);
	ResIdentifier res("shaderlab_fxml", 0, stream);
	try
	{
		out_root = LoadXml(res);
		return true;
	}
	catch (...)
	{
		if (err)
		{
			*err = "ShaderLabLite: failed to parse FXMLPROGRAM XML";
		}
		return false;
	}
}
} // namespace

bool ParseShaderLabLite(std::string_view source, XMLNode& out_effect, std::string* error_msg)
{
	std::string err_local;
	std::string* err = error_msg ? error_msg : &err_local;

	size_t i = 0;
	if (!Expect(source, i, "Shader", err, "Shader"))
	{
		return false;
	}
	std::string shader_name;
	if (!ParseQuoted(source, i, shader_name, err))
	{
		return false;
	}
	SkipWs(source, i);
	if (i >= source.size() || source[i] != '{')
	{
		*err = "ShaderLabLite: expected '{' after Shader name";
		return false;
	}
	size_t shader_close = 0;
	if (!FindMatchingBrace(source, i, shader_close, err))
	{
		return false;
	}
	std::string_view shader_body = Slice(source, i + 1, shader_close);

	// Fast path: whole body is FXMLPROGRAM ... ENDFXML wrapping a full <effect>
	{
		size_t bi = 0;
		SkipWs(shader_body, bi);
		// skip // comments
		while (bi + 1 < shader_body.size() && shader_body[bi] == '/' && shader_body[bi + 1] == '/')
		{
			bi += 2;
			while (bi < shader_body.size() && shader_body[bi] != '\n')
			{
				++bi;
			}
			SkipWs(shader_body, bi);
		}
		if (Match(shader_body, bi, "FXMLPROGRAM"))
		{
			// Match() already consumed "FXMLPROGRAM"; body until ENDFXML is raw effect XML.
			size_t end_pos = shader_body.find("ENDFXML", bi);
			if (end_pos == std::string_view::npos)
			{
				*err = "ShaderLabLite: missing ENDFXML";
				return false;
			}
			std::string xml_text(Slice(shader_body, bi, end_pos));
			while (!xml_text.empty() && std::isspace(static_cast<unsigned char>(xml_text.front())))
			{
				xml_text.erase(xml_text.begin());
			}
			while (!xml_text.empty() && std::isspace(static_cast<unsigned char>(xml_text.back())))
			{
				xml_text.pop_back();
			}
			XMLNode root(XMLNodeType::Element, "effect");
			if (!LoadEffectXmlFromString(xml_text, root, err))
			{
				return false;
			}
			out_effect = std::move(root);
			(void)shader_name;
			return true;
		}
	}

	std::vector<PropDesc> props;
	std::vector<PassDesc> passes;
	std::vector<std::string> includes;
	std::string library_hlsl;
	std::vector<StructDesc> structs;
	std::vector<std::pair<std::string, std::string>> macros;
	std::string fxml_snippets;

	size_t bi = 0;
	while (true)
	{
		SkipWs(shader_body, bi);
		if (bi >= shader_body.size())
		{
			break;
		}

		if (Match(shader_body, bi, "FXMLPROGRAM"))
		{
			// Match() already consumed "FXMLPROGRAM".
			size_t end_pos = shader_body.find("ENDFXML", bi);
			if (end_pos == std::string_view::npos)
			{
				*err = "ShaderLabLite: missing ENDFXML";
				return false;
			}
			fxml_snippets += std::string(Slice(shader_body, bi, end_pos));
			fxml_snippets += '\n';
			bi = end_pos + 7;
			continue;
		}

		if (Match(shader_body, bi, "Macro"))
		{
			std::string name;
			if (!ParseIdent(shader_body, bi, name, err))
			{
				return false;
			}
			SkipWs(shader_body, bi);
			if (!Expect(shader_body, bi, "=", err, "'='"))
			{
				return false;
			}
			SkipWs(shader_body, bi);
			size_t begin = bi;
			while (bi < shader_body.size() && shader_body[bi] != '\n')
			{
				++bi;
			}
			std::string value(Slice(shader_body, begin, bi));
			while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
			{
				value.pop_back();
			}
			macros.emplace_back(std::move(name), std::move(value));
			continue;
		}

		if (Match(shader_body, bi, "Struct"))
		{
			StructDesc sd;
			if (!ParseIdent(shader_body, bi, sd.name, err))
			{
				return false;
			}
			SkipWs(shader_body, bi);
			if (bi >= shader_body.size() || shader_body[bi] != '{')
			{
				*err = "ShaderLabLite: Struct expects '{'";
				return false;
			}
			size_t close = 0;
			if (!FindMatchingBrace(shader_body, bi, close, err))
			{
				return false;
			}
			std::string_view body = Slice(shader_body, bi + 1, close);
			size_t mi = 0;
			while (true)
			{
				SkipWs(body, mi);
				if (mi >= body.size())
				{
					break;
				}
				StructMemberDesc mem;
				if (!ParsePropType(body, mi, mem.type, err))
				{
					return false;
				}
				if (!ParseIdent(body, mi, mem.name, err))
				{
					return false;
				}
				PropDesc tmp;
				if (!ParseAttrAssignments(body, mi, tmp))
				{
					return false;
				}
				mem.array_size = tmp.array_size;
				// Map type names to fxml style
				char const* mapped = MapScalarType(mem.type);
				if (mapped)
				{
					mem.type = mapped;
				}
				sd.members.push_back(std::move(mem));
			}
			structs.push_back(std::move(sd));
			bi = close + 1;
			continue;
		}

		if (Match(shader_body, bi, "Include"))
		{
			std::string inc;
			if (!ParseQuoted(shader_body, bi, inc, err))
			{
				return false;
			}
			if (!inc.empty())
			{
				includes.push_back(std::move(inc));
			}
			continue;
		}

		// Top-level HLSLPROGRAM for library shaders (util.shader)
		if (Match(shader_body, bi, "HLSLPROGRAM") || Match(shader_body, bi, "CGPROGRAM"))
		{
			size_t prog_begin = bi;
			size_t end_pos = shader_body.find("ENDHLSL", prog_begin);
			size_t end_len = 7;
			if (end_pos == std::string_view::npos)
			{
				end_pos = shader_body.find("ENDCG", prog_begin);
				end_len = 5;
			}
			if (end_pos == std::string_view::npos)
			{
				*err = "ShaderLabLite: missing ENDHLSL/ENDCG";
				return false;
			}
			std::string chunk = std::string(Slice(shader_body, prog_begin, end_pos));
			std::string vs, ps;
			ExtractPragmasAndStrip(chunk, vs, ps);
			library_hlsl += chunk;
			library_hlsl += '\n';
			bi = end_pos + end_len;
			continue;
		}

		if (Match(shader_body, bi, "CBuffer") || Match(shader_body, bi, "cbuffer"))
		{
			std::string cb_name;
			SkipWs(shader_body, bi);
			if (bi < shader_body.size() && shader_body[bi] == '"')
			{
				if (!ParseQuoted(shader_body, bi, cb_name, err))
				{
					return false;
				}
			}
			else if (!ParseIdent(shader_body, bi, cb_name, err))
			{
				return false;
			}
			SkipWs(shader_body, bi);
			if (bi >= shader_body.size() || shader_body[bi] != '{')
			{
				*err = "ShaderLabLite: CBuffer expects '{'";
				return false;
			}
			size_t close = 0;
			if (!FindMatchingBrace(shader_body, bi, close, err))
			{
				return false;
			}
			if (!ParseCBufferMembers(Slice(shader_body, bi + 1, close), cb_name, props, err))
			{
				return false;
			}
			bi = close + 1;
			continue;
		}

		{
			size_t save = bi;
			if (ParseTopLevelTypedParam(shader_body, bi, props, err))
			{
				continue;
			}
			bi = save;
			if (err)
			{
				err->clear();
			}
		}

		if (Match(shader_body, bi, "Properties"))
		{
			SkipWs(shader_body, bi);
			if (bi >= shader_body.size() || shader_body[bi] != '{')
			{
				*err = "ShaderLabLite: Properties expects '{'";
				return false;
			}
			size_t close = 0;
			if (!FindMatchingBrace(shader_body, bi, close, err))
			{
				return false;
			}
			if (!ParseProperties(Slice(shader_body, bi + 1, close), props, err))
			{
				return false;
			}
			bi = close + 1;
			continue;
		}

		if (Match(shader_body, bi, "SubShader"))
		{
			SkipWs(shader_body, bi);
			if (bi >= shader_body.size() || shader_body[bi] != '{')
			{
				*err = "ShaderLabLite: SubShader expects '{'";
				return false;
			}
			size_t sub_close = 0;
			if (!FindMatchingBrace(shader_body, bi, sub_close, err))
			{
				return false;
			}
			std::string_view sub_body = Slice(shader_body, bi + 1, sub_close);
			size_t si = 0;
			while (true)
			{
				SkipWs(sub_body, si);
				if (si >= sub_body.size())
				{
					break;
				}
				if (Match(sub_body, si, "Pass"))
				{
					SkipWs(sub_body, si);
					if (si >= sub_body.size() || sub_body[si] != '{')
					{
						*err = "ShaderLabLite: Pass expects '{'";
						return false;
					}
					size_t pass_close = 0;
					if (!FindMatchingBrace(sub_body, si, pass_close, err))
					{
						return false;
					}
					PassDesc pass;
					pass.name = "p" + std::to_string(passes.size());
					if (!ParsePassBody(Slice(sub_body, si + 1, pass_close), pass, err))
					{
						return false;
					}
					passes.push_back(std::move(pass));
					si = pass_close + 1;
					continue;
				}
				if (Match(sub_body, si, "Tags") || Match(sub_body, si, "LOD"))
				{
					SkipWs(sub_body, si);
					if (si < sub_body.size() && sub_body[si] == '{')
					{
						size_t close = 0;
						if (!FindMatchingBrace(sub_body, si, close, err))
						{
							return false;
						}
						si = close + 1;
					}
					else
					{
						while (si < sub_body.size() && sub_body[si] != '\n')
						{
							++si;
						}
					}
					continue;
				}
				// skip unknown blocks / tokens in SubShader
				std::string skip_id;
				if (!ParseIdent(sub_body, si, skip_id, err))
				{
					// try skip one char
					++si;
					err->clear();
					continue;
				}
				SkipWs(sub_body, si);
				if (si < sub_body.size() && sub_body[si] == '{')
				{
					size_t close = 0;
					if (!FindMatchingBrace(sub_body, si, close, err))
					{
						return false;
					}
					si = close + 1;
				}
			}
			bi = sub_close + 1;
			// Collect all SubShaders' passes (Lite: usually one SubShader).
			continue;
		}

		if (Match(shader_body, bi, "Fallback") || Match(shader_body, bi, "CustomEditor"))
		{
			SkipWs(shader_body, bi);
			std::string tmp;
			ParseQuoted(shader_body, bi, tmp, nullptr);
			ParseIdent(shader_body, bi, tmp, nullptr);
			continue;
		}

		std::string skip_id;
		if (!ParseIdent(shader_body, bi, skip_id, err))
		{
			++bi;
			err->clear();
			continue;
		}
		SkipWs(shader_body, bi);
		if (bi < shader_body.size() && shader_body[bi] == '{')
		{
			size_t close = 0;
			if (!FindMatchingBrace(shader_body, bi, close, err))
			{
				return false;
			}
			bi = close + 1;
		}
	}

	bool const is_library = passes.empty();
	if (is_library && library_hlsl.empty() && props.empty() && includes.empty())
	{
		*err = "ShaderLabLite: empty shader (no Pass / HLSL / Properties / CBuffer)";
		return false;
	}

	// Merge HLSL from library block + all passes
	std::ostringstream hlsl_all;
	hlsl_all << library_hlsl;
	for (auto const& pass : passes)
	{
		hlsl_all << pass.hlsl << "\n";
	}

	XMLNode effect(XMLNodeType::Element, "effect");

	for (auto const& m : macros)
	{
		XMLNode mac(XMLNodeType::Element, "macro");
		mac.AppendAttrib(XMLAttribute("name", m.first));
		mac.AppendAttrib(XMLAttribute("value", m.second));
		effect.AppendNode(std::move(mac));
	}
	for (auto const& sd : structs)
	{
		XMLNode st(XMLNodeType::Element, "struct");
		st.AppendAttrib(XMLAttribute("name", sd.name));
		for (auto const& mem : sd.members)
		{
			XMLNode p(XMLNodeType::Element, "parameter");
			p.AppendAttrib(XMLAttribute("type", mem.type));
			p.AppendAttrib(XMLAttribute("name", mem.name));
			if (!mem.array_size.empty())
			{
				p.AppendAttrib(XMLAttribute("array_size", mem.array_size));
			}
			st.AppendNode(std::move(p));
		}
		effect.AppendNode(std::move(st));
	}
	(void)fxml_snippets;

	auto is_util_inc = [](std::string const& n) {
		return n == "util.shader";
	};
	auto is_mesh_inc = [](std::string const& n) {
		return n == "Mesh.shader";
	};

	bool has_util = false;
	bool has_model_camera = false;
	bool has_mesh = false;
	for (auto const& inc_name : includes)
	{
		if (is_util_inc(inc_name))
		{
			has_util = true;
		}
		if (inc_name == "ModelCamera.shader" || inc_name == "ModelCamera.fxml")
		{
			has_model_camera = true;
		}
		if (is_mesh_inc(inc_name))
		{
			has_mesh = true;
		}
		XMLNode inc(XMLNodeType::Element, "include");
		inc.AppendAttrib(XMLAttribute("name", inc_name));
		effect.AppendNode(std::move(inc));
	}
	// Default engine helpers for drawable shaders (not for library includes).
	if (!is_library && !has_util)
	{
		if (!has_model_camera)
		{
			XMLNode inc(XMLNodeType::Element, "include");
			inc.AppendAttrib(XMLAttribute("name", std::string_view("ModelCamera.shader")));
			effect.AppendNode(std::move(inc));
		}
		if (!has_mesh)
		{
			XMLNode inc(XMLNodeType::Element, "include");
			inc.AppendAttrib(XMLAttribute("name", std::string_view("Mesh.shader")));
			effect.AppendNode(std::move(inc));
		}
	}

	bool emitted_shared_cube_sampler = false;
	// Emit named cbuffers first, then top-level params.
	std::vector<std::string> cb_order;
	for (auto const& p : props)
	{
		if (!p.cbuffer_name.empty() &&
			std::find(cb_order.begin(), cb_order.end(), p.cbuffer_name) == cb_order.end())
		{
			cb_order.push_back(p.cbuffer_name);
		}
	}
	for (auto const& cb_name : cb_order)
	{
		XMLNode cb(XMLNodeType::Element, "cbuffer");
		cb.AppendAttrib(XMLAttribute("name", cb_name));
		for (auto const& p : props)
		{
			if (p.cbuffer_name == cb_name)
			{
				AppendParamNode(cb, p, emitted_shared_cube_sampler);
			}
		}
		effect.AppendNode(std::move(cb));
	}
	for (auto const& p : props)
	{
		if (p.cbuffer_name.empty())
		{
			AppendParamNode(effect, p, emitted_shared_cube_sampler);
		}
	}

	std::string const hlsl_text = hlsl_all.str();
	if (!hlsl_text.empty())
	{
		XMLNode shader_node(XMLNodeType::Element, "shader");
		XMLNode cdata(XMLNodeType::CData, "");
		cdata.Value(hlsl_text);
		shader_node.AppendNode(std::move(cdata));
		effect.AppendNode(std::move(shader_node));
	}

	(void)shader_name;
	if (!is_library)
	{
		// One technique per Pass when Pass has Name "...Tech"; otherwise single Main.
		bool const multi_tech = std::any_of(passes.begin(), passes.end(),
			[](PassDesc const& p) { return !p.tech_name.empty(); });

		if (!multi_tech)
		{
			XMLNode tech(XMLNodeType::Element, "technique");
			tech.AppendAttrib(XMLAttribute("name", std::string_view("Main")));
			for (auto const& pass : passes)
			{
				XMLNode pass_node(XMLNodeType::Element, "pass");
				pass_node.AppendAttrib(XMLAttribute("name", pass.name));
				for (auto const& m : pass.macros)
				{
					XMLNode mac(XMLNodeType::Element, "macro");
					mac.AppendAttrib(XMLAttribute("name", m.first));
					mac.AppendAttrib(XMLAttribute("value", m.second));
					pass_node.AppendNode(std::move(mac));
				}
				for (auto const& st : pass.states)
				{
					pass_node.AppendNode(MakeState(st.name, st.value, st.index));
				}
				pass_node.AppendNode(MakeState("vertex_shader", pass.vs_entry + "()"));
				pass_node.AppendNode(MakeState("pixel_shader", pass.ps_entry + "()"));
				tech.AppendNode(std::move(pass_node));
			}
			effect.AppendNode(std::move(tech));
		}
		else
		{
			for (auto const& pass : passes)
			{
				XMLNode tech(XMLNodeType::Element, "technique");
				std::string const tech_name = pass.tech_name.empty() ? "Main" : pass.tech_name;
				tech.AppendAttrib(XMLAttribute("name", tech_name));

				XMLNode pass_node(XMLNodeType::Element, "pass");
				pass_node.AppendAttrib(XMLAttribute("name", pass.name.empty() ? std::string("p0") : pass.name));
				for (auto const& m : pass.macros)
				{
					XMLNode mac(XMLNodeType::Element, "macro");
					mac.AppendAttrib(XMLAttribute("name", m.first));
					mac.AppendAttrib(XMLAttribute("value", m.second));
					pass_node.AppendNode(std::move(mac));
				}
				for (auto const& st : pass.states)
				{
					pass_node.AppendNode(MakeState(st.name, st.value, st.index));
				}
				pass_node.AppendNode(MakeState("vertex_shader", pass.vs_entry + "()"));
				pass_node.AppendNode(MakeState("pixel_shader", pass.ps_entry + "()"));
				tech.AppendNode(std::move(pass_node));
				effect.AppendNode(std::move(tech));
			}
		}
	}

	out_effect = std::move(effect);
	return true;
}
}
