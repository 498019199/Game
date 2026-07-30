#include <game/NpcSpawner.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <base/ZEngine.h>
#include <common/Log.h>
#include <common/Util.h>
#include <game/GameContext.h>
#include <game/Model.h>
#include <Manager/DataManager.h>
#include <render/Mesh.h>
#include <render/Renderable.h>
#include <render/RenderEffect.h>
#include <render/RenderMaterial.h>
#include <render/RenderFactory.h>
#include <render/Texture.h>
#include <world/SceneNode.h>

using namespace RenderWorker;
using namespace CommonWorker;

namespace
{
	bool HasAnyTexture(MeshTextures const& textures)
	{
		return !textures.albedo.empty() || !textures.metalness_glossiness.empty() || !textures.normal.empty()
			|| !textures.emissive.empty() || !textures.detail.empty() || !textures.detail2.empty()
			|| !textures.detail_mask.empty() || !textures.cubemap.empty() || !textures.translucency.empty()
			|| !textures.mask1.empty() || !textures.mask2.empty() || !textures.diffuse_warp.empty()
			|| !textures.fresnel_warp_color.empty() || !textures.fresnel_warp_rim.empty()
			|| !textures.fresnel_warp_spec.empty();
	}

std::string ToLowerAscii(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return value;
}

// color.tga / _color.TGA -> sibling map name (metalnessMask, selfIllumMask, ...).
std::string DeriveColorSibling(std::string const& albedo_path, std::string_view suffix)
{
	if (albedo_path.empty() || suffix.empty())
	{
		return {};
	}

	std::string const lower = ToLowerAscii(albedo_path);
	std::string_view const tokens[] = {"_color.", "-color.", "color."};
	for (std::string_view token : tokens)
	{
		auto const pos = lower.rfind(token);
		if (pos == std::string::npos)
		{
			continue;
		}
		std::string out = albedo_path;
		std::string replace(suffix);
		replace.append(token.substr(token.size() - 1)); // keep '.'
		out.replace(pos, token.size(), replace);
		return out;
	}
	return {};
}

std::string ResolveExistingTexturePath(std::string const& configured, std::string const& albedo, std::string_view sibling_suffix)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();
	auto exists = [&](std::string const& path) {
		return !path.empty()
			&& (!res_loader.Locate(path).empty() || !res_loader.Locate(path + ".dds").empty());
	};

	if (exists(configured))
	{
		return configured;
	}
	std::string const derived = DeriveColorSibling(albedo, sibling_suffix);
	if (exists(derived))
	{
		return derived;
	}
	return configured.empty() ? derived : configured;
}

// Optional maps (detail / detail2): missing files are normal — return empty, never a phantom path.
std::string ResolveOptionalTexturePath(std::string const& configured, std::string const& albedo, std::string_view sibling_suffix)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();
	auto exists = [&](std::string const& path) {
		return !path.empty()
			&& (!res_loader.Locate(path).empty() || !res_loader.Locate(path + ".dds").empty());
	};

	if (exists(configured))
	{
		return configured;
	}
	std::string const derived = DeriveColorSibling(albedo, sibling_suffix);
	if (exists(derived))
	{
		return derived;
	}
	return {};
}

	MeshTextures ResolveMeshTextures(MeshTextures const& src)
	{
		MeshTextures out = src;
		out.mask1 = ResolveExistingTexturePath(src.mask1, src.albedo, "mask1");
		out.mask2 = ResolveExistingTexturePath(src.mask2, src.albedo, "mask2");
		out.detail = ResolveOptionalTexturePath(src.detail, src.albedo, "detail");
		out.detail2 = ResolveOptionalTexturePath(src.detail2, src.albedo, "detail2");
		out.cubemap = ResolveExistingTexturePath(src.cubemap, src.albedo, "cubeMap");
		out.diffuse_warp = ResolveExistingTexturePath(src.diffuse_warp, src.albedo, "diffuseWarp");
		out.fresnel_warp_color = ResolveExistingTexturePath(src.fresnel_warp_color, src.albedo, "fresnelWarpColor");
		out.fresnel_warp_rim = ResolveExistingTexturePath(src.fresnel_warp_rim, src.albedo, "fresnelWarpRim");
		out.fresnel_warp_spec = ResolveExistingTexturePath(src.fresnel_warp_spec, src.albedo, "fresnelWarpSpec");
		if (out.normal.empty())
		{
			out.normal = ResolveExistingTexturePath({}, src.albedo, "normal");
		}
		// Packed workshop maps absorb metalness/selfIllum/detailMask/translucency.
		if (out.mask1.empty())
		{
			out.metalness_glossiness =
				ResolveExistingTexturePath(src.metalness_glossiness, src.albedo, "metalnessMask");
			out.emissive = ResolveExistingTexturePath(src.emissive, src.albedo, "selfIllumMask");
			out.detail_mask = ResolveExistingTexturePath(src.detail_mask, src.albedo, "detailMask");
			out.translucency = ResolveExistingTexturePath(src.translucency, src.albedo, "translucency");
		}
		else
		{
			out.metalness_glossiness.clear();
			out.emissive.clear();
			out.detail_mask.clear();
			out.translucency.clear();
		}
		return out;
	}

