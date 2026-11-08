#pragma once

#include <game/GameApi.h>
#include <render/Light.h>
#include <render/Camera.h>
#include <render/Mesh.h>
#include <base/ZEngine.h>
#include <base/Input.h>
#include <common/JsonDom.h>
#include <world/CameraController.h>
#include <world/ParticleSystem.h>

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <map>
#include <filesystem>

class GAME_API AScene
{
public:
	AScene();
	~AScene() noexcept;

	void AddModel(const RenderModelPtr& model);
	void RemoveModel(const RenderModelPtr& model);
	RenderModelPtr FindModelForNode(RenderWorker::SceneNode const& node) const;

    struct ParticleEffect {
        RenderWorker::SceneNodePtr node;
        std::shared_ptr<RenderWorker::ParticleSystem> system;
        CommonWorker::JsonValue document {CommonWorker::JsonValueType::Object};
        RenderWorker::ParticleSettings saved_settings;
        RenderWorker::float4x4 saved_transform = RenderWorker::float4x4::Identity();
        std::wstring saved_name;
    };
    RenderWorker::SceneNodePtr AddParticleEffect(RenderWorker::ParticleSettings const& settings,
        std::wstring_view name = L"Particles", RenderWorker::float4x4 const& transform = RenderWorker::float4x4::Identity());
    void RemoveParticleEffect(RenderWorker::SceneNode const* node);
    std::shared_ptr<RenderWorker::ParticleSystem> FindParticleEffect(RenderWorker::SceneNode const* node) const;
    std::vector<ParticleEffect> const& ParticleEffects() const { return particle_effects_; }

	void LoadScene(std::string_view scene_path);
    void SaveScene(std::filesystem::path const& path);
    bool Dirty() const;
    bool ContainsModel(RenderModelPtr const& model) const;
    std::string MaterialAsset(RenderModelPtr const& model, uint32_t slot) const;
    void AssignMaterial(RenderModelPtr const& model, uint32_t slot, std::string const& asset, RenderMaterialPtr const& material);
    void RefreshMaterial(std::string const& asset, RenderMaterialPtr const& material);
	void SetEditorMode(bool enabled);
	bool IsEditorMode() const;
    RenderWorker::CameraPtr const& SceneCamera() const { return scene_camera_; }
    RenderWorker::SceneNodePtr const& SceneCameraNode() const { return camera_node_; }
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
    void ClearParticleEffects();
    void MarkParticlesSaved();

    struct SceneModel {
        RenderModelPtr model;
        uint32_t object_index, model_index;
        RenderWorker::float4x4 saved_transform;
        std::map<uint32_t, std::string> materials, saved_materials;
    };
    CommonWorker::JsonValue scene_document_;
    std::vector<SceneModel> scene_models_;
    std::vector<ParticleEffect> particle_effects_;
    bool particles_changed_ = false;
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
    RenderWorker::float4x4 saved_camera_transform_ = RenderWorker::float4x4::Identity();
    RenderWorker::float4 saved_camera_projection_ {0,0,0,0};

	RenderWorker::FirstPersonController camera_controller_;
	bool camera_move_boost_ { false };
	bool editor_mode_ { false };
};
