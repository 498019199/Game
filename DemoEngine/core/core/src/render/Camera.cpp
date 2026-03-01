#include <render/Camera.h>
#include <render/RenderEffect.h>
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







PredefinedCameraCBuffer::PredefinedCameraCBuffer()
{
    effect_ = SyncLoadRenderEffect("PredefinedCBuffers.fxml");
    predefined_cbuffer_ = effect_->CBufferByName("klayge_camera");

    num_cameras_offset_ = effect_->ParameterByName("num_cameras")->CBufferOffset();
    camera_indices_offset_ = effect_->ParameterByName("camera_indices")->CBufferOffset();
    cameras_offset_ = effect_->ParameterByName("cameras")->CBufferOffset();
    prev_mvps_offset_ = effect_->ParameterByName("prev_mvps")->CBufferOffset();

    this->NumCameras(*predefined_cbuffer_) = 1;

    this->CameraIndices(*predefined_cbuffer_, 0) = 0;

    CameraInfo empty = {};
    this->Camera(*predefined_cbuffer_, 0) = empty;

    this->PrevMvp(*predefined_cbuffer_, 0) = float4x4::Identity();
}

uint32_t& PredefinedCameraCBuffer::NumCameras(RenderEffectConstantBuffer& cbuff) const
{
    return *cbuff.template VariableInBuff<uint32_t>(num_cameras_offset_);
}

uint32_t& PredefinedCameraCBuffer::CameraIndices(RenderEffectConstantBuffer& cbuff, uint32_t index) const
{
    return *(cbuff.template VariableInBuff<uint32_t>(camera_indices_offset_) + index);
}

PredefinedCameraCBuffer::CameraInfo& PredefinedCameraCBuffer::Camera(
    RenderEffectConstantBuffer& cbuff, uint32_t index) const
{
    return *(cbuff.template VariableInBuff<CameraInfo>(cameras_offset_) + index);
}

float4x4& PredefinedCameraCBuffer::PrevMvp(
    RenderEffectConstantBuffer& cbuff, uint32_t index) const
{
    return *(cbuff.template VariableInBuff<float4x4>(prev_mvps_offset_) + index);
}
}