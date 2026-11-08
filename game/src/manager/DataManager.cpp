#include <manager/DataManager.h>
#include <game/GameContext.h>
#include <GameSession.h>

#include <base/ZEngine.h>
#include <common/JsonDom.h>
#include <common/Log.h>

#include <cstdlib>
#include <filesystem>
#include <string>

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
    Gameplay::Protocol::Request request;
    request.mutable_npc_catalog();
    auto response = GameContext::Instance().GameplayInstance().Call(std::move(request));
    if (response.status() != Gameplay::Protocol::ERROR_CODE_OK) return false;
    // C# owns NPC definitions. This is only an ID -> loaded render-asset cache.
    for (auto const& definition : response.npc_catalog())
    {
        auto const* prefab = FindPrefabByPathOrName(prefabs_data_, definition.prefab());
        if (!prefab)
        {
            LogError() << "NPC presentation asset unavailable: " << definition.prefab() << std::endl;
            continue;
        }
        npc_id_to_prefab_.emplace(definition.id(), static_cast<std::size_t>(prefab - prefabs_data_.data()));
        npc_entries_.push_back(NpcConfigEntry{definition.id(), definition.prefab()});
    }
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
