#include <game/GameContext.h>
#include <game/Scene.h>
#include <game/SceneAssets.h>
#include <game/SceneParticles.h>
#include <render/ParticleRenderable.h>
#include <render/detail/MaterialFileIO.h>
#include <sstream>
#include <game/Model.h>
#include <game/NpcSpawner.h>

#include <base/Context.h>
#include <base/Input.h>
#include <common/JsonDom.h>
#include <common/Log.h>
#include <common/Util.h>
#include <base/App3D.h>
#include <render/Camera.h>
#include <render/Light.h>
#include <render/RenderFactory.h>
#include <render/RenderMaterial.h>
#include <render/Renderable.h>
#include <render/RenderableHelper.h>
#include <render/SkyBox.h>
#include <render/Texture.h>
#include <world/SceneNode.h>
#include <world/World.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>

namespace
{
	float GetFloat(CommonWorker::JsonValue const& value)
	{
		switch (value.Type())
		{
		case CommonWorker::JsonValueType::Float:
			return value.ValueFloat();
		case CommonWorker::JsonValueType::Int:
			return static_cast<float>(value.ValueInt());
		case CommonWorker::JsonValueType::UInt:
			return static_cast<float>(value.ValueUInt());
		default:
			return 0.0f;
		}
	}

	bool GetBool(CommonWorker::JsonValue const& value, bool default_value)
	{
		if (value.Type() == CommonWorker::JsonValueType::Bool)
		{
			return value.ValueBool();
		}
		return default_value;
	}

	RenderWorker::float3 GetFloat3(CommonWorker::JsonValue const& value, RenderWorker::float3 const& default_value)
	{
		if (value.Type() == CommonWorker::JsonValueType::String)
		{
			// Support "0 0 0" / "0,0,0"
			std::string text(value.ValueString());
			for (char& ch : text)
			{
				if (ch == ',')
				{
					ch = ' ';
				}
			}
			float x = default_value.x();
			float y = default_value.y();
			float z = default_value.z();
#ifdef _MSC_VER
			if (sscanf_s(text.c_str(), "%f %f %f", &x, &y, &z) >= 1)
#else
			if (std::sscanf(text.c_str(), "%f %f %f", &x, &y, &z) >= 1)
#endif
			{
				return RenderWorker::float3(x, y, z);
			}
			return default_value;
		}

		if (value.Type() != CommonWorker::JsonValueType::Array)
		{
			return default_value;
		}

		auto const& arr = value.ValueArray();
		RenderWorker::float3 result = default_value;
		if (arr.size() > 0)
		{
			result.x() = GetFloat(arr[0]);
		}
		if (arr.size() > 1)
		{
			result.y() = GetFloat(arr[1]);
		}
		if (arr.size() > 2)
		{
			result.z() = GetFloat(arr[2]);
		}
		return result;
	}

	int32_t GetInt32(CommonWorker::JsonValue const& value, int32_t default_value)
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

	std::wstring ToWString(std::string_view str)
	{
		std::wstring dest;
		CommonWorker::Convert(dest, str);
		return dest;
	}

	RenderWorker::float4x4 BuildTransformMatrix(
		RenderWorker::float3 const& position,
		RenderWorker::float3 const& rotation_deg,
		RenderWorker::float3 const& scale)
	{
		using namespace RenderWorker::MathWorker;
		RenderWorker::float4x4 const rot = rotation_matrix_yaw_pitch_roll(
			Deg2Rad(rotation_deg.y()),
			Deg2Rad(rotation_deg.x()),
			Deg2Rad(rotation_deg.z()));
		return translation(position) * rot * scaling(scale);
	}

	uint32_t ParseMouseButton(std::string_view name, uint32_t default_value)
	{
		if (name == "Left")
		{
			return RenderWorker::MB_Left;
		}
		if (name == "Right")
		{
			return RenderWorker::MB_Right;
		}
		if (name == "Middle")
		{
			return RenderWorker::MB_Middle;
		}
		return default_value;
	}

	std::string ReadJsonStringMember(CommonWorker::JsonValue const& obj, char const* key)
	{
		if (CommonWorker::JsonValue const* val = obj.Member(key))
		{
			if (val->Type() == CommonWorker::JsonValueType::String)
			{
				return std::string(val->ValueString());
			}
		}
		return {};
	}

	// Face order matches Texture::CubeFaces: +X -X +Y -Y +Z -Z
	bool ReadSkyBoxFacePaths(CommonWorker::JsonValue const& skybox_config, std::array<std::string, 6>& out_paths)
	{
		using CommonWorker::JsonValue;
		using CommonWorker::JsonValueType;

		auto try_assign = [&](JsonValue const& src) {
			struct FaceKey
			{
				char const* primary;
				char const* alias;
				size_t index;
			};
			FaceKey const keys[] = {
				{"Right", "PositiveX", 0},
				{"Left", "NegativeX", 1},
				{"Up", "PositiveY", 2},
				{"Down", "NegativeY", 3},
				{"Front", "PositiveZ", 4},
				{"Back", "NegativeZ", 5},
			};

			bool all = true;
			for (FaceKey const& key : keys)
			{
				std::string path = ReadJsonStringMember(src, key.primary);
				if (path.empty())
				{
					path = ReadJsonStringMember(src, key.alias);
				}
				if (path.empty())
				{
					all = false;
					break;
				}
				out_paths[key.index] = std::move(path);
			}
			return all;
		};

		if (JsonValue const* faces = skybox_config.Member("Faces"))
		{
			if (faces->Type() == JsonValueType::Object && try_assign(*faces))
			{
				return true;
			}
		}

		if (try_assign(skybox_config))
		{
			return true;
		}

		// Directory auto: <dir>/{right,left,up,down,front,back}.png
		std::string directory = ReadJsonStringMember(skybox_config, "Directory");
		if (directory.empty())
		{
			directory = ReadJsonStringMember(skybox_config, "FacesDir");
		}
		if (directory.empty())
		{
			return false;
		}

		while (!directory.empty() && (directory.back() == '/' || directory.back() == '\\'))
		{
			directory.pop_back();
		}

		char const* const names[6] = {"right", "left", "up", "down", "front", "back"};
		for (size_t i = 0; i < 6; ++i)
		{
			out_paths[i] = directory + "/" + names[i] + ".png";
		}
		return true;
	}
}

