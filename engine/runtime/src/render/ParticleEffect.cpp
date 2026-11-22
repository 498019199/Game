#include <render/ParticleEffect.h>

#include <stdexcept>

namespace RenderWorker
{
EffectDesc ParticleEffect::Describe()
{
    EffectDesc desc;
    desc.name = "Particle";
    desc.shader_files = {"Particle.hlsl"};
    desc.parameters = {
        {"particle_view_projection", EffectParameterType::Float4x4},
        {"particle_style", EffectParameterType::Float},
        {"particle_glow", EffectParameterType::Float},
        {"particle_refraction", EffectParameterType::Float4},
        {"particle_scene_color", EffectParameterType::Texture2D},
        {"particle_scene_sampler", EffectParameterType::Sampler}
    };
    auto& sampler = desc.parameters.back().sampler;
    sampler.filter = TFO_Min_Mag_Mip_Linear;
    sampler.addr_mode_u = sampler.addr_mode_v = TAM_Clamp;
    auto add = [&](char const* name, char const* pixel, AlphaBlendFactor destination)
    {
        PassDesc pass;
        pass.name = "Main";
        pass.entry_points[std::to_underlying(ShaderStage::Vertex)] = "ParticleVS";
        pass.entry_points[std::to_underlying(ShaderStage::Pixel)] = pixel;
        pass.rasterizer.cull_mode = CM_None;
        pass.depth_stencil.depth_write_mask = false;
        pass.depth_stencil.depth_func = CF_LessEqual;
        pass.blend.blend_enable[0] = true;
        pass.blend.src_blend[0] = pass.blend.src_blend_alpha[0] = ABF_One;
        pass.blend.dest_blend[0] = pass.blend.dest_blend_alpha[0] = destination;
        desc.techniques.push_back({name, ShaderModel(0, 0), {pass}});
    };
    add("ParticleAlphaTech", "ParticlePS", ABF_Inv_Src_Alpha);
    add("ParticleDistortionTech", "ParticleDistortionPS", ABF_Inv_Src_Alpha);
    add("ParticleAdditiveTech", "ParticlePS", ABF_One);
    return desc;
}

ParticleEffect::ParticleEffect() : effect_(SyncLoadRenderEffect(Describe()))
{
    auto parameter = [&](char const* name, RenderEffectDataType type)
    {
        auto* value = effect_->ParameterByName(name);
        if (!value || value->Type() != type)
            throw std::runtime_error(std::string("ParticleEffect: missing or incompatible parameter ") + name);
        if (type != REDT_texture2D && !value->InCBuffer())
            throw std::runtime_error(std::string("ParticleEffect: unbound constant ") + name);
        return value;
    };
    view_projection_ = parameter("particle_view_projection", REDT_float4x4);
    style_ = parameter("particle_style", REDT_float);
    glow_ = parameter("particle_glow", REDT_float);
    refraction_ = parameter("particle_refraction", REDT_float4);
    scene_color_ = parameter("particle_scene_color", REDT_texture2D);
    alpha_ = effect_->TechniqueByName("ParticleAlphaTech");
    additive_ = effect_->TechniqueByName("ParticleAdditiveTech");
    distortion_ = effect_->TechniqueByName("ParticleDistortionTech");
    if (!alpha_ || !additive_ || !distortion_ || !alpha_->Validate() || !additive_->Validate() || !distortion_->Validate())
        throw std::runtime_error("ParticleEffect: shader compilation or linking failed");
    // A cached template can belong to another live emitter. Always establish
    // this instance's own initial values after cloning it.
    SetViewProjection(float4x4::Identity());
    SetMode(ParticleBlendMode::Alpha);
    SetGlow(0);
    SetRefraction(nullptr, 0);
}

RenderTechnique* ParticleEffect::Technique(ParticleBlendMode mode) const
{
    switch (mode)
    {
    case ParticleBlendMode::Alpha: return alpha_;
    case ParticleBlendMode::Additive: return additive_;
    case ParticleBlendMode::Distortion: return distortion_;
    default: throw std::invalid_argument("ParticleEffect: invalid blend mode");
    }
}
void ParticleEffect::SetViewProjection(float4x4 const& matrix) { *view_projection_ = matrix; }
void ParticleEffect::SetMode(ParticleBlendMode mode)
{
    (void)Technique(mode);
    *style_ = mode == ParticleBlendMode::Additive ? 1.f : 0.f;
}
void ParticleEffect::SetGlow(float intensity) { *glow_ = intensity; }
void ParticleEffect::SetRefraction(TexturePtr const& scene_color, float strength)
{
    *scene_color_ = scene_color;
    *refraction_ = scene_color && scene_color->Width(0) && scene_color->Height(0)
        ? float4(1.f / scene_color->Width(0), 1.f / scene_color->Height(0), strength, 0.f)
        : float4(0, 0, 0, 0);
}
}
