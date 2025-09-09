#pragma once

#include <world/SceneNode.h>

namespace RenderWorker
{
class Camera:public SceneNode
{
public:
    Camera();
    
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

}