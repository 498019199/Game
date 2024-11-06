#pragma once
#include <render/IRenderView.h>

namespace CoreWorker
{
class IRenderMaterial
{
public:
    //http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
	enum TextureType : uint32_t
	{
		TS_Albedo,				// 反照率贴图；用于体现模型的纹理，颜色。
		TS_MetalnessGlossiness,	// 金属度贴图;  体现模型的金属高光反射。
								// 光泽度贴图；模型在某个角度看起来具有光泽。
		TS_Emissive,			// 自发光贴图；让模型自发光。
		TS_Normal,				// 法线贴图；用于增加模型的细节。
		TS_Height,				// 视差贴图；更有立体感的一种贴图方。
		TS_Occlusion,

		TS_NumTextureSlots
	};

    enum class SurfaceDetailMode : uint32_t
	{
		ParallaxMapping = 0,
		ParallaxOcclusionMapping,
		FlatTessellation,
		SmoothTessellation
	};


	void SetName(const std::string& name) { material_name_ = name; }
    const std::string& GetName() const { return material_name_; }

	void Albedo(float4 albedo) { albedo_ = albedo; }
    float4 Albedo() const { return albedo_; }
	void Diffuse(float4 diffuse) { diffuse_ = diffuse; }
    float4 Diffuse() const { return diffuse_; }
	void Specular(float4 specular) { specular_ = specular; }
    float4 Specular() const { return specular_; }

	void Metalness(float metalness) { metalness_ = metalness; }
    float Metalness() const { return metalness_; }
	void Glossiness(float glossiness) { glossiness_ = glossiness; }
    float Glossiness() const { return glossiness_; }
	void Emissive(float3 emissive) { emissive_ = emissive; }
    float3 Emissive() const { return emissive_; }
    
	void Transparent(float transparent) { transparent_ = transparent; }
    float Transparent() const { return transparent_; }

	void AlphaTestThreshold(float alpha_test) { alpha_test_ = alpha_test; }
    float AlphaTestThreshold() const { return alpha_test_ ; }
	void SSS(bool is) { sss_ = is; }
    bool SSS() const { return sss_; }
	void TwoSided(bool is) { two_sided_ = is; }
    bool TwoSided() const { return two_sided_; }

    void NormalScale(float scale) { normal_scale_offset_ = scale; }
    float NormalScale() const { return normal_scale_offset_ ; }
    void OcclusionStrength(float value) { occlusion_strength_ = value; }
	float OcclusionStrength() const { return occlusion_strength_ ; }

	void DetailMode(SurfaceDetailMode value){detail_mode_ = value;}
	SurfaceDetailMode DetailMode() const{return detail_mode_;}

    void HeightOffset(float offset) { height_offset_scale_.x() = offset; }
    float HeightOffset() { return height_offset_scale_.x(); }
    void HeightScale(float scale) { height_offset_scale_.y() = scale; }
    float HeightScale() { return height_offset_scale_.y(); }

    void EdgeTessHint(float value) {tess_factors_.x() = value; }
    float EdgeTessHint() { return tess_factors_.x(); }
    void InsideTessHint(float value) { tess_factors_.y() = value; }
    float InsideTessHint() { return tess_factors_.y(); }
    void MinTessFactor(float value) {tess_factors_.z() = value; }
    float MinTessFactor() { return tess_factors_.z(); }
    void MaxTessFactor(float value) { tess_factors_.w() = value; }
    float MaxTessFactor() { return tess_factors_.w(); }

	void SetTextureName(TextureType solt, const std::string_view& name);
    void SetTexture(TextureType solt, ShaderResourceViewPtr srv);
    const std::string& GetTextureName(TextureType slot) const;

    void LoadTextureSlots();
private:
    std::string material_name_;
	float4 albedo_;
	float4 diffuse_;
	float4 specular_;

	float shininess_;
	float metalness_;
	float glossiness_;
	float3 emissive_;

	bool transparent_;
	float alpha_test_;
	bool sss_;
	bool two_sided_;

    float normal_scale_offset_;
    float occlusion_strength_;

	float2 height_offset_scale_;
	float4 tess_factors_;

    IRenderMaterial::SurfaceDetailMode detail_mode_;
	std::vector<uint32_t> sw_cbuffer_;
	std::array<std::pair<std::string, ShaderResourceViewPtr>, TS_NumTextureSlots> textures_;
};









const float MAX_SHININESS = 8192;
const float INV_LOG_MAX_SHININESS = 1 / std::log(MAX_SHININESS);

inline float Shininess2Glossiness(float shininess)
{
	return std::log(shininess) * INV_LOG_MAX_SHININESS;
}

inline float Glossiness2Shininess(float glossiness)
{
	return std::pow(MAX_SHININESS, glossiness);
}

using RenderMaterialPtr = std::shared_ptr<IRenderMaterial>;
}