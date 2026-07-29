#pragma once

#include <game/GameApi.h>
#include <render/Mesh.h>

#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

struct ModelData;
struct PrefabData;

namespace NpcSpawner
{
	// What a component spawner reads and contributes while one npc is built.
	struct SpawnContext
	{
		PrefabData const* npc {nullptr};
		RenderWorker::float4x4 transform;
		// Spawners that build renderables append them here.
		std::vector<RenderModelPtr>* models {nullptr};
	};

	using ComponentSpawner = std::function<void(ModelData const&, SpawnContext const&)>;

	// Binds a component "type" to the class it builds; "model" builds an AModel.
	// Types match case-insensitively, and registering a known type replaces it.
	GAME_API void RegisterComponentSpawner(std::string_view type, ComponentSpawner spawner);

	// Load npc models, apply materials/effects, set root transform.
	// `npc_id` indexes Config/npc.json -> prefab (see DataManager::FindNpc).
	// Does not attach to the scene; caller decides (AddModel / AddToSceneRootHelper).
	GAME_API std::vector<RenderModelPtr> SpawnNpc(
		int32_t npc_id,
		RenderWorker::float3 const& position = RenderWorker::float3(0.0f, 0.0f, 0.0f),
		RenderWorker::float3 const& rotation_deg = RenderWorker::float3(0.0f, 0.0f, 0.0f),
		RenderWorker::float3 const& scale = RenderWorker::float3(1.0f, 1.0f, 1.0f));
}
