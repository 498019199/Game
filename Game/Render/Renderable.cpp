#include "Renderable.h"
#include "../Container/RenderVariable.h"
#include "../Container/ArrayRef.hpp"
#include "../Render/ICamera.h"
#include "../Render/SceneManager.h"
#include "../Platform/DxGraphDevice.h"

Renderable::Renderable(Context* pContext)
	:IEntity(pContext),m_ModleMat(float4x4::Identity())
{
	// 定义数据
	std::string strPath = "GBuffer.fxml";
	auto cvList = SyncLoadRenderEffects(MakeArrayRef(&strPath, 1));
	BindDeferredData(cvList);

	SetPosition(0, 2, -1);
}

void Renderable::RegisterObject(Context* pContext)
{
	pContext->RegisterFactory<SceneManager>();
}

void Renderable::SetPosition(float x, float y, float z)
{
	SetPosition(float3(x, y, z));
}

void Renderable::SetPosition(const float3& pos)
{
	m_Position.x() = pos.x();
	m_Position.y() = pos.y();
	m_Position.z() = pos.z();

	//m_ModleMat *= MathLib::MatrixMove(pos);
}

void Renderable::SetScale(float x, float y, float z)
{
	SetScale(float3(x, y, z));
}

void Renderable::SetScale(const float3& scale)
{
	m_Scale.x() = scale.x();
	m_Scale.y() = scale.y();
	m_Scale.z() = scale.z();

	//m_ModleMat *= MathLib::MatrixScale(scale);
}

void Renderable::SetAngle(const float3& angle)
{
	m_Angle.x() = angle.x();
	m_Angle.y() = angle.y();
	m_Angle.z() = angle.z();

	//m_ModleMat *= MathLib::MatrixMulVector()
}

void Renderable::SetAngle(float x, float y, float z)
{
	SetAngle(float3(x, y, z));
}

void Renderable::BindDeferredData(const RenderCVarlistPtr& cvList)
{
	m_cvList = cvList;

	mvp_param = m_cvList->QueryByName("mvp");
	model_view_param = m_cvList->QueryByName("model_view");
	inv_mv_param = m_cvList->QueryByName("inv_mv");
	pos_center = m_cvList->QueryByName("pos_center");
	albedo_tex_param = m_cvList->QueryByName("albedo_tex");
	metalness_tex_param = m_cvList->QueryByName("metalness_tex");
	glossiness_tex_param = m_cvList->QueryByName("glossiness_tex");
	emissive_tex_param = m_cvList->QueryByName("emissive_tex");
	normal_tex_param = m_cvList->QueryByName("normal_tex");
	height_tex_param = m_cvList->QueryByName("height_tex");
	cull_mode = m_cvList->QueryByName("cull_mode");
}

void Renderable::Render()
{
	auto layout = this->GetRenderLayout();
	if (nullptr == layout)
	{
		this->OnRenderBegin();
		this->OnRenderEnd();
	}
}

void Renderable::OnRenderBegin()
{
	auto pDevice = GetSubsystem<DxGraphDevice>();
	auto var = Context::Instance()->GetGlobalValue("control_move");
	var->Value(m_ModleMat);
	m_ModleMat(3, 0) = m_Position.x();
	m_ModleMat(3, 1) = m_Position.y();
	m_ModleMat(3, 2) = m_Position.z();

	auto camera = GetContext()->ActiveScene()->ActiveCamera();
	auto view = camera->ViewMatrix();
	auto proj = camera->ProjMatrix();
	auto inv_v = camera->InvertViewMatrix();
	auto mv = m_ModleMat * view;
	auto mvp = mv * proj;

	*inv_mv_param = inv_v;
	*mvp_param = mvp;
	*model_view_param = mv;
	*albedo_tex_param = m_Textures[Material::TextureType::TS_Albedo];
	*cull_mode = CULL_FACE_BACK;

	auto layout = this->GetRenderLayout();
	auto cvlist = this->GetRenderEffect();
	GetSubsystem<DxGraphDevice>()->DoRender(m_cvList, layout);
}

void Renderable::OnRenderEnd()
{

}

RenderLayoutPtr Renderable::GetRenderLayout() const
{
	return this->GetRenderLayout();
}

const RenderCVarlistPtr& Renderable::GetRenderEffect() const
{
	return m_cvList;
}

