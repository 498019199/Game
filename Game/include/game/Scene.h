#pragma once

#include <game/GameApi.h>
#include <render/Light.h>
#include <render/Mesh.h>
#include <base/ZEngine.h>

#include <string_view>
#include <vector>

class GAME_API AScene
{
public:
	AScene();
	~AScene() noexcept;

	void AddModel(const RenderModelPtr& model);
	void RemoveModel(const RenderModelPtr& model);

	void LoadScene(std::string_view scene_path);

	void UpdateDetailedMeshes(RenderWorker::float3 const& eye_pos, bool back_face_depth_pass);

private:
	void LoadPrefab(std::string_view prefab_path);
	void LoadSkyBox(std::string_view y_cube_path, std::string_view c_cube_path);
	void ClearSkyBox();
	void SetupDefaultLights();
	void ClearLights();

	std::vector<RenderModelPtr> models_;
	SceneNodePtr skybox_node_;
	RenderWorker::LightSourcePtr ambient_light_;
	RenderWorker::LightSourcePtr light_;
	SceneNodePtr light_node_;
};
