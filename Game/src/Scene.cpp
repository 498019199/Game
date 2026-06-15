#include <game/Scene.h>
#include <game/Model.h>

#include <common/JsonDom.h>
#include <common/Log.h>
#include <common/Util.h>
#include <render/RenderMaterial.h>
#include <render/Renderable.h>
#include <render/SkyBox.h>
#include <render/Texture.h>
#include <world/SceneNode.h>
#include <world/World.h>

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float DegToRad(float deg)
	{
		return deg * 3.14159265358979323846f / 180.0f;
	}

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

	MathWorker::float3 GetFloat3(CommonWorker::JsonValue const& value, MathWorker::float3 const& default_value)
	{
		if (value.Type() != CommonWorker::JsonValueType::Array)
		{
			return default_value;
		}

		auto const& arr = value.ValueArray();
		MathWorker::float3 result = default_value;
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

	std::wstring ToWString(std::string_view str)
	{
		std::wstring dest;
		CommonWorker::Convert(dest, str);
		return dest;
	}

	MathWorker::float4x4 BuildTransformMatrix(
		MathWorker::float3 const& position,
		MathWorker::float3 const& rotation_deg,
		MathWorker::float3 const& scale)
	{
		using namespace MathWorker;
		float4x4 const rot = rotation_matrix_yaw_pitch_roll(
			DegToRad(rotation_deg.y()),
			DegToRad(rotation_deg.x()),
			DegToRad(rotation_deg.z()));
		return translation(position) * rot * scaling(scale);
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
	}

	JsonValue const* game_objects = root.Member("GameObjects");
	if (!game_objects || game_objects->Type() != JsonValueType::Array)
	{
		return;
	}

	for (auto const& entry : game_objects->ValueArray())
	{
		if (entry.Type() == JsonValueType::String)
		{
			LoadPrefab(entry.ValueString());
		}
	}
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

	MathWorker::float3 position(0.0f, 0.0f, 0.0f);
	MathWorker::float3 rotation(0.0f, 0.0f, 0.0f);
	MathWorker::float3 scale(1.0f, 1.0f, 1.0f);
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
