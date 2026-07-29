#pragma once

#include <game/GameApi.h>
#include <render/Mesh.h>

#include <cstdint>
#include <vector>

namespace NpcSpawner
{
	// Load npc models, apply materials/effects, set root transform.
	// Does not attach to the scene; caller decides (AddModel / AddToSceneRootHelper).
	GAME_API std::vector<RenderModelPtr> SpawnNpc(
		int32_t npc_id,
		RenderWorker::float3 const& position = RenderWorker::float3(0.0f, 0.0f, 0.0f),
		RenderWorker::float3 const& rotation_deg = RenderWorker::float3(0.0f, 0.0f, 0.0f),
		RenderWorker::float3 const& scale = RenderWorker::float3(1.0f, 1.0f, 1.0f));
}
