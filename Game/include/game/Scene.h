#pragma once

#include <game/GameApi.h>
#include <render/Light.h>
#include <render/Mesh.h>
#include <base/ZEngine.h>
#include <base/Input.h>
#include <common/JsonDom.h>
#include <world/CameraController.h>

#include <string>
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
	void SetupCameraController(RenderWorker::Camera& camera);

	bool HasSceneCamera() const { return has_camera_; }
	RenderWorker::float3 const& CameraEye() const { return camera_eye_; }
	RenderWorker::float3 const& CameraLookAt() const { return camera_look_at_; }
	RenderWorker::float3 const& CameraUp() const { return camera_up_; }
	float CameraNear() const { return camera_near_; }
	float CameraFar() const { return camera_far_; }
	float CameraFovDeg() const { return camera_fov_deg_; }

	void UpdateDetailedMeshes(RenderWorker::float3 const& eye_pos, bool back_face_depth_pass);

private:
	void LoadPrefab(std::string_view prefab_path);
	void LoadSkyBox(std::string_view y_cube_path, std::string_view c_cube_path);
	void LoadTerrain(std::string_view height_map_path, std::string_view normal_map_path);
	void LoadCameraConfig(CommonWorker::JsonValue const& root);
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

	bool has_camera_ { false };
	RenderWorker::float3 camera_eye_ { -0.4f, 1.0f, 3.9f };
	RenderWorker::float3 camera_look_at_ { 0.0f, 1.0f, 0.0f };
	RenderWorker::float3 camera_up_ { 0.0f, 1.0f, 0.0f };
	float camera_near_ { 0.1f };
	float camera_far_ { 200.0f };
	float camera_fov_deg_ { 0.0f };

	bool has_camera_controller_ { false };
	std::string camera_controller_type_;
	uint32_t controller_rotate_button_ { RenderWorker::MB_Left };
	uint32_t controller_zoom_button_ { RenderWorker::MB_Right };
	uint32_t controller_move_button_ { RenderWorker::MB_Middle };
	RenderWorker::ControllerPtr camera_controller_;
};