ShaderResourceViewPtr LoadNpcTextureSrv(std::string const& tex_path)
{
	if (tex_path.empty())
	{
		return {};
	}

	auto& context = Context::Instance();
	if (!context.RenderFactoryValid())
	{
		return {};
	}

	auto& res_loader = context.ResLoaderInstance();
	if (res_loader.Locate(tex_path).empty() && res_loader.Locate(tex_path + ".dds").empty())
	{
		LogError() << "ApplyNpcMaterial: texture not found: " << tex_path << std::endl;
		return {};
	}

	auto& rf = context.RenderFactoryInstance();
	auto tex = SyncLoadTexture(tex_path, EAH_GPU_Read | EAH_Immutable);
	if (!tex)
	{
		LogError() << "ApplyNpcMaterial: SyncLoadTexture failed: " << tex_path << std::endl;
		return {};
	}
	auto srv = rf.MakeTextureSrv(tex);
	if (!srv)
	{
		LogError() << "ApplyNpcMaterial: MakeTextureSrv failed: " << tex_path << std::endl;
	}
	return srv;
}

void BindNpcTextureSlot(RenderMaterial& mtl, RenderMaterial::TextureSlot slot, std::string const& tex_path)
{
	if (tex_path.empty())
	{
		return;
	}

	mtl.TextureName(slot, tex_path);
	if (auto srv = LoadNpcTextureSrv(tex_path))
	{
		mtl.Texture(slot, std::move(srv));
	}
}

void SetEffectTextureParam(RenderEffect& effect, char const* name, std::string const& tex_path, char const* enabled_name)
{
	auto* param = effect.ParameterByName(name);
	auto* enabled = enabled_name ? effect.ParameterByName(enabled_name) : nullptr;
	if (!param)
	{
		return;
	}

	if (auto srv = LoadNpcTextureSrv(tex_path))
	{
		*param = srv;
		if (enabled)
		{
			*enabled = 1;
		}
	}
	else if (enabled)
	{
		*enabled = 0;
	}
}

int ExpectedShaderParamComponents(RenderEffectDataType type)
{
	switch (type)
	{
	case REDT_float2:
	case REDT_int2:
	case REDT_uint2:
		return 2;
	case REDT_float3:
	case REDT_int3:
	case REDT_uint3:
		return 3;
	case REDT_float4:
	case REDT_int4:
	case REDT_uint4:
		return 4;
	default:
		return 1;
	}
}

bool FillShaderParamComponents(ShaderParamValue const& value, std::array<float, 4>& comps, int& count)
{
	count = 0;
	switch (value.form)
	{
	case ShaderParamValue::Form::Bool:
		comps[0] = value.boolean ? 1.0f : 0.0f;
		count = 1;
		return true;
	case ShaderParamValue::Form::Number:
		comps[0] = value.number;
		count = 1;
		return true;
	case ShaderParamValue::Form::Vector:
		count = value.count;
		for (int i = 0; i < value.count && i < 4; ++i)
		{
			comps[static_cast<size_t>(i)] = value.comps[static_cast<size_t>(i)];
		}
		return count > 0;
	case ShaderParamValue::Form::String:
	default:
		return false;
	}
}

