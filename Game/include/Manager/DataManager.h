#pragma once

#include <game/GameApi.h>
#include <common/JsonDom.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

	struct NpcTextures
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
	};

struct NpcPart
{
	// Match key against mesh name (substring, case-insensitive), e.g. "base" / "shoulder".
	std::string name;
	NpcTextures textures;
};

struct NpcData
{
	int32_t id {0};
	std::string name;
	// One NPC may be assembled from multiple mesh parts (e.g. C/L/U).
	std::vector<std::string> models;
	// Shared material ball applied to all mesh parts (UE MIC path or engine name).
	std::string material;
	// Optional RenderEffect / Technique override (e.g. SimpleAlbedoNormal.shader / SimpleAlbedoNormalTech).
	std::string render_effect;
	std::string render_technique;
	NpcTextures textures;
	// Optional per-mesh-name texture overrides; longer names are matched first.
	std::vector<NpcPart> parts;
};

class GAME_API DataManager
{
public:
	DataManager() = default;
	DataManager(DataManager const&) = delete;
	DataManager& operator=(DataManager const&) = delete;

	bool LoadConfig(std::string_view path);
	CommonWorker::JsonValue const* GetConfig(std::string_view path) const;
	void UnloadConfig(std::string_view path);
	void Clear();

	// Load NPC table from generated NpcConfig (Game/Tool/gen_npc_config.py).
	bool LoadNpcConfig();
	NpcData const* FindNpc(int32_t id) const;
	NpcData const* FindNpcByName(std::string_view name) const;
	std::vector<NpcData> const& GetAllNpcs() const noexcept { return npcs_; }

private:
	std::unordered_map<std::string, CommonWorker::JsonValue> configs_;
	std::vector<NpcData> npcs_;
};
