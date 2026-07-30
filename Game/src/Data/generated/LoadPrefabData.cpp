#include <Manager/DataManager.h>

#include <base/ZEngine.h>
#include <common/JsonDom.h>
#include <common/Log.h>
#include <common/ResIdentifier.h>
#include <common/Util.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

using namespace CommonWorker;

namespace
{
	std::string JsonAsString(JsonValue const& value)
	{
		if (value.Type() == JsonValueType::String)
		{
			return std::string(value.ValueString());
		}
		return {};
	}

	std::string NormalizeAssetPath(std::string path)
	{
		for (char& ch : path)
		{
			if (ch == '\\')
			{
				ch = '/';
			}
		}
		while (!path.empty() && path.front() == '/')
		{
			path.erase(path.begin());
		}
		return path;
	}

	JsonValue const* FirstMember(JsonValue const& object, std::initializer_list<char const*> names)
	{
		for (char const* name : names)
		{
			if (JsonValue const* member = object.Member(name))
			{
				return member;
			}
		}
		return nullptr;
	}

	std::string MemberString(JsonValue const& object, std::initializer_list<char const*> names)
	{
		JsonValue const* member = FirstMember(object, names);
		return member ? JsonAsString(*member) : std::string{};
	}

	std::string FirstPathString(JsonValue const& value)
	{
		if (value.Type() == JsonValueType::String)
		{
			return JsonAsString(value);
		}
		if (value.Type() == JsonValueType::Array)
		{
			for (JsonValue const& item : value.ValueArray())
			{
				std::string path = JsonAsString(item);
				if (!path.empty())
				{
					return path;
				}
			}
		}
		return {};
	}

	std::string ModelPathFromComponent(JsonValue const& component)
	{
		for (char const* key : {"path", "model", "models", "paths"})
		{
			if (JsonValue const* member = component.Member(key))
			{
				std::string path = FirstPathString(*member);
				if (!path.empty())
				{
					return path;
				}
			}
		}
		return {};
	}

	MeshTextures LoadTextures(JsonValue const& container)
	{
		JsonValue const* textures_node = container.Member("textures");
		JsonValue const& src = textures_node ? *textures_node : container;

		MeshTextures textures;
		if (src.Type() != JsonValueType::Object)
		{
			return textures;
		}

		auto pick = [&](std::initializer_list<char const*> names) {
			return MemberString(src, names);
		};

		textures.albedo = pick({"albedo", "color", "da", "DA"});
		textures.metalness_glossiness = pick({"metalness_glossiness", "metalnessMask", "dcse", "DCSE"});
		textures.normal = pick({"normal", "nr", "NR"});
		textures.emissive = pick({"emissive", "selfIllumMask"});
		textures.detail = pick({"detail"});
		textures.detail2 = pick({"detail2"});
		textures.detail_mask = pick({"detail_mask", "detailMask"});
		textures.cubemap = pick({"cubemap", "cubeMap"});
		textures.translucency = pick({"translucency"});
		textures.mask1 = pick({"mask1"});
		textures.mask2 = pick({"mask2"});
		textures.diffuse_warp = pick({"diffuse_warp", "diffuseWarp"});
		textures.fresnel_warp_color = pick({"fresnel_warp_color", "fresnelWarpColor"});
		textures.fresnel_warp_rim = pick({"fresnel_warp_rim", "fresnelWarpRim"});
		textures.fresnel_warp_spec = pick({"fresnel_warp_spec", "fresnelWarpSpec"});
		return textures;
	}

	bool ParseCommaSeparatedFloats(std::string_view text, std::array<float, 4>& comps, int& count)
	{
		count = 0;
		std::string buffer(text);
		std::replace(buffer.begin(), buffer.end(), ',', ' ');
		std::istringstream stream(buffer);
		float value = 0.0f;
		while (count < 4 && (stream >> value))
		{
			comps[static_cast<size_t>(count)] = value;
			++count;
		}
		return count > 0;
	}

	bool ParseUeVectorString(std::string_view text, std::array<float, 4>& comps, int& count)
	{
		count = 0;
		std::string buffer(text);
		for (char& ch : buffer)
		{
			if (ch == '(' || ch == ')' || ch == 'R' || ch == 'G' || ch == 'B' || ch == 'A' || ch == '=')
			{
				ch = ' ';
			}
		}
		return ParseCommaSeparatedFloats(buffer, comps, count);
	}

