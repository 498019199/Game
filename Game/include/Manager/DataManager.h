#pragma once

#include <game/GameApi.h>
#include <common/JsonDom.h>
#include <Data/PrefabData.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class GAME_API DataManager
{
public:
	DataManager() = default;
	DataManager(DataManager const&) = delete;
	DataManager& operator=(DataManager const&) = delete;

	bool LoadPrefabs(std::string_view path);
	bool LoadNpcConfig(std::string_view path = "Config/npc.json");
	bool LoadGameConfig();

	bool LoadConfig(std::string_view path);
	CommonWorker::JsonValue const* GetConfig(std::string_view path) const;
	void UnloadConfig(std::string_view path);
	void Clear();

	PrefabData const* FindNpc(int32_t id) const;
	PrefabData const* FindNpcByName(std::string_view name) const;
	std::vector<PrefabData> const& GetAllPrefabs() const noexcept { return prefabs_data_; }
	std::vector<NpcConfigEntry> const& GetAllNpcs() const noexcept { return npc_entries_; }

private:
	std::unordered_map<std::string, CommonWorker::JsonValue> configs_;

	std::vector<PrefabData> prefabs_data_;
	std::vector<NpcConfigEntry> npc_entries_;
	// npc id -> index into prefabs_data_
	std::unordered_map<int32_t, std::size_t> npc_id_to_prefab_;
};
