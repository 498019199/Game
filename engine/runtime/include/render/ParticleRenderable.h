#pragma once

#include <render/Renderable.h>
#include <world/ParticleSystem.h>

namespace RenderWorker
{
// CPU simulated particles rendered as camera-facing quads in the scene pass.
// A renderable belongs to one emitter; attach it and its ParticleSystem to the
// same SceneNode so the emitter's local positions inherit that node's transform.
class ZENGINE_CORE_API ParticleRenderable final : public Renderable
{
public:
    explicit ParticleRenderable(std::shared_ptr<ParticleSystem> system);

    RenderTechnique* GetRenderTechnique() const override;
    void AddToRenderQueue() override;
    void OnRenderBegin() override;

private:
    struct Vertex
    {
        float3 position;
        float4 color;
        float2 uv;
        float age;
    };
    struct DrawParticle
    {
        float3 center;
        Particle const* particle;
        float depth;
    };

    bool EmitterVisible() const;
    void EnsureCapacity(uint32_t count);

    std::shared_ptr<ParticleSystem> system_;
    RenderTechnique* alpha_technique_ = nullptr;
    RenderTechnique* additive_technique_ = nullptr;
    RenderTechnique* distortion_technique_ = nullptr;
    RenderEffectParameter* view_projection_ = nullptr;
    RenderEffectParameter* particle_style_ = nullptr;
    RenderEffectParameter* particle_glow_ = nullptr;
    RenderEffectParameter* particle_refraction_ = nullptr;
    RenderEffectParameter* scene_color_ = nullptr;
    TexturePtr scene_color_snapshot_;
    GraphicsBufferPtr vertices_buffer_;
    uint32_t capacity_ = 0;
    std::vector<DrawParticle> draw_particles_;
    std::vector<Vertex> vertices_;
};
}
