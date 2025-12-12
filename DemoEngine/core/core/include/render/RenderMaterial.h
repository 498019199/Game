#pragma once

#include <render/RenderEffect.h>
#include <render/RenderView.h>

namespace RenderWorker
{
class RenderMaterial;
using RenderMaterialPtr = std::shared_ptr<RenderMaterial>;

class ZENGINE_CORE_API RenderMaterial final
{
    ZENGINE_NONCOPYABLE(RenderMaterial);

public:
    enum TextureSlot
    {
        TS_Albedo,
        TS_MetalnessGlossiness,
        TS_Emissive,
        TS_Normal,
        TS_Height,
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

public:
    RenderMaterial();

    RenderMaterialPtr Clone() const;

	void Name(std::string_view name);
	const std::string& Name() const;

    void Albedo(const float4& value);
	const float4&  Albedo() const;
    void Metalness(float value);
	float Metalness() const;
	void Glossiness(float value);
	float Glossiness() const;
	void Emissive(float3 const& value);
	const float3& Emissive() const;
    void Transparent(bool value);
	bool Transparent() const;
	void AlphaTestThreshold(float value);
	float AlphaTestThreshold() const;
	void Sss(bool value);
	bool Sss() const;
	void TwoSided(bool value);
	bool TwoSided() const;
	void NormalScale(float value);
	float NormalScale() const;
	void OcclusionStrength(float value);
	float OcclusionStrength() const;

    void HeightOffset(float value);
    float HeightOffset() const;
    void HeightScale(float value);
    float HeightScale() const;
    void EdgeTessHint(float value);
    float EdgeTessHint() const;
    void InsideTessHint(float value);
    float InsideTessHint() const;
    void MinTessFactor(float value);
    float MinTessFactor() const;
    void MaxTessFactor(float value);
    float MaxTessFactor() const;

	void DetailMode(SurfaceDetailMode value);
	SurfaceDetailMode DetailMode() const;

    void TextureName(TextureSlot slot, std::string_view name);
	const std::string& TextureName(TextureSlot slot) const;
    
    void Texture(TextureSlot slot, ShaderResourceViewPtr srv);
	const ShaderResourceViewPtr& Texture(TextureSlot slot) const;

	void LoadTextureSlots();
private:
	std::string name_;

	bool is_sw_mode_ = false;
    RenderEffectConstantBufferPtr cbuffer_;
    std::vector<uint32_t> sw_cbuffer_;

    RenderEffectParameter* albedo_tex_param_ = nullptr;
    RenderEffectParameter* metalness_glossiness_tex_param_ = nullptr;
    RenderEffectParameter* emissive_tex_param_ = nullptr;
    RenderEffectParameter* normal_tex_param_ = nullptr;
    RenderEffectParameter* height_tex_param_ = nullptr;
    RenderEffectParameter* occlusion_tex_param_ = nullptr;

    bool transparent_ = false;
    bool sss_ = false;
    bool two_sided_ = false;
    SurfaceDetailMode detail_mode_ = SurfaceDetailMode::ParallaxMapping;
    std::array<std::pair<std::string, ShaderResourceViewPtr>, TS_NumTextureSlots> textures_;
};

}