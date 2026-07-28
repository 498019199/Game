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
		npc.models.reserve(entry.model_count);
		for (std::size_t i = 0; i < entry.model_count; ++i)
		{
			char const* path = entry.models[i];
			npc.models.emplace_back(path ? path : "");
		}
			npc.material = entry.material ? entry.material : "";
			npc.render_effect = entry.render_effect ? entry.render_effect : "";
			npc.render_technique = entry.render_technique ? entry.render_technique : "";
			npc.textures.albedo = entry.textures.albedo ? entry.textures.albedo : "";
			npc.textures.metalness_glossiness =
				entry.textures.metalness_glossiness ? entry.textures.metalness_glossiness : "";
			npc.textures.normal = entry.textures.normal ? entry.textures.normal : "";
			npc.textures.emissive = entry.textures.emissive ? entry.textures.emissive : "";
			npc.textures.detail = entry.textures.detail ? entry.textures.detail : "";
			npc.textures.detail2 = entry.textures.detail2 ? entry.textures.detail2 : "";
			npc.textures.detail_mask = entry.textures.detail_mask ? entry.textures.detail_mask : "";
			npc.textures.cubemap = entry.textures.cubemap ? entry.textures.cubemap : "";
			npc.textures.translucency = entry.textures.translucency ? entry.textures.translucency : "";
			npc.textures.mask1 = entry.textures.mask1 ? entry.textures.mask1 : "";
			npc.textures.mask2 = entry.textures.mask2 ? entry.textures.mask2 : "";
			npc.parts.reserve(entry.part_count);
			for (std::size_t i = 0; i < entry.part_count; ++i)
			{
				NpcConfigPart const& src = entry.parts[i];
				NpcPart part;
				part.name = src.name ? src.name : "";
				part.textures.albedo = src.textures.albedo ? src.textures.albedo : "";
				part.textures.metalness_glossiness =
					src.textures.metalness_glossiness ? src.textures.metalness_glossiness : "";
				part.textures.normal = src.textures.normal ? src.textures.normal : "";
				part.textures.emissive = src.textures.emissive ? src.textures.emissive : "";
				part.textures.detail = src.textures.detail ? src.textures.detail : "";
				part.textures.detail2 = src.textures.detail2 ? src.textures.detail2 : "";
				part.textures.detail_mask = src.textures.detail_mask ? src.textures.detail_mask : "";
				part.textures.cubemap = src.textures.cubemap ? src.textures.cubemap : "";
				part.textures.translucency = src.textures.translucency ? src.textures.translucency : "";
				part.textures.mask1 = src.textures.mask1 ? src.textures.mask1 : "";
				part.textures.mask2 = src.textures.mask2 ? src.textures.mask2 : "";
				npc.parts.push_back(std::move(part));
			}
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