using namespace RenderWorker;
using namespace CommonWorker;

AScene::AScene() = default;

AScene::~AScene() noexcept
{
    ClearParticleEffects();
	while (!models_.empty())
	{
		RemoveModel(models_.back());
	}
	ClearSkyBox();
	ClearTerrain();
	ClearCamera();
	ClearLights();
}

void AScene::AddModel(RenderModelPtr const& model)
{
	if (!model)
	{
		return;
	}

	models_.push_back(model);
	AddToSceneRootHelper(*model);
}

void AScene::RemoveModel(RenderModelPtr const& model)
{
	auto iter = std::find(models_.begin(), models_.end(), model);
	if (iter == models_.end())
	{
		return;
	}

	if (auto* parent = model->RootNode()->Parent())
	{
		parent->RemoveChild(model->RootNode());
	}

	models_.erase(iter);
}

RenderModelPtr AScene::FindModelForNode(SceneNode const& node) const
{
	SceneNode const* walk = &node;
	SceneNode const& scene_root = Context::Instance().WorldInstance().SceneRootNode();
	while (walk->Parent() && walk->Parent() != &scene_root)
	{
		walk = walk->Parent();
	}

	for (RenderModelPtr const& model : models_)
	{
		if (model && model->RootNode().get() == walk)
		{
			return model;
		}
	}
	return nullptr;
}

void AScene::LoadScene(std::string_view scene_path)
{
    auto& res_loader = Context::Instance().ResLoaderInstance();
    auto scene_file = res_loader.Open(scene_path);
    if (!scene_file) throw std::runtime_error("Could not open scene: " + std::string(scene_path));
    JsonValue root = LoadJson(*scene_file);
    if (root.Type() != JsonValueType::Object) throw std::runtime_error("Scene must be a JSON object");
    auto objects = root.Member("GameObjects");
    if (!objects || objects->Type() != JsonValueType::Array) throw std::runtime_error("Scene must contain GameObjects");
    auto particle_definitions = SceneParticles::Read(root);
    auto assets = std::filesystem::path(res_loader.LocalFolder()) / "../../assets";
    std::map<std::string, RenderMaterialPtr> loaded_materials;
    for (auto const& entry : objects->ValueArray()) {
        for (auto const& ref : SceneAssets::Read(entry)) {
            auto file = SceneAssets::Resolve(assets, ref.asset);
            if (!loaded_materials.contains(ref.asset)) loaded_materials[ref.asset] = LoadRenderMaterialFile(file, true);
        }
        if (entry.Type() == JsonValueType::Object) if (auto transforms = entry.Member("ModelTransforms")) {
            if (transforms->Type() != JsonValueType::Array) throw std::runtime_error("Invalid ModelTransforms");
            for (auto const& item : transforms->ValueArray()) {
                (void)SceneAssets::Index(item.Member("Model"));
                auto matrix = item.Member("Matrix");
                if (!matrix || matrix->Type() != JsonValueType::Array || matrix->ValueArray().size() != 16)
                    throw std::runtime_error("Model transform needs 16 matrix elements");
                for (auto const& v : matrix->ValueArray()) {
                    if ((v.Type() != JsonValueType::Float && v.Type() != JsonValueType::Int && v.Type() != JsonValueType::UInt) || !std::isfinite(GetFloat(v)))
                        throw std::runtime_error("Invalid model matrix value");
                }
            }
        }
    }
    // Validate document and referenced assets before removing the current scene.
    ClearParticleEffects();
    scene_models_.clear();
	while (!models_.empty())
	{
		RemoveModel(models_.back());
	}
	ClearSkyBox();
	ClearTerrain();
	ClearCamera();
	ClearLights();


	JsonValue const* skybox_config = root.Member("SkyBox");
	if (skybox_config && skybox_config->Type() == JsonValueType::Object)
	{
		std::string y_cube_path;
		std::string c_cube_path;

		if (JsonValue const* y_cube_val = skybox_config->Member("YCube"))
		{
			if (y_cube_val->Type() == JsonValueType::String)
			{
				y_cube_path = std::string(y_cube_val->ValueString());
			}
		}
		if (JsonValue const* c_cube_val = skybox_config->Member("CCube"))
		{
			if (c_cube_val->Type() == JsonValueType::String)
			{
				c_cube_path = std::string(c_cube_val->ValueString());
			}
		}

		if (!y_cube_path.empty() && !c_cube_path.empty())
		{
			LoadSkyBox(y_cube_path, c_cube_path);
		}
		else
		{
			std::array<std::string, 6> face_paths {};
			bool const has_faces = ReadSkyBoxFacePaths(*skybox_config, face_paths);
			if (has_faces)
			{
				LoadSkyBoxFromFaces(face_paths);
			}
		}
	}

	JsonValue const* terrain_config = root.Member("Terrain");
	if (terrain_config && terrain_config->Type() == JsonValueType::Object)
	{
		std::string height_map_path;
		std::string normal_map_path;

		if (JsonValue const* height_val = terrain_config->Member("HeightMap"))
		{
			if (height_val->Type() == JsonValueType::String)
			{
				height_map_path = std::string(height_val->ValueString());
			}
		}
		if (JsonValue const* normal_val = terrain_config->Member("NormalMap"))
		{
			if (normal_val->Type() == JsonValueType::String)
			{
				normal_map_path = std::string(normal_val->ValueString());
			}
		}

		if (!height_map_path.empty() && !normal_map_path.empty())
		{
			LoadTerrain(height_map_path, normal_map_path);
		}
	}

	SetupDefaultLights();

	if (JsonValue const* ambient_config = root.Member("AmbientLight"))
	{
		if (ambient_config->Type() == JsonValueType::Object)
		{
			LoadAmbientLight(*ambient_config);
		}
	}

	LoadCameraConfig(root);

	JsonValue const* game_objects = root.Member("GameObjects");
	if (game_objects && game_objects->Type() == JsonValueType::Array)
	{
        uint32_t object_index = 0;
		for (auto const& entry : game_objects->ValueArray())
		{
            size_t const first_model = models_.size();
			if (entry.Type() == JsonValueType::String)
			{
				LoadPrefab(entry.ValueString());
			}
			else if (entry.Type() == JsonValueType::Object)
			{
				LoadGameObject(entry);
			}
            for (size_t i = first_model; i < models_.size(); ++i) {
                SceneModel record {models_[i], object_index, static_cast<uint32_t>(i - first_model), models_[i]->RootNode()->TransformToParent(), {}, {}};
                scene_models_.push_back(std::move(record));
            }
            for (auto const& ref : SceneAssets::Read(entry)) {
                if (ref.model >= models_.size() - first_model) throw std::runtime_error("Scene material model index is out of range");
                AssignMaterial(models_[first_model + ref.model], ref.slot, ref.asset, loaded_materials.at(ref.asset));
            }
            if (entry.Type() == JsonValueType::Object) if (auto transforms = entry.Member("ModelTransforms")) {
                for (auto const& item : transforms->ValueArray()) {
                    uint32_t index = SceneAssets::Index(item.Member("Model"));
                    if (index >= models_.size() - first_model) throw std::runtime_error("Scene transform model index is out of range");
                    float4x4 matrix;
                    auto values = item.Member("Matrix")->ValueArray();
                    for (size_t row = 0; row < 4; ++row) for (size_t col = 0; col < 4; ++col) matrix(row,col) = GetFloat(values[row * 4 + col]);
                    auto node = models_[first_model + index]->RootNode(); node->TransformToParent(matrix); node->UpdateTransforms();
                }
            }
            ++object_index;
		}
	}
    for (auto const& effect : particle_definitions) {
        AddParticleEffect(effect.settings, ToWString(effect.name), effect.transform);
        particle_effects_.back().document = effect.document;
    }
    MarkParticlesSaved();
    scene_document_ = std::move(root);
    for (auto& record : scene_models_) {
        record.saved_materials = record.materials;
        record.saved_transform = record.model->RootNode()->TransformToParent();
    }
}