bool ApplyShaderParamToEffect(
	RenderEffect& effect, std::string const& name, ShaderParamValue const& value, std::string_view context)
{
	RenderEffectParameter* param = effect.ParameterByName(name);
	if (!param)
	{
		LogInfo() << "ApplyEffectParameterValues: unknown parameter '" << name << "' for " << context << std::endl;
		return false;
	}

	auto scalar_float = [&]() -> float {
		switch (value.form)
		{
		case ShaderParamValue::Form::Bool:
			return value.boolean ? 1.0f : 0.0f;
		case ShaderParamValue::Form::Number:
			return value.number;
		case ShaderParamValue::Form::Vector:
			return value.count > 0 ? value.comps[0] : 0.0f;
		default:
			return 0.0f;
		}
	};

	auto scalar_bool = [&]() -> bool {
		switch (value.form)
		{
		case ShaderParamValue::Form::Bool:
			return value.boolean;
		case ShaderParamValue::Form::Number:
			return value.number != 0.0f;
		case ShaderParamValue::Form::Vector:
			return value.count > 0 && value.comps[0] != 0.0f;
		default:
			return false;
		}
	};

	RenderEffectDataType const type = param->Type();
	int const expect = ExpectedShaderParamComponents(type);
	if (expect > 1)
	{
		std::array<float, 4> comps{};
		int count = 0;
		if (!FillShaderParamComponents(value, comps, count))
		{
			LogError() << "ApplyEffectParameterValues: cannot convert parameter '" << name << "' to vector for "
					   << context << std::endl;
			return false;
		}
		if (count != expect)
		{
			LogInfo() << "ApplyEffectParameterValues: parameter '" << name << "' expected " << expect
					  << " components, got " << count << " — padding/truncating" << std::endl;
		}

		switch (type)
		{
		case REDT_float2:
			*param = float2(comps[0], comps[1]);
			break;
		case REDT_float3:
			*param = float3(comps[0], comps[1], comps[2]);
			break;
		case REDT_float4:
			*param = float4(comps[0], comps[1], comps[2], comps[3]);
			break;
		case REDT_int2:
			*param = int2(static_cast<int32_t>(comps[0]), static_cast<int32_t>(comps[1]));
			break;
		case REDT_int3:
			*param = int3(static_cast<int32_t>(comps[0]), static_cast<int32_t>(comps[1]), static_cast<int32_t>(comps[2]));
			break;
		case REDT_int4:
			*param = int4(
				static_cast<int32_t>(comps[0]), static_cast<int32_t>(comps[1]), static_cast<int32_t>(comps[2]),
				static_cast<int32_t>(comps[3]));
			break;
		case REDT_uint2:
			*param = uint2(static_cast<uint32_t>(comps[0]), static_cast<uint32_t>(comps[1]));
			break;
		case REDT_uint3:
			*param = uint3(
				static_cast<uint32_t>(comps[0]), static_cast<uint32_t>(comps[1]), static_cast<uint32_t>(comps[2]));
			break;
		case REDT_uint4:
			*param = uint4(
				static_cast<uint32_t>(comps[0]), static_cast<uint32_t>(comps[1]), static_cast<uint32_t>(comps[2]),
				static_cast<uint32_t>(comps[3]));
			break;
		default:
			return false;
		}
		return true;
	}

	switch (type)
	{
	case REDT_bool:
		*param = scalar_bool();
		return true;
	case REDT_float:
		*param = scalar_float();
		return true;
	case REDT_int:
		*param = static_cast<int32_t>(scalar_float());
		return true;
	case REDT_uint:
		*param = static_cast<uint32_t>(scalar_float());
		return true;
	case REDT_texture2D:
		if (value.form == ShaderParamValue::Form::String && !value.text.empty())
		{
			if (auto srv = LoadNpcTextureSrv(value.text))
			{
				*param = srv;
				return true;
			}
		}
		LogError() << "ApplyEffectParameterValues: texture parameter '" << name << "' needs a string path for "
				   << context << std::endl;
		return false;
	default:
		LogError() << "ApplyEffectParameterValues: unsupported parameter type for '" << name << "' in " << context
				   << std::endl;
		return false;
	}
}

void ApplyEffectParameterValues(RenderEffect& effect, ShaderParamMap const& values, std::string_view context)
{
	for (auto const& [name, param_value] : values)
	{
		ApplyShaderParamToEffect(effect, name, param_value, context);
	}
}

ShaderParamMap MergeParameterValues(ShaderParamMap const& base, ShaderParamMap const& overrides)
{
	ShaderParamMap merged = base;
	for (auto const& [key, param_value] : overrides)
	{
		merged[key] = param_value;
	}
	return merged;
}

