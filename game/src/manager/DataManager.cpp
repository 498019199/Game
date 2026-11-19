#include <manager/DataManager.h>

#ifdef ZENGINE_ENABLE_MANAGED_GAMEPLAY
#include <game/GameContext.h>
#include <GameSession.h>
#endif

#include <base/ZEngine.h>
#include <common/JsonDom.h>
#include <common/Log.h>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>

namespace
{
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

	PrefabData const* FindPrefabByPathOrName(
		std::vector<PrefabData> const& prefabs,
		std::string_view prefab_path)
	{
		std::string const normalized = NormalizeAssetPath(std::string(prefab_path));
		if (normalized.empty())
		{
			return nullptr;
		}

		for (PrefabData const& prefab : prefabs)
		{
			if (!prefab.path.empty() && NormalizeAssetPath(prefab.path) == normalized)
			{
				return &prefab;
			}
		}

		std::filesystem::path const as_path(normalized);
		std::string const stem = as_path.stem().string();
		for (PrefabData const& prefab : prefabs)
		{
			if (prefab.name == stem)
			{
				return &prefab;
			}
		}
		return nullptr;
	}
} // namespace

bool DataManager::LoadConfig(std::string_view path)
{
	if (path.empty())
	{
		return false;
	}

	std::string const key(path);
	if (configs_.contains(key))
	{
		return true;
	}

	auto& res_loader = Context::Instance().ResLoaderInstance();
	ResIdentifierPtr config_file = res_loader.Open(path);
	if (!config_file)
	{
		LogError() << "Could NOT open config file: " << path << std::endl;
		return false;
	}

	configs_.emplace(key, LoadJson(*config_file));
	return true;
}

CommonWorker::JsonValue const* DataManager::GetConfig(std::string_view path) const
{
	auto const iter = configs_.find(std::string(path));
	if (iter == configs_.end())
	{
		return nullptr;
	}
	return &iter->second;
}

void DataManager::UnloadConfig(std::string_view path)
{
	configs_.erase(std::string(path));
}

void DataManager::Clear()
{
	configs_.clear();
	prefabs_data_.clear();
	npc_entries_.clear();
	npc_id_to_prefab_.clear();
}

bool DataManager::LoadNpcConfig()
{
    npc_entries_.clear();
    npc_id_to_prefab_.clear();

    auto cache_npc = [this](int32_t id, std::string_view prefab_path)
    {
        auto const* prefab = FindPrefabByPathOrName(prefabs_data_, prefab_path);
        if (!prefab)
        {
            LogError() << "NPC presentation asset unavailable: " << prefab_path << std::endl;
            return;
        }
        auto const index = static_cast<std::size_t>(prefab - prefabs_data_.data());
        if (npc_id_to_prefab_.emplace(id, index).second)
            npc_entries_.push_back(NpcConfigEntry{id, std::string(prefab_path)});
    };

#ifdef ZENGINE_ENABLE_MANAGED_GAMEPLAY
    Gameplay::Protocol::Request request;
    request.mutable_npc_catalog();
    auto response = GameContext::Instance().GameplayInstance().Call(std::move(request));
    if (response.status() != Gameplay::Protocol::ERROR_CODE_OK) return false;
    for (auto const& definition : response.npc_catalog())
        cache_npc(definition.id(), definition.prefab());
#else
    using CommonWorker::JsonValueType;
    constexpr std::string_view npc_config = "config/npc.json";
    if (!LoadConfig(npc_config)) return false;

    auto const* catalog = GetConfig(npc_config);
    if (catalog->Type() != JsonValueType::Array) return false;
    for (auto const& entry : catalog->ValueArray())
    {
        if (entry.Type() != JsonValueType::Object) continue;
        auto const* id = entry.Member("id");
        auto const* prefab = entry.Member("prefab");
        if (!id || !prefab || prefab->Type() != JsonValueType::String) continue;

        if (id->Type() == JsonValueType::Int)
            cache_npc(id->ValueInt(), prefab->ValueString());
        else if (id->Type() == JsonValueType::UInt && std::in_range<int32_t>(id->ValueUInt()))
            cache_npc(static_cast<int32_t>(id->ValueUInt()), prefab->ValueString());
    }
#endif
    return true;
}

bool DataManager::LoadGameConfig()
{
	prefabs_data_.clear();
	npc_entries_.clear();
	npc_id_to_prefab_.clear();

	LoadPrefabs("../../assets/private/prefabs");
	if (!LoadPrefabs("../../assets/prefabs"))
	{
		return false;
	}
	return LoadNpcConfig();
}

PrefabData const* DataManager::FindNpc(int32_t id) const
{
	auto const iter = npc_id_to_prefab_.find(id);
	if (iter == npc_id_to_prefab_.end())
	{
		return nullptr;
	}
	if (iter->second >= prefabs_data_.size())
	{
		return nullptr;
	}
	return &prefabs_data_[iter->second];
}

PrefabData const* DataManager::FindNpcByName(std::string_view name) const
{
	for (PrefabData const& npc : prefabs_data_)
	{
		if (npc.name == name)
		{
			return &npc;
		}
	}
	return nullptr;
}
