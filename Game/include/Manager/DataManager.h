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
	// Mapped from UE-style packs: DA -> albedo, DCSE -> metalness_glossiness, NR -> normal.
	std::string albedo;
	std::string metalness_glossiness;
	std::string normal;
};

struct NpcData
{
	int32_t id {0};
	std::string name;
	// One NPC may be assembled from multiple mesh parts (e.g. C/L/U).
	std::vector<std::string> models;
	// Shared material ball applied to all mesh parts (UE MIC path or engine name).
	std::string material;
	NpcTextures textures;
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
