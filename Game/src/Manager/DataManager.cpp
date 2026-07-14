#include <Manager/DataManager.h>
#include <Data/generated/NpcConfig.gen.h>

#include <base/ZEngine.h>
#include <common/Log.h>

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
	npcs_.clear();
}

bool DataManager::LoadNpcConfig()
{
	npcs_.clear();
	npcs_.reserve(NpcConfig::Count());

	for (NpcConfigEntry const& entry : NpcConfig::All())
	{
		NpcData npc;
		npc.id = entry.id;
		npc.name = entry.name ? entry.name : "";
		npc.model = entry.model ? entry.model : "";
		npcs_.push_back(std::move(npc));
	}

	return true;
}

NpcData const* DataManager::FindNpc(int32_t id) const
{
	for (NpcData const& npc : npcs_)
	{
		if (npc.id == id)
		{
			return &npc;
		}
	}
	return nullptr;
}

NpcData const* DataManager::FindNpcByName(std::string_view name) const
{
	for (NpcData const& npc : npcs_)
	{
		if (npc.name == name)
		{
			return &npc;
		}
	}
	return nullptr;
}