namespace {
void ReplaceModelMaterial(RenderModelPtr const& model, uint32_t slot, RenderMaterialPtr const& material)
{
    if (!material || slot >= model->NumMaterials()) throw std::runtime_error("Invalid model material slot");
    auto instance = material->Clone();
    model->GetMaterial(static_cast<int32_t>(slot)) = instance;
    model->ForEachMesh([&](Renderable& renderable) {
        if (auto mesh = dynamic_cast<StaticMesh*>(&renderable); mesh && mesh->MaterialID() >= 0 && static_cast<uint32_t>(mesh->MaterialID()) == slot) mesh->Material(instance);
    });
}
}
bool AScene::ContainsModel(RenderModelPtr const& model) const
{
    return std::any_of(scene_models_.begin(), scene_models_.end(), [&](auto const& record) { return record.model == model; });
}
std::string AScene::MaterialAsset(RenderModelPtr const& model, uint32_t slot) const
{
    for (auto const& record : scene_models_) if (record.model == model) {
        auto found = record.materials.find(slot); return found == record.materials.end() ? std::string() : found->second;
    }
    return {};
}
void AScene::AssignMaterial(RenderModelPtr const& model, uint32_t slot, std::string const& asset, RenderMaterialPtr const& material)
{
    auto assets = std::filesystem::path(Context::Instance().ResLoaderInstance().LocalFolder()) / "../../assets";
    (void)SceneAssets::Resolve(assets, asset);
    for (auto& record : scene_models_) if (record.model == model) {
        ReplaceModelMaterial(model, slot, material); record.materials[slot] = asset; return;
    }
    throw std::runtime_error("Select a scene model in Hierarchy before assigning materials");
}
void AScene::RefreshMaterial(std::string const& asset, RenderMaterialPtr const& material)
{
    for (auto& record : scene_models_) for (auto const& [slot, path] : record.materials)
        if (path == asset) ReplaceModelMaterial(record.model, slot, material);
}
bool AScene::Dirty() const
{
    if (particles_changed_) return true;
    for (auto const& effect : particle_effects_) {
        if (!(effect.system->Settings() == effect.saved_settings) || effect.node->Name() != effect.saved_name) return true;
        auto const& transform = effect.node->TransformToParent();
        for (size_t i = 0; i < 16; ++i) if (transform[i] != effect.saved_transform[i]) return true;
    }
    if (scene_camera_ && camera_node_) {
        auto const& current = camera_node_->TransformToParent();
        for (size_t i = 0; i < 16; ++i) if (current[i] != saved_camera_transform_[i]) return true;
        float4 projection(scene_camera_->FOV(), scene_camera_->Aspect(), scene_camera_->NearPlane(), scene_camera_->FarPlane());
        for (size_t i = 0; i < 4; ++i) if (projection[i] != saved_camera_projection_[i]) return true;
    }
    for (auto const& record : scene_models_) {
        if (record.materials != record.saved_materials) return true;
        auto const& current = record.model->RootNode()->TransformToParent();
        for (size_t row = 0; row < 4; ++row) for (size_t col = 0; col < 4; ++col)
            if (current(row,col) != record.saved_transform(row,col)) return true;
    }
    return false;
}
void AScene::SaveScene(std::filesystem::path const& path)
{
    JsonValue document(scene_document_);
    auto objects = document.Member("GameObjects");
    if (!objects) throw std::runtime_error("No scene document loaded");
    // Group all submodels of each scene entry; retain unrelated/unknown JSON data.
    std::map<uint32_t, JsonValue> transforms;
    for (auto const& record : scene_models_) {
        JsonValue entry(objects->ValueArray()[record.object_index]);
        for (auto const& [slot, asset] : record.materials) SceneAssets::Set(entry, record.model_index, slot, asset);
        if (entry.Type() == JsonValueType::String) {
            std::string prefab(entry.ValueString()); entry = JsonValue(JsonValueType::Object);
            entry.AppendValue("Prefab", JsonValue(std::string_view(prefab)));
        }
        objects->ValueIndex(record.object_index, std::move(entry));
        auto [it, inserted] = transforms.try_emplace(record.object_index, JsonValueType::Array);
        (void)inserted;
        JsonValue item(JsonValueType::Object), matrix(JsonValueType::Array);
        auto const& value = record.model->RootNode()->TransformToParent();
        for (size_t row = 0; row < 4; ++row) for (size_t col = 0; col < 4; ++col) {
            if (!std::isfinite(value(row,col))) throw std::runtime_error("Cannot save non-finite model transform");
            matrix.AppendValue(JsonValue(value(row,col)));
        }
        item.AppendValue("Model", JsonValue(record.model_index)); item.AppendValue("Matrix", std::move(matrix));
        it->second.AppendValue(std::move(item));
    }
    for (auto& [index, matrices] : transforms) {
        JsonValue entry(objects->ValueArray()[index]);
        if (auto value = entry.Member("ModelTransforms")) *value = std::move(matrices);
        else entry.AppendValue("ModelTransforms", std::move(matrices));
        objects->ValueIndex(index, std::move(entry));
    }
    if (scene_camera_ && camera_node_) {
        auto old = document.Member("Camera");
        JsonValue config = old && old->Type() == JsonValueType::Object ? *old : JsonValue(JsonValueType::Object);
        auto set = [&](char const* key, JsonValue value) { if (auto field = config.Member(key)) *field = std::move(value); else config.AppendValue(key, std::move(value)); };
        auto vector = [](float3 const& v) { JsonValue a(JsonValueType::Array); for (size_t i=0;i<3;++i) a.AppendValue(JsonValue(v[i])); return a; };
        set("Position", vector(scene_camera_->EyePos()));
        set("LookAt", vector(scene_camera_->EyePos() + scene_camera_->ForwardVec()));
        set("Up", vector(scene_camera_->UpVec()));
        set("FOV", JsonValue(MathWorker::Rad2Deg(scene_camera_->FOV())));
        set("Aspect", JsonValue(scene_camera_->Aspect()));
        set("Near", JsonValue(scene_camera_->NearPlane()));
        set("Far", JsonValue(scene_camera_->FarPlane()));
        if (old) *old = std::move(config); else document.AppendValue("Camera", std::move(config));
    }
    JsonValue effects(JsonValueType::Array);
    for (auto const& effect : particle_effects_) {
        SceneParticles::Definition definition;
        CommonWorker::Convert(definition.name, effect.node->Name());
        definition.transform = effect.node->TransformToParent();
        definition.settings = effect.system->Settings();
        definition.document = effect.document;
        effects.AppendValue(SceneParticles::Write(definition));
    }
    if (auto existing = document.Member("ParticleEffects")) *existing = std::move(effects);
    else if (!particle_effects_.empty()) document.AppendValue("ParticleEffects", std::move(effects));
    std::ostringstream stream; SaveJson(document, stream);
    RenderWorker::Detail::WriteMaterialFileAtomically(path, stream.str());
    scene_document_ = std::move(document);
    MarkParticlesSaved();
    if (scene_camera_ && camera_node_) {
        saved_camera_transform_ = camera_node_->TransformToParent();
        saved_camera_projection_ = float4(scene_camera_->FOV(), scene_camera_->Aspect(), scene_camera_->NearPlane(), scene_camera_->FarPlane());
    }
    for (auto& record : scene_models_) {
        record.saved_materials = record.materials; record.saved_transform = record.model->RootNode()->TransformToParent();
    }
}