void ApplyTexturesToMaterial(RenderMaterial& mtl, std::string const& material_name, MeshTextures const& textures_in)
{
	MeshTextures const textures = ResolveMeshTextures(textures_in);
	bool const has_textures = HasAnyTexture(textures);
	if (material_name.empty() && !has_textures)
	{
		return;
	}

	if (!material_name.empty())
	{
		mtl.Name(material_name);
	}

	// FBX/MIC often leave diffuse/base color at 0; albedo *= map, so black tint kills all color.
	if (has_textures)
	{
		mtl.Albedo(float4(1.0f, 1.0f, 1.0f, 1.0f));
		if (mtl.Glossiness() <= 0.0f)
		{
			mtl.Glossiness(0.5f);
		}
	}

		BindNpcTextureSlot(mtl, RenderMaterial::TS_Albedo, textures.albedo);
		BindNpcTextureSlot(mtl, RenderMaterial::TS_MetalnessGlossiness, textures.metalness_glossiness);
		if ((!textures.metalness_glossiness.empty() || !textures.mask1.empty()) && mtl.Metalness() <= 0.0f)
		{
			// metalness scales factor.x; keep base at 1 so the mask is visible.
			mtl.Metalness(1.0f);
		}
		BindNpcTextureSlot(mtl, RenderMaterial::TS_Normal, textures.normal);
		BindNpcTextureSlot(mtl, RenderMaterial::TS_Emissive, textures.emissive);
	}

	void ApplyData2ExtraTextures(RenderEffect& effect, MeshTextures const& textures_in)
	{
		MeshTextures const textures = ResolveMeshTextures(textures_in);
		SetEffectTextureParam(effect, "mask1_tex", textures.mask1, "mask1_map_enabled");
		SetEffectTextureParam(effect, "mask2_tex", textures.mask2, "mask2_map_enabled");
		SetEffectTextureParam(effect, "detail_tex", textures.detail, "detail_map_enabled");
		SetEffectTextureParam(effect, "detail2_tex", textures.detail2, "detail2_map_enabled");
		SetEffectTextureParam(effect, "detail_mask_tex", textures.detail_mask, "detail_mask_enabled");
		bool const has_detail_mask = !textures.mask1.empty() || !textures.detail_mask.empty();
		// detail1 requires mask; detail2 can run without mask (dmask=1).
		if (textures.detail.empty() || !has_detail_mask)
		{
			if (auto* enabled = effect.ParameterByName("detail_map_enabled"))
			{
				*enabled = 0;
			}
		}
		if (textures.detail2.empty())
		{
			if (auto* enabled = effect.ParameterByName("detail2_map_enabled"))
			{
				*enabled = 0;
			}
		}
		SetEffectTextureParam(effect, "cubemap_tex", textures.cubemap, "cubemap_map_enabled");
		SetEffectTextureParam(effect, "translucency_tex", textures.translucency, "translucency_map_enabled");
		SetEffectTextureParam(effect, "diffuse_warp_tex", textures.diffuse_warp, "diffuse_warp_enabled");
		SetEffectTextureParam(effect, "fresnel_warp_color_tex", textures.fresnel_warp_color, "fresnel_warp_color_enabled");
		SetEffectTextureParam(effect, "fresnel_warp_rim_tex", textures.fresnel_warp_rim, "fresnel_warp_rim_enabled");
		SetEffectTextureParam(effect, "fresnel_warp_spec_tex", textures.fresnel_warp_spec, "fresnel_warp_spec_enabled");
		if (auto* selfillum = effect.ParameterByName("selfillum_map_enabled"))
		{
			*selfillum = (!textures.mask1.empty() || !textures.emissive.empty()) ? 1 : 0;
		}
	}

MeshPart const* FindPartForMeshName(std::string const& mesh_name_lower, std::vector<MeshPart> const& parts)
{
	for (MeshPart const& part : parts)
	{
		if (part.name.empty())
		{
			continue;
		}
		std::string const key = ToLowerAscii(part.name);
		if (mesh_name_lower.find(key) != std::string::npos)
		{
			return &part;
		}
	}
	return nullptr;
}

