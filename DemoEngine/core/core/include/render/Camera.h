#pragma once

#include <world/SceneComponent.h>

namespace RenderWorker
{
class RenderEffectConstantBuffer;
class RenderEffect;
using RenderEffectPtr = std::shared_ptr<RenderEffect>;

class ZENGINE_CORE_API Camera: public SceneComponent
{
public:
    NANO_RTTI_REGISTER_RUNTIME_CLASS(SceneComponent)
    Camera();
    
    SceneComponentPtr Clone() const override;
    
    const float3& EyePos() const;
    float3 LookAt() const;
    const float3& RightVec() const;
    const float3& UpVec() const;
    const float3& ForwardVec() const;
    float LookAtDist() const
    {
        return look_at_dist_;
    }
    void LookAtDist(float look_at_dist)
    {
        look_at_dist_ = look_at_dist;
    }
    
    float FOV() const
    { 
        return fov_; 
    }
    float Aspect() const
    { 
        return aspect_; 
    }
    float NearPlane() const
    { 
        return near_plane_; 
    }
    float FarPlane() const
    { 
        return far_plane_; 
    
    }

    const float4x4& ViewMatrix() const;
    const float4x4& ProjMatrix() const;
    const float4x4& ViewProjMatrix() const;

    const float4x4& InverseViewMatrix() const;
    const float4x4& InverseProjMatrix() const;
    const float4x4& InverseViewProjMatrix() const;
    // 设置摄像机的投射矩阵
	//////////////////////////////////////////////////////////////////////////////////
	void ProjParams(float fov, float aspect, float near_plane, float far_plane);
    void ProjOrthoParams(float w, float h, float near_plane, float far_plane);

    void Dirty();
private:
    float		look_at_dist_ {1};

    float		fov_ {0.f};
    float		aspect_ {0.f};
    float		near_plane_ {0.f};
    float		far_plane_ {0.f};

    float4x4	proj_mat_ {float4x4::Identity()};
    float4x4    inv_proj_mat_ {float4x4::Identity()}; 
    mutable float4x4	view_proj_mat_ {float4x4::Identity()}; 
    mutable float4x4	inv_view_proj_mat_ {float4x4::Identity()}; 
    mutable bool view_proj_mat_dirty_{false};
};


using CameraPtr = std::shared_ptr<Camera>;



class ZENGINE_CORE_API PredefinedCameraCBuffer
{
public:
    static constexpr uint32_t max_num_cameras = 8;

    struct CameraInfo
    {
        alignas(16) float4x4 model_view;
        alignas(16) float4x4 mvp;
        alignas(16) float4x4 inv_mv;
        alignas(16) float4x4 inv_mvp;
        alignas(16) float3 eye_pos;
        alignas(4) float padding0;
        alignas(16) float3 forward_vec;
        alignas(4) float padding1;
        alignas(16) float3 up_vec;
        alignas(4) float padding2;
    };
    static_assert(sizeof(CameraInfo) == 304);

public:
    PredefinedCameraCBuffer();

    RenderEffectConstantBuffer* CBuffer() const
    {
        return predefined_cbuffer_;
    }

    uint32_t& NumCameras(RenderEffectConstantBuffer& cbuff) const;
    uint32_t& CameraIndices(RenderEffectConstantBuffer& cbuff, uint32_t index) const;
    CameraInfo& Camera(RenderEffectConstantBuffer& cbuff, uint32_t index) const;
    float4x4& PrevMvp(RenderEffectConstantBuffer& cbuff, uint32_t index) const;

private:
    RenderEffectPtr effect_;
    RenderEffectConstantBuffer* predefined_cbuffer_;

    uint32_t num_cameras_offset_;
    uint32_t camera_indices_offset_;
    uint32_t cameras_offset_;
    uint32_t prev_mvps_offset_;
};
}