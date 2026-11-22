#pragma once

#include <render/EffectDesc.h>
#include <render/RenderEffect.h>
#include <world/ParticleSystem.h>

namespace RenderWorker
{
// One independent parameter instance; compiled shaders are shared through ResLoader.
class ZENGINE_CORE_API ParticleEffect final
{
    ZENGINE_NONCOPYABLE(ParticleEffect);
public:
    ParticleEffect();
    static EffectDesc Describe();

    RenderEffectPtr const& Instance() const noexcept { return effect_; }
    RenderTechnique* Technique(ParticleBlendMode mode) const;
    void SetViewProjection(float4x4 const& matrix);
    void SetMode(ParticleBlendMode mode);
    void SetGlow(float intensity);
    void SetRefraction(TexturePtr const& scene_color, float strength);

private:
    RenderEffectPtr effect_;
    RenderTechnique* alpha_ = nullptr;
    RenderTechnique* additive_ = nullptr;
    RenderTechnique* distortion_ = nullptr;
    RenderEffectParameter* view_projection_ = nullptr;
    RenderEffectParameter* style_ = nullptr;
    RenderEffectParameter* glow_ = nullptr;
    RenderEffectParameter* refraction_ = nullptr;
    RenderEffectParameter* scene_color_ = nullptr;
};
}
