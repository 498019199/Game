#pragma once
// #include <core/SceneNode.h>
#include <common/common.h>
// class LightSource: public SceneNode
// {
// public:
//     // 光照模型
//     enum LightType
//     {
//         LT_Ambient = 0,
//         LT_Directional,     //平行光
//         LT_Point,           //点光
//         LT_Spot,            // 聚光灯
//         LT_SphereArea,
//         LT_TubeArea,

//         LT_NumLightTypes
//     };

//     enum LightSrcAttrib
//     {
//         LSA_NoShadow = 1UL << 0,
//         LSA_NoDiffuse = 1UL << 1,
//         LSA_NoSpecular = 1UL << 2,
//         LSA_IndirectLighting = 1UL << 3,
//         LSA_Temporary = 1UL << 4
//     };
// public:
// 	explicit LightSource(LightType type);
// 	virtual ~LightSource() noexcept;

// 	LightType Type() const;

//     const float3& Position() const;
//     const float3& Direction() const;
//     quater Rotation() const;

//     virtual const float3& Falloff() const;
// 	virtual void Falloff(float3 const & fall_off);

//     virtual float Range() const;
// 	virtual void Range(float range);
// private:
//     LightType type_;
//     float4 color_ = float4(0, 0, 0, 0); // 光源颜色
//     float range_ = -1; // 光照范围
//     float3 falloff_; // 衰退系数
// };

// class AmbientLightSource: public LightSource
// {

// };

// class DirectionalLightSource: public LightSource
// {

// };

// class PointLightSource: public LightSource
// {

// };

// class SpotLightSource: public LightSource
// {

// };

// class SphereAreaLightSource: public LightSource
// {

// };

// class TubeAreaLightSource: public LightSource
// {

// };

namespace RenderWorker
{
struct DirectionalLightSource
{
    DirectionalLightSource() { memset(this, 0, sizeof(DirectionalLightSource)); }

    float4 ambient_;
    float4 diffuse_;
    float4 specular_;
    float3 direction_;
    float pad_;// 占位最后一个float，这样我们就可以设置光源数组了。
};

struct PointLightSource
{
    PointLightSource() { memset(this, 0, sizeof(PointLightSource)); }

    float4 ambient_;
    float4 diffuse_;
    float4 specular_;

    // 打包到4D矢量: (Position, Range)
	float3 pos_;
	float range_;

	// 打包到4D矢量: (A0, A1, A2, Pad)
	float3 att_;
	float pad_; // 占位最后一个float，，这样我们就可以设置光源数组了。
};

struct SpotLightSource
{
    SpotLightSource() { memset(this, 0, sizeof(SpotLightSource)); }

    float4 ambient_;
    float4 diffuse_;
    float4 specular_;

	// 打包到4D矢量: (Position, Range)
	float3 pos_;
	float range_;

	// 打包到4D矢量: (Direction, Spot)
	float3 direction_;
	float spot_;

	// 打包到4D矢量: (Att, Pad)
	float3 att_;
	float pad_; // 占位最后一个float，，这样我们就可以设置光源数组了。
};

// 物体表面材质
struct Material
{
    Material() = default;

    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

    Material(Material&&) = default;
    Material& operator=(Material&&) = default;

    float4 ambient_;
    float4 diffuse_;
    float4 specular_; // w = 镜面反射强度
    float4 reflect_;
};
}