void ApplyNpcMaterial(RenderModel& model, MeshData const& data, std::string_view npc_name)
{
	bool const has_parts = !data.parts.empty();
	bool const has_fallback = !data.material.empty() || HasAnyTexture(data.textures);
	if (!has_parts && !has_fallback)
	{
		return;
	}

	// No parts: keep legacy behavior ??paint every material with top-level textures.
	if (!has_parts)
	{
		for (size_t i = 0; i < model.NumMaterials(); ++i)
		{
			RenderMaterialPtr& mtl = model.GetMaterial(static_cast<int32_t>(i));
			if (!mtl)
			{
				continue;
			}
			ApplyTexturesToMaterial(*mtl, data.material, data.textures);
		}
		return;
	}

		std::unordered_set<int32_t> covered_materials;
		for (uint32_t mesh_index = 0; mesh_index < model.NumMeshes(); ++mesh_index)
		{
			auto& mesh = CommonWorker::checked_cast<StaticMesh&>(*model.Mesh(mesh_index));
			std::string mesh_name;
			CommonWorker::Convert(mesh_name, mesh.Name());
			std::string const mesh_name_lower = ToLowerAscii(mesh_name);

			MeshPart const* part = FindPartForMeshName(mesh_name_lower, data.parts);
			if (!part)
			{
				LogInfo() << "ApplyNpcMaterial: unmatched mesh '" << mesh_name << "' for npc " << npc_name
						  << std::endl;
				continue;
			}

			int32_t const mtl_id = mesh.MaterialID();
			if ((mtl_id < 0) || (static_cast<size_t>(mtl_id) >= model.NumMaterials()))
			{
				LogError() << "ApplyNpcMaterial: invalid MaterialID " << mtl_id << " on mesh '" << mesh_name
						   << "'" << std::endl;
				continue;
			}

			RenderMaterialPtr& mtl = model.GetMaterial(mtl_id);
			if (!mtl)
			{
				continue;
			}

			ApplyTexturesToMaterial(*mtl, "", part->textures);
			covered_materials.insert(mtl_id);
		}

		// Uncovered materials: top-level textures, else first part (parts-only NPCs like chaos_knight).
		MeshTextures const* fallback_tex = nullptr;
		std::string fallback_mtl_name;
		if (has_fallback)
		{
			fallback_tex = &data.textures;
			fallback_mtl_name = data.material;
		}
		else if (!data.parts.empty())
		{
			fallback_tex = &data.parts.front().textures;
		}
		if (!fallback_tex)
		{
			return;
		}

		for (size_t i = 0; i < model.NumMaterials(); ++i)
		{
			if (covered_materials.contains(static_cast<int32_t>(i)))
			{
				continue;
			}

			RenderMaterialPtr& mtl = model.GetMaterial(static_cast<int32_t>(i));
			if (!mtl)
			{
				continue;
			}
			ApplyTexturesToMaterial(*mtl, fallback_mtl_name, *fallback_tex);
		}
}

void ApplyNpcRenderEffect(RenderModel& model, MeshData const& data, std::string_view npc_name)
{
	if (data.render_effect.empty())
	{
		return;
	}

	RenderEffectPtr effect_template = SyncLoadRenderEffect(data.render_effect);
	if (!effect_template)
	{
		LogError() << "ApplyNpcRenderEffect: failed to load effect '" << data.render_effect << "' for npc "
				   << npc_name << std::endl;
		return;
	}

	if (data.render_technique.empty())
	{
		LogError() << "ApplyNpcRenderEffect: render_technique is empty for npc " << npc_name << std::endl;
		return;
	}

	if (!effect_template->TechniqueByName(data.render_technique))
	{
		LogError() << "ApplyNpcRenderEffect: technique '" << data.render_technique << "' not found in '"
				   << data.render_effect << "' for npc " << npc_name << std::endl;
		return;
	}

	MeshTextures const* fallback_tex = HasAnyTexture(data.textures) ? &data.textures
		: (!data.parts.empty() ? &data.parts.front().textures : nullptr);

	model.ForEachMesh([&](Renderable& mesh) {
		auto& static_mesh = CommonWorker::checked_cast<StaticMesh&>(mesh);
		RenderEffectPtr mesh_effect = effect_template->Clone();
		RenderTechnique* tech = mesh_effect->TechniqueByName(data.render_technique);
		static_mesh.Technique(mesh_effect, tech);

		MeshTextures const* tex = fallback_tex;
		ShaderParamMap params = data.parameter_values;
		std::string mesh_name;
		CommonWorker::Convert(mesh_name, static_mesh.Name());
		if (MeshPart const* part = FindPartForMeshName(ToLowerAscii(mesh_name), data.parts))
		{
			tex = &part->textures;
			params = MergeParameterValues(params, part->parameter_values);
		}
		if (tex)
		{
			ApplyData2ExtraTextures(*mesh_effect, *tex);
		}
		if (!params.empty())
		{
			ApplyEffectParameterValues(*mesh_effect, params, npc_name);
		}
	});
}

