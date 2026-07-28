#include <game/Model.h>

#include <base/App3D.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

using namespace RenderWorker;
using namespace CommonWorker;

DetailedMesh::DetailedMesh(std::wstring_view name)
	: StaticMesh(name)
{
	effect_ = SyncLoadRenderEffect("SimpleAlbedoNormal.shader");
	technique_ = effect_->TechniqueByName("SimpleAlbedoNormalTech");
}

void DetailedMesh::DoBuildMeshInfo(RenderModel const& model)
{
	StaticMesh::DoBuildMeshInfo(model);

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const& caps = re.DeviceCaps();
	depth_texture_support_ = caps.depth_texture_support;

	float3 extinction_coefficient(0.2f, 0.8f, 0.12f);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		extinction_coefficient.x() = MathWorker::srgb_to_linear(extinction_coefficient.x());
		extinction_coefficient.y() = MathWorker::srgb_to_linear(extinction_coefficient.y());
		extinction_coefficient.z() = MathWorker::srgb_to_linear(extinction_coefficient.z());
	}
}

void DetailedMesh::OnRenderBegin()
{
	StaticMesh::OnRenderBegin();

	auto& app = Context::Instance().AppInstance();
	if (auto* mvp = effect_->ParameterByName("worldviewproj"))
	{
		*mvp = model_mat_ * app.ActiveCamera().ViewProjMatrix();
	}
	if (auto* eye_os = effect_->ParameterByName("eye_pos_os"))
	{
		*eye_os = MathWorker::transform_coord(app.ActiveCamera().EyePos(), inv_model_mat_);
	}
	// Force raw albedo once to verify sampling; set to 0 after textures look correct.
	if (auto* dbg = effect_->ParameterByName("debug_albedo_only"))
	{
		*dbg = 0.0f;
	}
}

void DetailedMesh::EyePos(float3 const& eye_pos)
{
	if (auto* p = effect_->ParameterByName("eye_pos"))
	{
		*p = MathWorker::transform_coord(eye_pos, inv_model_mat_);
	}
}

void DetailedMesh::LightPos(float3 const& light_pos)
{
	if (auto* p = effect_->ParameterByName("light_pos"))
	{
		*p = MathWorker::transform_coord(light_pos, inv_model_mat_);
	}
}

void DetailedMesh::LightColor(float3 const& light_color)
{
	if (auto* p = effect_->ParameterByName("light_color"))
	{
		*p = light_color;
	}
}

void DetailedMesh::LightFalloff(float3 const& light_falloff)
{
	if (auto* p = effect_->ParameterByName("light_falloff"))
	{
		*p = light_falloff;
	}
}

void DetailedMesh::BackFaceDepthPass(bool dfdp)
{

}

void DetailedMesh::BackFaceDepthTex(TexturePtr const& tex)
{
	auto& app = Context::Instance().AppInstance();
	Camera const& camera = app.ActiveCamera();
	if (depth_texture_support_)
	{
		float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
		if (auto* p = effect_->ParameterByName("near_q"))
		{
			*p = float2(camera.NearPlane() * q, q);
		}
	}
	if (auto* p = effect_->ParameterByName("far_plane"))
	{
		*p = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
	}
}

void DetailedMesh::SigmaT(float sigma_t)
{
	if (auto* p = effect_->ParameterByName("sigma_t"))
	{
		*p = -sigma_t;
	}
}

void DetailedMesh::MtlThickness(float thickness)
{
	if (auto* p = effect_->ParameterByName("material_thickness"))
	{
		*p = -thickness;
	}
}

AModel::AModel(const SceneNodePtr& root_node)
	: RenderModel(root_node)
{
}

AModel::AModel(std::wstring_view name, uint32_t node_attrib)
	: RenderModel(name, node_attrib)
{
}

void AModel::DoBuildModelInfo()
{
	this->ForEachMesh([this](Renderable& mesh) {
		if (auto* detailed_mesh = dynamic_cast<DetailedMesh*>(&mesh))
		{
			detailed_mesh->BuildMeshInfo(*this);
		}
	});
}

StaticMeshPtr CreateDetailedMesh(std::wstring_view name)
{
	return MakeSharedPtr<DetailedMesh>(name);
}

RenderModelPtr CreateGameModel(std::wstring_view name, uint32_t node_attrib)
{
	return MakeSharedPtr<AModel>(name, node_attrib);
}
