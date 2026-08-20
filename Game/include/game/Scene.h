#pragma once

#include <game/GameApi.h>
#include <render/Light.h>
#include <render/Camera.h>
#include <render/Mesh.h>
#include <base/ZEngine.h>
#include <base/Input.h>
#include <common/JsonDom.h>
#include <world/CameraController.h>

#include <string>
#include <string_view>
#include <vector>
#include <array>

class GAME_API AScene
{
public:
	AScene();
	~AScene() noexcept;

	void AddModel(const RenderModelPtr& model);
	void RemoveModel(const RenderModelPtr& model);
	RenderModelPtr FindModelForNode(RenderWorker::SceneNode const& node) const;

	void LoadScene(std::string_view scene_path);
	void SetEditorMode(bool enabled);
	bool IsEditorMode() const;
	void SetupCameraController(RenderWorker::Camera& camera);
	void SetCameraControllerInputEnabled(bool enabled);
	void SetCameraControllerMoveBoost(bool boost);

	void UpdateDetailedMeshes(RenderWorker::float3 const& eye_pos, bool back_face_depth_pass);

private:
	void LoadPrefab(std::string_view prefab_path);
	void LoadGameObject(CommonWorker::JsonValue const& entry);
	void LoadSkyBox(std::string_view y_cube_path, std::string_view c_cube_path);
	void LoadSkyBoxFromFaces(std::array<std::string, 6> const& face_paths);
	void LoadTerrain(std::string_view height_map_path, std::string_view normal_map_path);
	void LoadCameraConfig(CommonWorker::JsonValue const& root);
	void LoadAmbientLight(CommonWorker::JsonValue const& config);
	void ClearSkyBox();
	void ClearTerrain();
	void ClearCamera();
	void SetupDefaultLights();
	void ClearLights();

	std::vector<RenderModelPtr> models_;
	SceneNodePtr skybox_node_;
	SceneNodePtr terrain_node_;
	TexturePtr skybox_y_cube_;
	TexturePtr skybox_c_cube_;
	RenderWorker::LightSourcePtr ambient_light_;
	RenderWorker::LightSourcePtr light_;
	SceneNodePtr light_node_;
	RenderModelPtr light_proxy_;
	RenderModelPtr ambient_proxy_;

	RenderWorker::CameraPtr scene_camera_;
	SceneNodePtr camera_node_;
	RenderModelPtr camera_proxy_;
	RenderablePtr camera_frustum_;

	RenderWorker::FirstPersonController camera_controller_;
	bool camera_move_boost_ { false };
	bool editor_mode_ { false };
};