SceneNodePtr AScene::AddParticleEffect(ParticleSettings const& settings, std::wstring_view name, float4x4 const& transform)
{
    SceneParticles::ValidateTransform(transform);
    auto system = MakeSharedPtr<ParticleSystem>(settings);
    auto node = MakeSharedPtr<SceneNode>(name, SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
    node->AddComponent(system);
    if (Context::Instance().RenderFactoryValid())
        node->AddComponent(MakeSharedPtr<RenderableComponent>(MakeSharedPtr<ParticleRenderable>(system)));
    node->TransformToParent(transform);
    ParticleEffect effect;
    effect.node = node;
    effect.system = std::move(system);
    particle_effects_.push_back(std::move(effect));
    auto& world = Context::Instance().WorldInstance();
    {
        std::lock_guard lock(world.MutexForUpdate());
        world.SceneRootNode().AddChild(node);
    }
    particles_changed_ = true;
    return node;
}

void AScene::RemoveParticleEffect(SceneNode const* node)
{
    auto found = std::find_if(particle_effects_.begin(), particle_effects_.end(),
        [node](auto const& effect) { return effect.node.get() == node; });
    if (found == particle_effects_.end()) return;
    {
        std::lock_guard lock(Context::Instance().WorldInstance().MutexForUpdate());
        if (auto parent = found->node->Parent()) parent->RemoveChild(found->node);
    }
    particle_effects_.erase(found);
    particles_changed_ = true;
}

std::shared_ptr<ParticleSystem> AScene::FindParticleEffect(SceneNode const* node) const
{
    for (auto const& effect : particle_effects_) if (effect.node.get() == node) return effect.system;
    return {};
}

void AScene::ClearParticleEffects()
{
    if (!particle_effects_.empty()) {
        std::lock_guard lock(Context::Instance().WorldInstance().MutexForUpdate());
        for (auto const& effect : particle_effects_)
            if (auto parent = effect.node->Parent()) parent->RemoveChild(effect.node);
    }
    particle_effects_.clear();
    particles_changed_ = false;
}

void AScene::MarkParticlesSaved()
{
    for (auto& effect : particle_effects_) {
        effect.saved_settings = effect.system->Settings();
        effect.saved_transform = effect.node->TransformToParent();
        effect.saved_name = effect.node->Name();
    }
    particles_changed_ = false;
}

void AScene::SetEditorMode(bool enabled)
{
	editor_mode_ = enabled;
}

bool AScene::IsEditorMode() const
{
	if (editor_mode_)
	{
		return true;
	}
	if (!Context::Instance().AppValid())
	{
		return false;
	}
	return Context::Instance().AppInstance().Name().find("Editor") != std::string::npos;
}

void AScene::LoadCameraConfig(JsonValue const& root)
{
	if (!IsEditorMode())
	{
		return;
	}

	float3 position(0.0f, 1.0f, 0.0f);
	float3 look_at(0.0f, 1.0f, 1.0f);
	float3 up(0.0f, 1.0f, 0.0f);

	if (JsonValue const* camera_config = root.Member("Camera"))
	{
		if (camera_config->Type() == JsonValueType::Object)
		{
			if (JsonValue const* pos_val = camera_config->Member("Position"))
			{
				position = GetFloat3(*pos_val, position);
			}
			if (JsonValue const* look_val = camera_config->Member("LookAt"))
			{
				look_at = GetFloat3(*look_val, look_at);
			}
			if (JsonValue const* up_val = camera_config->Member("Up"))
			{
				up = GetFloat3(*up_val, up);
			}
		}
	}

    float fov = 45.f, aspect = 16.f/9.f, near_plane = .1f, far_plane = 200.f;
    if (auto config = root.Member("Camera"); config && config->Type() == JsonValueType::Object) {
        auto value = [&](char const* key, float fallback) { auto v = config->Member(key); return v ? GetFloat(*v) : fallback; };
        fov = value("FOV", fov); aspect = value("Aspect", aspect);
        near_plane = value("Near", near_plane); far_plane = value("Far", far_plane);
    }
    // Invalid projection data must never introduce NaNs into rendering or picking.
    if (!std::isfinite(fov) || fov < 1.f || fov > 179.f) fov = 45.f;
    if (!std::isfinite(aspect) || aspect < .01f) aspect = 16.f/9.f;
    if (!std::isfinite(near_plane) || near_plane < .001f) near_plane = .1f;
    if (!std::isfinite(far_plane) || far_plane <= near_plane) far_plane = near_plane + 200.f;
	scene_camera_ = MakeSharedPtr<Camera>();
    scene_camera_->ProjParams(MathWorker::Deg2Rad(fov), aspect, near_plane, far_plane);
	camera_node_ = MakeSharedPtr<SceneNode>(
		L"Camera", SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
	camera_node_->AddComponent(scene_camera_);
	camera_node_->TransformToParent(MathWorker::inverse(MathWorker::look_at_lh(position, look_at, up)));

	camera_proxy_ = LoadCameraProxyModel(scene_camera_);
	if (camera_proxy_ && camera_proxy_->RootNode())
	{
		camera_proxy_->RootNode()->TransformToParent(
			MathWorker::scaling(0.15f, 0.15f, 0.15f) * camera_proxy_->RootNode()->TransformToParent());
		camera_node_->AddChild(camera_proxy_->RootNode());
	}
	else
	{
		LogError() << "Could NOT load editor camera proxy: CameraProxy.glb" << std::endl;
	}

    // Editor draws the clipped frustum overlay in its Scene viewport, independently
    // of the render backend. Do not render editor lines into the game image.
    camera_node_->UpdateTransforms();
    saved_camera_transform_ = camera_node_->TransformToParent();
    saved_camera_projection_ = float4(scene_camera_->FOV(), scene_camera_->Aspect(), scene_camera_->NearPlane(), scene_camera_->FarPlane());

	Context::Instance().WorldInstance().SceneRootNode().AddChild(camera_node_);
}

void AScene::SetupCameraController(Camera& camera)
{
	camera_move_boost_ = false;
	camera_controller_.Scalers(0.05f, 1.5f);
	camera_controller_.AttachCamera(camera);
}

void AScene::SetCameraControllerInputEnabled(bool enabled)
{
	camera_controller_.InputEnabled(enabled);
}

void AScene::SetCameraControllerMoveBoost(bool boost)
{
	if (camera_move_boost_ == boost)
	{
		return;
	}

	camera_move_boost_ = boost;
	camera_controller_.Scalers(0.05f, boost ? 1.5f * 5.f : 1.5f);
}

void AScene::LoadSkyBox(std::string_view y_cube_path, std::string_view c_cube_path)
{
	TexturePtr y_cube = SyncLoadTexture(y_cube_path, EAH_GPU_Read | EAH_Immutable);
	TexturePtr c_cube = SyncLoadTexture(c_cube_path, EAH_GPU_Read | EAH_Immutable);
	if (!y_cube || !c_cube)
	{
		LogError() << "Could NOT load skybox textures." << std::endl;
		return;
	}

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube, c_cube);
	skybox_y_cube_ = y_cube;
	skybox_c_cube_ = c_cube;

	skybox_node_ = MakeSharedPtr<SceneNode>(
		MakeSharedPtr<RenderableComponent>(skybox),
		L"SkyBox",
		SceneNode::SOA_NotCastShadow);
	Context::Instance().WorldInstance().SceneRootNode().AddChild(skybox_node_);
}

void AScene::LoadSkyBoxFromFaces(std::array<std::string, 6> const& face_paths)
{
	for (std::string const& path : face_paths)
	{
		if (path.empty())
		{
			LogError() << "SkyBox Faces requires Right/Left/Up/Down/Front/Back (or Directory)." << std::endl;
			return;
		}
	}

	std::array<TexturePtr, 6> face_tex {};
	for (size_t i = 0; i < face_paths.size(); ++i)
	{
		face_tex[i] = SyncLoadTexture(face_paths[i], EAH_GPU_Read | EAH_Immutable);
		if (!face_tex[i])
		{
			LogError() << "Could NOT load skybox face: " << face_paths[i] << std::endl;
			return;
		}
	}

	uint32_t const size = face_tex[0]->Width(0);
	ElementFormat const format = face_tex[0]->Format();
	for (size_t i = 0; i < face_tex.size(); ++i)
	{
		if ((face_tex[i]->Width(0) != size) || (face_tex[i]->Height(0) != size))
		{
			LogError() << "SkyBox face must be square and same size: " << face_paths[i]
					   << " (" << face_tex[i]->Width(0) << "x" << face_tex[i]->Height(0)
					   << ", expected " << size << "x" << size << ")" << std::endl;
			return;
		}
		if (face_tex[i]->Format() != format)
		{
			LogError() << "SkyBox face format mismatch: " << face_paths[i] << std::endl;
			return;
		}
	}

	auto& rf = Context::Instance().RenderFactoryInstance();
	// BC formats (e.g. BC7_SRGB) cannot bind as render target; GPU_Write maps to
	// D3D11_BIND_RENDER_TARGET and fails CreateTexture2D. DEFAULT + SRV is enough for CopySubresourceRegion.
	TexturePtr cube = rf.MakeTextureCube(size, 1, 1, format, 1, 0, EAH_GPU_Read);
	if (!cube)
	{
		LogError() << "Could NOT create skybox cube texture." << std::endl;
		return;
	}

	for (uint32_t face = 0; face < 6; ++face)
	{
		face_tex[face]->CopyToSubTextureCube(
			*cube,
			0,
			static_cast<Texture::CubeFaces>(Texture::CF_Positive_X + face),
			0,
			0,
			0,
			size,
			size,
			0,
			Texture::CF_Positive_X,
			0,
			0,
			0,
			size,
			size,
			TextureFilter::Point);
	}

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CubeMap(cube);
	skybox_y_cube_ = cube;
	skybox_c_cube_.reset();

	skybox_node_ = MakeSharedPtr<SceneNode>(
		MakeSharedPtr<RenderableComponent>(skybox),
		L"SkyBox",
		SceneNode::SOA_NotCastShadow);
	Context::Instance().WorldInstance().SceneRootNode().AddChild(skybox_node_);
}

void AScene::ClearSkyBox()
{
	if (!skybox_node_)
	{
		return;
	}

	if (auto* parent = skybox_node_->Parent())
	{
		parent->RemoveChild(skybox_node_);
	}
	skybox_node_.reset();
	skybox_y_cube_.reset();
	skybox_c_cube_.reset();
}

void AScene::LoadTerrain(std::string_view height_map_path, std::string_view normal_map_path)
{
	TexturePtr height_map = SyncLoadTexture(height_map_path, EAH_GPU_Read | EAH_Immutable);
	TexturePtr normal_map = SyncLoadTexture(normal_map_path, EAH_GPU_Read | EAH_Immutable);
	if (!height_map || !normal_map)
	{
		LogError() << "Could NOT load terrain textures." << std::endl;
		return;
	}

	auto terrain_mesh = MakeSharedPtr<TerrainRenderable>(height_map, normal_map);
	terrain_node_ = MakeSharedPtr<SceneNode>(L"TerrainNode", SceneNode::SOA_Cullable);
	terrain_node_->AddComponent(MakeSharedPtr<RenderableComponent>(terrain_mesh));
	Context::Instance().WorldInstance().SceneRootNode().AddChild(terrain_node_);
}

void AScene::ClearTerrain()
{
	if (!terrain_node_)
	{
		return;
	}

	if (auto* parent = terrain_node_->Parent())
	{
		parent->RemoveChild(terrain_node_);
	}
	terrain_node_.reset();
}

void AScene::LoadGameObject(JsonValue const& entry)
{
	using namespace CommonWorker;
	using namespace RenderWorker;

	int32_t npc_id = 0;
	JsonValue const* npc_id_val = entry.Member("NpcId");
	if (!npc_id_val)
	{
		npc_id_val = entry.Member("npcid");
	}
	if (!npc_id_val)
	{
		npc_id_val = entry.Member("Npcid");
	}
	if (npc_id_val)
	{
		npc_id = GetInt32(*npc_id_val, 0);
	}

	float3 position(0.0f, 0.0f, 0.0f);
	float3 rotation(0.0f, 0.0f, 0.0f);
	float3 scale(1.0f, 1.0f, 1.0f);

	// Prefer Transform component when present; otherwise read root-level fields.
	bool has_transform_component = false;
	if (JsonValue const* components = entry.Member("Components"))
	{
		if (components->Type() == JsonValueType::Array)
		{
			for (auto const& component : components->ValueArray())
			{
				JsonValue const* type_val = component.Member("Type");
				if (!type_val || type_val->Type() != JsonValueType::String)
				{
					continue;
				}
				if (type_val->ValueString() != "Transform")
				{
					continue;
				}
				has_transform_component = true;
				if (JsonValue const* position_val = component.Member("Position"))
				{
					position = GetFloat3(*position_val, position);
				}
				if (JsonValue const* rotation_val = component.Member("Rotation"))
				{
					rotation = GetFloat3(*rotation_val, rotation);
				}
				if (JsonValue const* scale_val = component.Member("Scale"))
				{
					scale = GetFloat3(*scale_val, scale);
				}
			}
		}
	}

	if (!has_transform_component)
	{
		if (JsonValue const* position_val = entry.Member("Position"))
		{
			position = GetFloat3(*position_val, position);
		}
		if (JsonValue const* rotation_val = entry.Member("Rotation"))
		{
			rotation = GetFloat3(*rotation_val, rotation);
		}
		if (JsonValue const* scale_val = entry.Member("Scale"))
		{
			scale = GetFloat3(*scale_val, scale);
		}
	}

	if (npc_id != 0)
	{
		auto models = NpcSpawner::SpawnNpc(npc_id, position, rotation, scale);
		if (models.empty())
		{
			LogError() << "Could NOT spawn npc id " << npc_id << " from scene GameObjects." << std::endl;
			return;
		}
		for (RenderModelPtr const& model : models)
		{
			AddModel(model);
		}
		return;
	}

	if (JsonValue const* prefab_val = entry.Member("Prefab"))
	{
		if (prefab_val->Type() == JsonValueType::String)
		{
			LoadPrefab(prefab_val->ValueString());
			return;
		}
	}

	LogError() << "GameObject entry missing NpcId/Prefab." << std::endl;
}

void AScene::LoadPrefab(std::string_view prefab_path)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();
	ResIdentifierPtr prefab_file = res_loader.Open(prefab_path);
	if (!prefab_file)
	{
		LogError() << "Could NOT open prefab file: " << prefab_path << std::endl;
		return;
	}

	JsonValue const root = LoadJson(*prefab_file);

	std::wstring object_name;
	if (JsonValue const* name_val = root.Member("Name"))
	{
		if (name_val->Type() == JsonValueType::String)
		{
			object_name = ToWString(name_val->ValueString());
		}
	}

	float3 position(0.0f, 0.0f, 0.0f);
	float3 rotation(0.0f, 0.0f, 0.0f);
	float3 scale(1.0f, 1.0f, 1.0f);
	std::string mesh_path;
	std::string material_path;
	bool cast_shadow = true;

	JsonValue const* components = root.Member("Components");
	if (components && components->Type() == JsonValueType::Array)
	{
		for (auto const& component : components->ValueArray())
		{
			JsonValue const* type_val = component.Member("Type");
			if (!type_val || type_val->Type() != JsonValueType::String)
			{
				continue;
			}

			std::string_view const type = type_val->ValueString();
			if (type == "Transform")
			{
				if (JsonValue const* position_val = component.Member("Position"))
				{
					position = GetFloat3(*position_val, position);
				}
				if (JsonValue const* rotation_val = component.Member("Rotation"))
				{
					rotation = GetFloat3(*rotation_val, rotation);
				}
				if (JsonValue const* scale_val = component.Member("Scale"))
				{
					scale = GetFloat3(*scale_val, scale);
				}
			}
			else if (type == "MeshRenderer")
			{
				if (JsonValue const* mesh_val = component.Member("Mesh"))
				{
					if (mesh_val->Type() == JsonValueType::String)
					{
						mesh_path = std::string(mesh_val->ValueString());
					}
				}
				if (JsonValue const* material_val = component.Member("Material"))
				{
					if (material_val->Type() == JsonValueType::String)
					{
						material_path = std::string(material_val->ValueString());
					}
				}
				if (JsonValue const* cast_shadow_val = component.Member("CastShadow"))
				{
					cast_shadow = GetBool(*cast_shadow_val, cast_shadow);
				}
			}
		}
	}

	if (mesh_path.empty())
	{
		return;
	}

	uint32_t node_attrib = SceneNode::SOA_Cullable;
	if (!cast_shadow)
	{
		node_attrib |= SceneNode::SOA_NotCastShadow;
	}

	RenderModelPtr model = SyncLoadModel(
		mesh_path,
		EAH_GPU_Read | EAH_Immutable,
		node_attrib,
		nullptr,
		CreateGameModel,
		CreateDetailedMesh);
	if (!model)
	{
		LogError() << "Could NOT load mesh: " << mesh_path << std::endl;
		return;
	}

	if (!object_name.empty())
	{
		model->RootNode()->Name(object_name);
	}

	model->RootNode()->TransformToParent(BuildTransformMatrix(position, rotation, scale));

	if (!material_path.empty())
	{
		RenderMaterialPtr mtl = SyncLoadRenderMaterial(material_path);
		if (mtl)
		{
			model->ForEachMesh([&](Renderable& mesh) {
				mesh.Material(mtl);
			});
		}
	}

	model->BuildModelInfo();
	AddModel(model);
}

