#include <Manager/DataManager.h>

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

	int32_t JsonToInt32(CommonWorker::JsonValue const& value, int32_t default_value)
	{
		switch (value.Type())
		{
		case CommonWorker::JsonValueType::Int:
			return static_cast<int32_t>(value.ValueInt());
		case CommonWorker::JsonValueType::UInt:
			return static_cast<int32_t>(value.ValueUInt());
		case CommonWorker::JsonValueType::Float:
			return static_cast<int32_t>(value.ValueFloat());
		case CommonWorker::JsonValueType::String:
		{
			std::string const text(value.ValueString());
			char* end = nullptr;
			long const parsed = std::strtol(text.c_str(), &end, 10);
			if (end && end != text.c_str())
			{
				return static_cast<int32_t>(parsed);
			}
			return default_value;
		}
		default:
			return default_value;
		}
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

bool DataManager::LoadNpcConfig(std::string_view path)
{
	npc_entries_.clear();
	npc_id_to_prefab_.clear();

	if (path.empty())
	{
		return false;
	}

	auto& res_loader = Context::Instance().ResLoaderInstance();
	ResIdentifierPtr config_file = res_loader.Open(path);
	if (!config_file)
	{
		LogError() << "LoadNpcConfig: could NOT open " << path << std::endl;
		return false;
	}

	CommonWorker::JsonValue const root = LoadJson(*config_file);
	if (root.Type() != CommonWorker::JsonValueType::Array)
	{
		LogError() << "LoadNpcConfig: root must be an array: " << path << std::endl;
		return false;
	}

	for (CommonWorker::JsonValue const& item : root.ValueArray())
	{
		if (item.Type() != CommonWorker::JsonValueType::Object)
		{
			LogError() << "LoadNpcConfig: entry must be an object in " << path << std::endl;
			continue;
		}

		CommonWorker::JsonValue const* id_val = item.Member("id");
		CommonWorker::JsonValue const* prefab_val = item.Member("prefab");
		if (!id_val || !prefab_val || prefab_val->Type() != CommonWorker::JsonValueType::String)
		{
			LogError() << "LoadNpcConfig: entry needs id and prefab string in " << path << std::endl;
			continue;
		}

		NpcConfigEntry entry;
		entry.id = JsonToInt32(*id_val, 0);
		entry.prefab = NormalizeAssetPath(std::string(prefab_val->ValueString()));
		if (entry.id == 0 || entry.prefab.empty())
		{
			LogError() << "LoadNpcConfig: invalid id/prefab in " << path << std::endl;
			continue;
		}

		PrefabData const* prefab = FindPrefabByPathOrName(prefabs_data_, entry.prefab);
		if (!prefab)
		{
			LogError() << "LoadNpcConfig: prefab not loaded for id " << entry.id << ": " << entry.prefab
					   << std::endl;
			continue;
		}

		std::size_t const prefab_index = static_cast<std::size_t>(prefab - prefabs_data_.data());
		if (npc_id_to_prefab_.contains(entry.id))
		{
			LogError() << "LoadNpcConfig: duplicate npc id " << entry.id << " in " << path << std::endl;
			continue;
		}

		npc_id_to_prefab_.emplace(entry.id, prefab_index);
		npc_entries_.push_back(std::move(entry));
	}

	return true;
}

bool DataManager::LoadGameConfig()
{
	prefabs_data_.clear();
	npc_entries_.clear();
	npc_id_to_prefab_.clear();

	if (!LoadPrefabs("../../Assets/Prefabs"))
	{
		return false;
	}
	return LoadNpcConfig("Config/npc.json");
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