	bool ParseStringAsVector(std::string_view text, std::array<float, 4>& comps, int& count)
	{
		if (text.find('(') != std::string_view::npos || text.find('=') != std::string_view::npos)
		{
			return ParseUeVectorString(text, comps, count);
		}
		return ParseCommaSeparatedFloats(text, comps, count);
	}

	float JsonNumber(JsonValue const& value)
	{
		switch (value.Type())
		{
		case JsonValueType::Bool:
			return value.ValueBool() ? 1.0f : 0.0f;
		case JsonValueType::Int:
			return static_cast<float>(value.ValueInt());
		case JsonValueType::UInt:
			return static_cast<float>(value.ValueUInt());
		case JsonValueType::Float:
			return value.ValueFloat();
		default:
			return 0.0f;
		}
	}

	ShaderParamValue LoadShaderParamValue(JsonValue const& value)
	{
		ShaderParamValue out;
		switch (value.Type())
		{
		case JsonValueType::Bool:
			out.form = ShaderParamValue::Form::Bool;
			out.boolean = value.ValueBool();
			out.number = out.boolean ? 1.0f : 0.0f;
			break;
		case JsonValueType::Int:
		case JsonValueType::UInt:
		case JsonValueType::Float:
			out.form = ShaderParamValue::Form::Number;
			out.number = JsonNumber(value);
			break;
		case JsonValueType::String:
		{
			std::string const text = JsonAsString(value);
			if (ParseStringAsVector(text, out.comps, out.count))
			{
				out.form = ShaderParamValue::Form::Vector;
			}
			else
			{
				out.form = ShaderParamValue::Form::String;
				out.text = text;
			}
			break;
		}
		case JsonValueType::Array:
		{
			out.form = ShaderParamValue::Form::Vector;
			for (JsonValue const& item : value.ValueArray())
			{
				if (out.count >= 4)
				{
					break;
				}
				out.comps[static_cast<size_t>(out.count)] = JsonNumber(item);
				++out.count;
			}
			break;
		}
		default:
			out.form = ShaderParamValue::Form::String;
			break;
		}
		return out;
	}

	ShaderParamMap LoadParameterValues(JsonValue const& container)
	{
		ShaderParamMap values;
		JsonValue const* node = container.Member("parameter_values");
		if (!node || node->Type() != JsonValueType::Object)
		{
			return values;
		}

		for (auto const& [name, param_value] : node->ValueObject())
		{
			if (name.empty())
			{
				continue;
			}
			values.emplace(name, LoadShaderParamValue(param_value));
		}
		return values;
	}

	std::vector<MeshPart> LoadParts(JsonValue const& component)
	{
		JsonValue const* parts_node = component.Member("parts");
		if (!parts_node || parts_node->Type() != JsonValueType::Object)
		{
			return {};
		}

		std::vector<MeshPart> parts;
		parts.reserve(parts_node->ValueObject().size());
		for (auto const& [part_name, part_value] : parts_node->ValueObject())
		{
			if (part_name.empty() || part_value.Type() != JsonValueType::Object)
			{
				continue;
			}

			MeshPart part;
			part.name = part_name;
			part.textures = LoadTextures(part_value);
			part.parameter_values = LoadParameterValues(part_value);
			parts.push_back(std::move(part));
		}

		// Longer keys first so "shoulder" wins over a shorter substring match.
		std::sort(parts.begin(), parts.end(), [](MeshPart const& lhs, MeshPart const& rhs) {
			return lhs.name.size() > rhs.name.size();
		});
		return parts;
	}

	MeshData LoadModelComponent(JsonValue const& component)
	{
		MeshData model;
		model.model_path = ModelPathFromComponent(component);
		model.material = MemberString(component, {"material"});
		model.render_effect = MemberString(component, {"render_effect", "RenderEffect"});
		model.render_technique = MemberString(component, {"render_technique", "RenderTechnique"});
		model.textures = LoadTextures(component);
		model.parameter_values = LoadParameterValues(component);
		model.parts = LoadParts(component);
		return model;
	}