void AScene::SetupDefaultLights()
{
	auto& root_node = Context::Instance().WorldInstance().SceneRootNode();

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1.5f, 1.5f, 1.5f));
	light_->Falloff(float3(1.0f, 0.5f, 0.0f));

	light_node_ = MakeSharedPtr<SceneNode>(L"LightNode", SceneNode::SOA_Cullable);
	light_node_->TransformToParent(MathWorker::translation(0.0f, 2.0f, -3.0f));
	light_node_->AddComponent(light_);

	if (IsEditorMode())
	{
		light_proxy_ = LoadLightSourceProxyModel(light_);
		if (light_proxy_ && light_proxy_->RootNode())
		{
			light_proxy_->RootNode()->TransformToParent(
				MathWorker::scaling(0.05f, 0.05f, 0.05f) * light_proxy_->RootNode()->TransformToParent());
			light_node_->AddChild(light_proxy_->RootNode());
		}
		else
		{
			LogError() << "Could NOT load editor light proxy: PointLightProxy.glb" << std::endl;
		}
	}

	root_node.AddChild(light_node_);
}

void AScene::LoadAmbientLight(JsonValue const& config)
{
	float3 color(0.1f, 0.1f, 0.1f);
	if (JsonValue const* color_val = config.Member("Color"))
	{
		color = GetFloat3(*color_val, color);
	}

	bool enabled = true;
	if (JsonValue const* enabled_val = config.Member("Enabled"))
	{
		enabled = GetBool(*enabled_val, enabled);
	}
	if (!enabled)
	{
		return;
	}

	auto& root_node = Context::Instance().WorldInstance().SceneRootNode();
	if (ambient_light_)
	{
		root_node.RemoveComponent(ambient_light_);
		ambient_light_.reset();
	}

	ambient_light_ = MakeSharedPtr<AmbientLightSource>();
	ambient_light_->Color(color);

	bool use_skybox = true;
	if (JsonValue const* use_skybox_val = config.Member("UseSkyBox"))
	{
		use_skybox = GetBool(*use_skybox_val, use_skybox);
	}
	if (use_skybox)
	{
		if (skybox_y_cube_ && skybox_c_cube_)
		{
			ambient_light_->SkylightTex(skybox_y_cube_, skybox_c_cube_);
		}
		else if (skybox_y_cube_)
		{
			ambient_light_->SkylightTex(skybox_y_cube_);
		}
	}

	root_node.AddComponent(ambient_light_);

	if (IsEditorMode())
	{
		ambient_proxy_ = LoadLightSourceProxyModel(ambient_light_);
		if (ambient_proxy_ && ambient_proxy_->RootNode())
		{
			ambient_proxy_->RootNode()->Name(L"AmbientLight");
			ambient_proxy_->RootNode()->TransformToParent(
				MathWorker::scaling(0.05f, 0.05f, 0.05f) * MathWorker::translation(0.0f, 3.0f, 0.0f));
			root_node.AddChild(ambient_proxy_->RootNode());
		}
		else
		{
			LogError() << "Could NOT load editor light proxy: AmbientLightProxy.glb" << std::endl;
		}
	}
}

