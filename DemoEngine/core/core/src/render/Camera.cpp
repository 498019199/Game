#include <Render/Camera.h>

namespace RenderWorker
{
Camera::Camera()
{
    ProjParams(MathWorker::PI / 4, 1, 1, 1000);
}

const float3& Camera::EyePos() const
{
    const float4x4& inv_view_mat = InverseViewMatrix();
    return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(3));
}

float3 Camera::LookAt() const
{
    return EyePos() + ForwardVec() * LookAtDist();
}

const float3& Camera::RightVec() const
{
    const float4x4& inv_view_mat = InverseViewMatrix();
    return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(0));
}

const float3& Camera::UpVec() const
{
    const float4x4& inv_view_mat = InverseViewMatrix();
    return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(1));
}

const float3& Camera::ForwardVec() const
{
    const float4x4& inv_view_mat = InverseViewMatrix();
    return *reinterpret_cast<float3 const *>(&inv_view_mat.Row(2));
}

const float4x4& Camera::ViewMatrix() const
{
    return InverseTransformToWorld();
}

const float4x4& Camera::ProjMatrix() const
{
    return proj_mat_;
}

const float4x4& Camera::ViewProjMatrix() const
{
    if (view_proj_mat_dirty_)
    {
        view_proj_mat_ = ViewMatrix() * ProjMatrix();
        inv_view_proj_mat_ = InverseProjMatrix() * InverseViewMatrix();
        view_proj_mat_dirty_ = false;
    }
    return view_proj_mat_;
}

const float4x4& Camera::InverseViewMatrix() const
{
    return TransformToWorld();
}

const float4x4& Camera::InverseProjMatrix() const
{
    return inv_proj_mat_;
}

const float4x4& Camera::InverseViewProjMatrix() const
{
    if (view_proj_mat_dirty_)
    {
        view_proj_mat_ = ViewMatrix() * ProjMatrix();
        inv_view_proj_mat_ = InverseProjMatrix() * InverseViewMatrix();
        view_proj_mat_dirty_ = false;
    }
    return inv_view_proj_mat_;
}

void Camera::ProjParams(float fov, float aspect, float near_plane, float far_plane)
{
    fov_		= fov;
    aspect_		= aspect;
    near_plane_	= near_plane;
    far_plane_	= far_plane;

    proj_mat_ = MathWorker::PerspectiveFovLH(fov, aspect, near_plane, far_plane);
    inv_proj_mat_ = MathWorker::Inverse(proj_mat_);
    view_proj_mat_dirty_ = true;
}

void Camera::ProjOrthoParams(float w, float h, float near_plane, float far_plane)
{
    fov_		= 0;
    aspect_		= w / h;
    near_plane_	= near_plane;
    far_plane_	= far_plane;

    proj_mat_ = MathWorker::OrthoLH(w, h, near_plane, far_plane);
    inv_proj_mat_ = MathWorker::Inverse(proj_mat_);
    view_proj_mat_dirty_ = true;
}

void Camera::Dirty()
{
    view_proj_mat_dirty_ = true;
}
}