	bool ParsePrefab(JsonValue const& root, PrefabData& out, std::filesystem::path const& file_path)
	{
		if (root.Type() != JsonValueType::Object)
		{
			LogError() << "LoadPrefabs: root must be an object: " << file_path.generic_string() << std::endl;
			return false;
		}

		out.name = MemberString(root, {"name", "Name"});
		if (out.name.empty())
		{
			out.name = file_path.stem().string();
		}

		JsonValue const* components = FirstMember(root, {"components", "Components"});
		if (!components)
		{
			// Flat prefab without components: treat the whole object as one model.
			ModelData component;
			component.type = "model";
			component.model = LoadModelComponent(root);
			if (component.model->model_path.empty())
			{
				LogError() << "LoadPrefabs: no model path in " << file_path.generic_string() << std::endl;
				return false;
			}
			out.components.push_back(std::move(component));
			return true;
		}

		if (components->Type() != JsonValueType::Array)
		{
			LogError() << "LoadPrefabs: components must be an array: " << file_path.generic_string() << std::endl;
			return false;
		}

		for (JsonValue const& component_node : components->ValueArray())
		{
			if (component_node.Type() != JsonValueType::Object)
			{
				LogError() << "LoadPrefabs: component must be an object: " << file_path.generic_string() << std::endl;
				continue;
			}

			ModelData component;
			component.type = MemberString(component_node, {"type", "Type"});
			if (component.type.empty())
			{
				LogError() << "LoadPrefabs: component missing type: " << file_path.generic_string() << std::endl;
				continue;
			}

			std::string type_lower = component.type;
			std::transform(type_lower.begin(), type_lower.end(), type_lower.begin(), [](unsigned char ch) {
				return static_cast<char>(std::tolower(ch));
			});

			if (type_lower == "model")
			{
				component.model = LoadModelComponent(component_node);
				if (component.model->model_path.empty())
				{
					LogError() << "LoadPrefabs: model component missing path in "
							   << file_path.generic_string() << std::endl;
					continue;
				}
			}
			else
			{
				LogInfo() << "LoadPrefabs: component type '" << component.type
						  << "' has no payload loader in " << file_path.generic_string() << std::endl;
			}

			out.components.push_back(std::move(component));
		}

		return !out.components.empty();
	}

	bool LoadPrefabFile(std::filesystem::path const& file_path, PrefabData& out)
	{
		std::string const abs_file = file_path.generic_string();
		auto stream = MakeSharedPtr<std::ifstream>(abs_file.c_str(), std::ios_base::binary);
		if (!stream || !stream->is_open())
		{
			LogError() << "LoadPrefabs: failed to open " << abs_file << std::endl;
			return false;
		}

		uint64_t timestamp = 0;
		std::error_code ec;
		auto const write_time = std::filesystem::last_write_time(file_path, ec);
		if (!ec)
		{
			timestamp = static_cast<uint64_t>(write_time.time_since_epoch().count());
		}

		ResIdentifier res(abs_file, timestamp, stream);
		JsonValue const root = LoadJson(res);
		return ParsePrefab(root, out, file_path);
	}
} // namespace

bool DataManager::LoadPrefabs(std::string_view path)
{
	if (path.empty())
	{
		return false;
	}

	// Caller passes a path relative to the exe / Assets mounts (e.g. "../../Assets/Prefabs").
	auto& res_loader = Context::Instance().ResLoaderInstance();
	std::string const root_abs = res_loader.AbsPath(path);
	if (root_abs.empty())
	{
		LogError() << "LoadPrefabs: path not found: " << path << std::endl;
		return false;
	}

	std::error_code ec;
	std::filesystem::path const root(root_abs);
	if (!std::filesystem::is_directory(root, ec))
	{
		LogError() << "LoadPrefabs: not a directory: " << root_abs << std::endl;
		return false;
	}

	for (auto const& entry : std::filesystem::recursive_directory_iterator(root, ec))
	{
		if (ec)
		{
			LogError() << "LoadPrefabs: failed to iterate " << root_abs << ": " << ec.message() << std::endl;
			return false;
		}
		if (!entry.is_regular_file(ec))
		{
			continue;
		}

		std::string ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});
		if (ext != ".prefab")
		{
			continue;
		}

		PrefabData prefab;
		if (!LoadPrefabFile(entry.path(), prefab))
		{
			continue;
		}

		// Prefer path relative to Assets so it matches npc.json ("Prefabs/Model/xxx.prefab").
		std::error_code rel_ec;
		std::filesystem::path const assets_root = root.parent_path();
		std::filesystem::path const rel = std::filesystem::relative(entry.path(), assets_root, rel_ec);
		if (!rel_ec)
		{
			prefab.path = NormalizeAssetPath(rel.generic_string());
		}
		else
		{
			prefab.path = NormalizeAssetPath(entry.path().filename().generic_string());
		}

		prefabs_data_.push_back(std::move(prefab));
	}

	return true;
}