void AScene::ClearCamera()
{
	if (camera_node_)
	{
		if (auto* parent = camera_node_->Parent())
		{
			parent->RemoveChild(camera_node_);
		}
		camera_node_.reset();
	}
	camera_proxy_.reset();
	scene_camera_.reset();
}

void AScene::ClearLights()
{
	auto& root_node = Context::Instance().WorldInstance().SceneRootNode();

	if (ambient_proxy_ && ambient_proxy_->RootNode())
	{
		if (auto* parent = ambient_proxy_->RootNode()->Parent())
		{
			parent->RemoveChild(ambient_proxy_->RootNode());
		}
	}
	ambient_proxy_.reset();

	if (ambient_light_)
	{
		root_node.RemoveComponent(ambient_light_);
		ambient_light_.reset();
	}

	if (light_node_)
	{
		if (auto* parent = light_node_->Parent())
		{
			parent->RemoveChild(light_node_);
		}
		light_node_.reset();
	}

	light_proxy_.reset();
	light_.reset();
}

void AScene::UpdateDetailedMeshes(float3 const& eye_pos, bool back_face_depth_pass)
{
	if (back_face_depth_pass)
	{
		GameContext::Instance().TickCombat(Context::Instance().AppInstance().FrameTime());
	}

	float3 const light_pos = light_ ? light_->Position() : float3(0.0f, 0.0f, 0.0f);
	float3 light_color(0.0f, 0.0f, 0.0f);
	float3 const light_falloff = light_ ? light_->Falloff() : float3(1.0f, 0.0f, 0.0f);
	if (light_)
	{
		light_color = light_->Color();
	}

	Context::Instance().WorldInstance().SceneRootNode().Traverse([&](SceneNode& node) {
		node.ForEachComponentOfType<RenderableComponent>([&](RenderableComponent& comp) {
			if (auto* detailed_mesh = dynamic_cast<DetailedMesh*>(&comp.BoundRenderable()))
			{
				detailed_mesh->EyePos(eye_pos);
				detailed_mesh->LightPos(light_pos);
				detailed_mesh->LightColor(light_color);
				detailed_mesh->LightFalloff(light_falloff);
				detailed_mesh->BackFaceDepthPass(back_face_depth_pass);
			}
		});
		return true;
	});
}
