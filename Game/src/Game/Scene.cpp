#include <game/Scene.h>
#include <game/Model.h>
#include <game/NpcSpawner.h>

#include <base/Context.h>
#include <base/Input.h>
#include <common/JsonDom.h>
#include <common/Log.h>
#include <common/Util.h>
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
	while (!models_.empty())
	{
		RemoveModel(models_.back());
	}
	ClearSkyBox();
	ClearTerrain();
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

void AScene::LoadScene(std::string_view scene_path)
{
	while (!models_.empty())
	{
		RemoveModel(models_.back());
	}
	ClearSkyBox();
	ClearTerrain();
	ClearLights();

	auto& res_loader = Context::Instance().ResLoaderInstance();
	ResIdentifierPtr scene_file = res_loader.Open(scene_path);
	if (!scene_file)
	{
		LogError() << "Could NOT open scene file: " << scene_path << std::endl;
		return;
	}

	JsonValue const root = LoadJson(*scene_file);

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
		for (auto const& entry : game_objects->ValueArray())
		{
			if (entry.Type() == JsonValueType::String)
			{
				LoadPrefab(entry.ValueString());
			}
			else if (entry.Type() == JsonValueType::Object)
			{
				LoadGameObject(entry);
			}
		}
	}
}

void AScene::LoadCameraConfig(JsonValue const& root)
{
}

void AScene::SetupCameraController(Camera& camera)
{
	camera_controller_.Scalers(0.05f, 1.5f);
	camera_controller_.AttachCamera(camera);
}

void AScene::SetCameraControllerInputEnabled(bool enabled)
{
	camera_controller_.InputEnabled(enabled);
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

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(
		MathWorker::scaling(0.05f, 0.05f, 0.05f) * light_proxy->RootNode()->TransformToParent());

	light_node_ = MakeSharedPtr<SceneNode>(L"LightNode", SceneNode::SOA_Cullable);
	light_node_->TransformToParent(MathWorker::translation(0.0f, 2.0f, -3.0f));
	light_node_->AddComponent(light_);
	light_node_->AddChild(light_proxy->RootNode());
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
}

void AScene::ClearLights()
{
	auto& root_node = Context::Instance().WorldInstance().SceneRootNode();

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

	light_.reset();
}

void AScene::UpdateDetailedMeshes(float3 const& eye_pos, bool back_face_depth_pass)
{
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
