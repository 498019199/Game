#include "ICamera.h"
ICamera::ICamera(Context* pContext)
	:IEntity(pContext),m_nState(0), m_nAttr(0),
	m_fViewDist(0), m_fFov(0),
	m_fViewPlaneWidth(0), m_fViewPlaneHeight(0),
	m_fViewPortWidth(0), m_fViewPortHeight(0)
{
	ViewParams(float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1));
	ProjParams(MathLib::PI / 2, 1, 1000, 1);
}

ICamera::~ICamera()
{

}

void ICamera::RegisterObject(Context* pContext)
{
	pContext->RegisterFactory<ICamera>();
}

bool ICamera::OnInit()
{
	return true;
}

bool ICamera::OnShut()
{
	return true;
}

const float4x4& ICamera::ViewMatrix() const
{
	return m_ViewMat;
}

const float4x4& ICamera::ProjMatrix() const
{
	return m_ProjMat;
}

const float4x4& ICamera::InvertViewMatrix() const
{
	return m_InvViewMat;
}

const float4x4& ICamera::InvertProjMatrix() const
{
	return m_InvProjMat;
}

void ICamera::ViewParams(const float3& eye_pos, const float3& look_at, const float3& up_vec)
{
	m_fViewDist = MathLib::Length(look_at - eye_pos);
	m_ViewMat = MathLib::LookAtLH(eye_pos, look_at, up_vec);
	m_InvViewMat = MathLib::Inverse(m_ViewMat);
}

void ICamera::ProjParams(float fFov, float fNearClip, float fFarClip, float fAspectRatio)
{
	m_fFov = fFov;
	m_fNearClipZ = fNearClip;
	m_fFarClipZ = fFarClip;
	m_fAspectRatio = fAspectRatio;

	m_ProjMat = MathLib::PerspectiveFovLH(m_fFov, m_fNearClipZ, m_fFarClipZ, m_fAspectRatio);
	m_InvProjMat = MathLib::Inverse(m_InvViewMat);
}

void ICamera::ProjParams(float fFov, float fNearClip, float fFarClip, int nWidth, int nHeight)
{
	float fAspectRatio = float(nWidth) / (float(nHeight));

	m_fViewPlaneWidth = 2.0f;
	m_fViewPortHeight = m_fViewPlaneWidth * fAspectRatio;
	float tan_fov_div2 = tan(fFov * 0.5f);
	m_fViewDist = 0.5f * m_fViewPlaneWidth * tan_fov_div2;

	ProjParams(fFov, fNearClip, fFarClip, fAspectRatio);
}
