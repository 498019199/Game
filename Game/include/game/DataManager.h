#pragma once

#include <game/GameApi.h>
#include <common/JsonDom.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct NpcData
{
	int32_t id {0};
	std::string name;
	std::string model;
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
