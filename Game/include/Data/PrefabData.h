#pragma once

// Runtime shape of a .prefab file; filled by DataManager::LoadPrefabs.

#include <game/GameApi.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// Generic shader uniform value from prefab parameter_values (type resolved at apply time).
struct ShaderParamValue
{
	enum class Form
	{
		Bool,
		Number,
		Vector,
		String,
	};

	Form form = Form::Number;
	bool boolean = false;
	float number = 0.0f;
	std::array<float, 4> comps{};
	int count = 0;
	std::string text;
};

using ShaderParamMap = std::unordered_map<std::string, ShaderParamValue>;

struct MeshTextures
{
	// Workshop 4-map path: color->albedo, normal, mask1, mask2.
	// Unpacked fallbacks: metalnessMask/selfIllumMask/detailMask/translucency.
	std::string albedo;
	std::string metalness_glossiness;
	std::string normal;
	std::string emissive;
	std::string detail;
	std::string detail2;
	std::string detail_mask;
	std::string cubemap;
	std::string translucency;
	std::string mask1;
	std::string mask2;
	std::string diffuse_warp;
	std::string fresnel_warp_color;
	std::string fresnel_warp_rim;
	std::string fresnel_warp_spec;
};

struct MeshPart
{
	// Match key against mesh name (substring, case-insensitive), e.g. "base" / "shoulder".
	std::string name;
	MeshTextures textures;
	ShaderParamMap parameter_values;
};

// Payload of a "model" component: the meshes and materials of one AModel.
struct MeshData
{
	std::string model_path;
	// Shared material ball applied to all mesh parts (UE MIC path or engine name).
	std::string material;
	// Optional RenderEffect / Technique override (e.g. SimpleAlbedoNormal.shader / SimpleAlbedoNormalTech).
	std::string render_effect;
	std::string render_technique;
	MeshTextures textures;
	ShaderParamMap parameter_values;
	// Optional per-mesh-name texture overrides; longer names are matched first.
	std::vector<MeshPart> parts;
};

struct ModelData
{
	// Selects the class the spawner builds; "model" builds an AModel.
	std::string type;
	// Set when `type` is "model"; other types add their own payloads.
	std::optional<MeshData> model;
};

struct PrefabData
{
	std::string name;
	// Asset-relative path, e.g. "Prefabs/Model/chaos_knight.prefab" (matches npc.json).
	std::string path;
	std::vector<ModelData> components;
};

// One row from Config/npc.json: id -> prefab path.
struct NpcConfigEntry
{
	int32_t id = 0;
	std::string prefab;
};