RenderWorker::float4x4 BuildNpcTransform(
	RenderWorker::float3 const& position,
	RenderWorker::float3 const& rotation_deg,
	RenderWorker::float3 const& scale)
{
	using namespace RenderWorker::MathWorker;
	RenderWorker::float4x4 const rot = rotation_matrix_yaw_pitch_roll(
		Deg2Rad(rotation_deg.y()),
		Deg2Rad(rotation_deg.x()),
		Deg2Rad(rotation_deg.z()));
	return translation(position) * rot * scaling(scale);
}

constexpr std::string_view kModelComponentType = "model";

// The class behind "type": "model"; CreateGameModel builds AModel, CreateDetailedMesh its meshes.
void SpawnModelComponent(ModelData const& component, NpcSpawner::SpawnContext const& ctx)
{
	if (!component.model)
	{
		LogError() << "NpcSpawner: component '" << component.type << "' carries no model data" << std::endl;
		return;
	}
	if (!ctx.models)
	{
		LogError() << "NpcSpawner: model component has nowhere to put its models" << std::endl;
		return;
	}

	MeshData const* data = &*component.model;
	if (data->model_path.empty())
	{
		LogError() << "NpcSpawner: model component has no model path" << std::endl;
		return;
	}

	std::string const npc_name = ctx.npc ? ctx.npc->name : std::string();
	float4x4 const transform = ctx.transform;
	RenderModelPtr model = SyncLoadModel(
		data->model_path,
		EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable,
		[data, npc_name, transform](RenderModel& loaded_model)
		{
			ApplyNpcMaterial(loaded_model, *data, npc_name);
			ApplyNpcRenderEffect(loaded_model, *data, npc_name);
			if (!npc_name.empty())
			{
				std::wstring node_name;
				CommonWorker::Convert(node_name, npc_name);
				loaded_model.RootNode()->Name(node_name);
			}
			loaded_model.RootNode()->TransformToParent(transform);
		},
		CreateGameModel,
		CreateDetailedMesh);
	if (!model)
	{
		LogError() << "NpcSpawner: failed to load model: " << data->model_path << std::endl;
		return;
	}
	ctx.models->push_back(std::move(model));
}

using SpawnerMap = std::unordered_map<std::string, NpcSpawner::ComponentSpawner>;

SpawnerMap& ComponentSpawners()
{
	// "model" ships with the engine; other types register their own class on top.
	static SpawnerMap spawners{
		{std::string(kModelComponentType), NpcSpawner::ComponentSpawner(&SpawnModelComponent)},
	};
	return spawners;
}
} // namespace

namespace NpcSpawner
{
void RegisterComponentSpawner(std::string_view type, ComponentSpawner spawner)
{
	if (type.empty() || !spawner)
	{
		LogError() << "NpcSpawner: RegisterComponentSpawner needs a type and a spawner" << std::endl;
		return;
	}
	ComponentSpawners()[ToLowerAscii(std::string(type))] = std::move(spawner);
}

std::vector<RenderModelPtr> SpawnNpc(
	int32_t npc_id,
	float3 const& position,
	float3 const& rotation_deg,
	float3 const& scale)
{
	std::vector<RenderModelPtr> models;
	PrefabData const* npc = GameContext::Instance().DataManagerInstance().FindNpc(npc_id);
	if (!npc)
	{
		LogError() << "NpcSpawner: npc id " << npc_id << " not found" << std::endl;
		return models;
	}
	if (npc->components.empty())
	{
		LogError() << "NpcSpawner: npc " << npc->name << " has no components" << std::endl;
		return models;
	}

	SpawnContext ctx;
	ctx.npc = npc;
	ctx.transform = BuildNpcTransform(position, rotation_deg, scale);
	ctx.models = &models;

	SpawnerMap const& spawners = ComponentSpawners();
	for (ModelData const& component : npc->components)
	{
		auto const iter = spawners.find(ToLowerAscii(component.type));
		if (iter == spawners.end())
		{
			LogError() << "NpcSpawner: no spawner registered for component type '" << component.type
					   << "' on npc " << npc->name << std::endl;
			continue;
		}
		iter->second(component, ctx);
	}
	return models;
}
} // namespace NpcSpawner
