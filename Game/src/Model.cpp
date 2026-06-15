#include <game/Model.h>

#include <base/App3D.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

using namespace RenderWorker;
using namespace CommonWorker;

DetailedMesh::DetailedMesh(std::wstring_view name)
	: StaticMesh(name)
{
	effect_ = SyncLoadRenderEffect("SubSurface.fxml");
	technique_ = effect_->TechniqueByName("BackFaceDepthTech");
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
	*(effect_->ParameterByName("extinction_coefficient")) = extinction_coefficient;
}

void DetailedMesh::OnRenderBegin()
{
	StaticMesh::OnRenderBegin();

	auto& app = Context::Instance().AppInstance();
	*(effect_->ParameterByName("worldviewproj")) = app.ActiveCamera().ViewProjMatrix();
}

void DetailedMesh::EyePos(float3 const& eye_pos)
{
	*(effect_->ParameterByName("eye_pos")) = eye_pos;
}

void DetailedMesh::LightPos(float3 const& light_pos)
{
	*(effect_->ParameterByName("light_pos")) = light_pos;
}

void DetailedMesh::LightColor(float3 const& light_color)
{
	*(effect_->ParameterByName("light_color")) = light_color;
}

void DetailedMesh::LightFalloff(float3 const& light_falloff)
{
	*(effect_->ParameterByName("light_falloff")) = light_falloff;
}

void DetailedMesh::BackFaceDepthPass(bool dfdp)
{
	if (dfdp)
	{
		if (depth_texture_support_)
		{
			technique_ = effect_->TechniqueByName("BackFaceDepthTech");
		}
		else
		{
			technique_ = effect_->TechniqueByName("BackFaceDepthTechWODepthTexture");
		}
	}
	else
	{
		if (depth_texture_support_)
		{
			technique_ = effect_->TechniqueByName("SubSurfaceTech");
		}
		else
		{
			technique_ = effect_->TechniqueByName("SubSurfaceTechWODepthTexture");
		}
	}
}

void DetailedMesh::BackFaceDepthTex(TexturePtr const& tex)
{
	*(effect_->ParameterByName("back_face_depth_tex")) = tex;

	auto& app = Context::Instance().AppInstance();
	Camera const& camera = app.ActiveCamera();
	if (depth_texture_support_)
	{
		float q = camera.FarPlane() / (camera.FarPlane() - camera.NearPlane());
		*(effect_->ParameterByName("near_q")) = float2(camera.NearPlane() * q, q);
	}
	*(effect_->ParameterByName("far_plane")) = float2(camera.FarPlane(), 1.0f / camera.FarPlane());
}

void DetailedMesh::SigmaT(float sigma_t)
{
	*(effect_->ParameterByName("sigma_t")) = -sigma_t;
}

void DetailedMesh::MtlThickness(float thickness)
{
	*(effect_->ParameterByName("material_thickness")) = -thickness;
}

AModel::AModel(const SceneNodePtr& root_node)
	: RenderModel(root_node)
{
}

AModel::AModel(std::wstring_view name, uint32_t node_attrib)
	: RenderModel(name, node_attrib)
{
}

void AModel::BuildModelInfo()
{
	RenderModel::BuildModelInfo();

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
