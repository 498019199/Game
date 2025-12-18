#include <render/Camera.h>
#include <world/SceneNode.h>

namespace RenderWorker
{
Camera::Camera()
{
    ProjParams(PI / 4, 1, 1, 1000);
}

SceneComponentPtr Camera::Clone() const
{
    auto ret = MakeSharedPtr<Camera>();
    ret->look_at_dist_ = look_at_dist_;

    ret->fov_ = fov_;
    ret->aspect_ = aspect_;
    ret->near_plane_ = near_plane_;
    ret->far_plane_ = far_plane_;
    ret->proj_mat_ = proj_mat_;
    ret->inv_proj_mat_ = inv_proj_mat_;
    // ret->proj_mat_wo_adjust_ = proj_mat_wo_adjust_;
    // ret->inv_proj_mat_wo_adjust_ = inv_proj_mat_wo_adjust_;

    // ret->prev_view_mat_ = prev_view_mat_;
    // ret->prev_proj_mat_ = prev_proj_mat_;

    ret->view_proj_mat_ = view_proj_mat_;
    ret->inv_view_proj_mat_ = inv_view_proj_mat_;
    ret->view_proj_mat_dirty_ = view_proj_mat_dirty_;
    // ret->view_proj_mat_wo_adjust_ = view_proj_mat_wo_adjust_;
    // ret->inv_view_proj_mat_wo_adjust_ = inv_view_proj_mat_wo_adjust_;
    // ret->view_proj_mat_wo_adjust_dirty_ = view_proj_mat_wo_adjust_dirty_;
    // ret->camera_dirty_ = camera_dirty_;

    //ret->frustum_ = frustum_;
    //ret->frustum_dirty_ = frustum_dirty_;

    //ret->mode_ = mode_;
    //ret->cur_jitter_index_ = cur_jitter_index_;

    return ret;
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
    return this->BoundSceneNode()->InverseTransformToWorld();
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
    return this->BoundSceneNode()->TransformToWorld();
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
    inv_proj_mat_ = MathWorker::inverse(proj_mat_);
    view_proj_mat_dirty_ = true;
}

void Camera::ProjOrthoParams(float w, float h, float near_plane, float far_plane)
{
    fov_		= 0;
    aspect_		= w / h;
    near_plane_	= near_plane;
    far_plane_	= far_plane;

    proj_mat_ = MathWorker::OrthoLH(w, h, near_plane, far_plane);
    inv_proj_mat_ = MathWorker::inverse(proj_mat_);
    view_proj_mat_dirty_ = true;
}

void Camera::Dirty()
{
    view_proj_mat_dirty_ = true;
}
}