#include <render/ParticleRenderable.h>

#include <base/Context.h>
#include <render/Camera.h>
#include <render/FrameBuffer.h>
#include <render/GraphicsBuffer.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/Texture.h>
#include <render/Viewport.h>
#include <world/SceneNode.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace RenderWorker
{
ParticleRenderable::ParticleRenderable(std::shared_ptr<ParticleSystem> system)
    : Renderable(L"Particle Effect"), system_(std::move(system))
{
    if (!system_) throw std::invalid_argument("ParticleRenderable requires a ParticleSystem");

    effect_ = particle_effect_.Instance();

    technique_ = this->GetRenderTechnique();
    effect_attrs_ = EA_SimpleForward | EA_TransparencyFront;
    auto& rf = Context::Instance().RenderFactoryInstance();
    rls_[0] = rf.MakeRenderLayout();
    rls_[0]->TopologyType(RenderLayout::TT_TriangleList);
    // Even a freshly created or stopped system has a valid empty layout.
    this->EnsureCapacity(1);
    rls_[0]->NumVertices(0);
    rls_[0]->NumIndices(0);
    pos_aabb_ = AABBox(float3(-1, -1, -1), float3(1, 1, 1));
    tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));
}

RenderTechnique* ParticleRenderable::GetRenderTechnique() const
{
    return particle_effect_.Technique(system_->Settings().blend);
}

bool ParticleRenderable::EmitterVisible() const
{
    if (!system_->Enabled()) return false;
    for (auto const* node = system_->BoundSceneNode(); node; node = node->Parent())
        if (!node->Visible()) return false;
    return true;
}

void ParticleRenderable::AddToRenderQueue()
{
    if (this->EmitterVisible() && !system_->Particles().empty()) Renderable::AddToRenderQueue();
}

void ParticleRenderable::EnsureCapacity(uint32_t count)
{
    if (count <= capacity_) return;
    capacity_ = std::max(count, std::max(64u, capacity_ * 2));
    auto& rf = Context::Instance().RenderFactoryInstance();
    static_assert(sizeof(Vertex) == 10 * sizeof(float));
    vertices_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write,
        capacity_ * 4 * sizeof(Vertex), nullptr);
    rls_[0]->BindVertexStream(vertices_buffer_, MakeSpan({
        VertexElement(VEU_Position, 0, EF_BGR32F),
        VertexElement(VEU_Diffuse, 0, EF_ABGR32F),
        VertexElement(VEU_TextureCoord, 0, EF_GR32F),
        VertexElement(VEU_TextureCoord, 1, EF_R32F)}));

    std::vector<uint32_t> indices(capacity_ * 6);
    for (uint32_t i = 0; i < capacity_; ++i)
    {
        uint32_t const base = i * 4;
        uint32_t const offset = i * 6;
        indices[offset] = base;
        indices[offset + 1] = base + 1;
        indices[offset + 2] = base + 2;
        indices[offset + 3] = base;
        indices[offset + 4] = base + 2;
        indices[offset + 5] = base + 3;
    }
    auto indices_buffer = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
        static_cast<uint32_t>(indices.size() * sizeof(uint32_t)), indices.data());
    rls_[0]->BindIndexStream(indices_buffer, EF_R32UI);
}

void ParticleRenderable::OnRenderBegin()
{
    auto& layout = *rls_[0];
    layout.NumVertices(0);
    layout.NumIndices(0);
    auto const& particles = system_->Particles();
    if (!this->EmitterVisible() || particles.empty()) return;

    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    if (!re.CurFrameBuffer()) return;
    auto const& viewport = re.CurFrameBuffer()->Viewport();
    if (!viewport || viewport->NumCameras() == 0 || !viewport->Camera()) return;
    auto const& camera = *viewport->Camera();
    if (!camera.BoundSceneNode()) return;

    // Simulation and rendering both run on the main thread. The background
    // scene update cannot enter World::Flush while its update mutex is held.
    auto const& settings = system_->Settings();
    bool const additive = settings.blend == ParticleBlendMode::Additive;
    bool const distortion = settings.blend == ParticleBlendMode::Distortion;
    bool const world_space = settings.simulation_space == ParticleSimulationSpace::World;
    if (distortion)
    {
        // No stale snapshot is ever drawn after a failed/minimized capture.
        // Zero displacement is a true no-op, including framebuffer alpha.
        if (settings.distortion_strength <= 0.f || !re.CaptureCurrentColor(scene_color_snapshot_)) return;
        particle_effect_.SetRefraction(scene_color_snapshot_, settings.distortion_strength);
    }
    draw_particles_.clear();
    draw_particles_.reserve(particles.size());
    auto const forward = MathWorker::normalize(camera.ForwardVec());
    for (auto const& particle : particles)
    {
        auto const center = world_space ? particle.position : MathWorker::transform_coord(particle.position, model_mat_);
        draw_particles_.push_back({center, &particle, MathWorker::dot(center - camera.EyePos(), forward)});
    }
    if (!additive)
    {
        std::stable_sort(draw_particles_.begin(), draw_particles_.end(),
            [](DrawParticle const& a, DrawParticle const& b) { return a.depth > b.depth; });
    }

    // Keep sprites camera-facing even when their emitter rotates. Use the
    // largest axis scale to preserve a round sprite under nonuniform scaling.
    float const scale = world_space ? 1.f : std::max({
        MathWorker::length(float3(model_mat_.Row(0))),
        MathWorker::length(float3(model_mat_.Row(1))),
        MathWorker::length(float3(model_mat_.Row(2)))});
    auto const right = MathWorker::normalize(camera.RightVec());
    auto const up = MathWorker::normalize(camera.UpVec());
    vertices_.clear();
    vertices_.reserve(draw_particles_.size() * 4);
    for (auto const& item : draw_particles_)
    {
        auto const& particle = *item.particle;
        float const half_size = particle.size * scale * 0.5f;
        auto const cosine = std::cos(particle.rotation);
        auto const sine = std::sin(particle.rotation);
        auto horizontal = (right * cosine + up * sine) * half_size;
        auto vertical = (up * cosine - right * sine) * half_size;
        auto center = item.center;
        float const flicker = 1.f - settings.flicker_amount * 0.5f *
            (1.f - std::cos(6.28318530718f * particle.age * settings.flicker_frequency));
        auto color = particle.color;
        if (distortion) color.w() *= flicker;
        else
        {
            float const intensity = settings.emissive_intensity * flicker;
            color.x() *= intensity;
            color.y() *= intensity;
            color.z() *= intensity;
        }
        auto tail_color = color;
        std::array<float2, 4> uvs {float2(0, 1), float2(0, 0), float2(1, 0), float2(1, 1)};

        // World particles already captured their emitter's transform at birth.
        // Transform local velocities as directions, so translation cannot alter a trail.
        auto velocity = particle.velocity;
        if (!world_space)
            velocity = float3(model_mat_.Row(0)) * velocity.x() +
                float3(model_mat_.Row(1)) * velocity.y() + float3(model_mat_.Row(2)) * velocity.z();
        auto const screen_velocity = right * MathWorker::dot(velocity, right) + up * MathWorker::dot(velocity, up);
        float const screen_speed = MathWorker::length(screen_velocity);
        if (settings.velocity_stretch > 0.f && screen_speed > 0.0001f)
        {
            auto const along = screen_velocity / screen_speed;
            auto const across = right * MathWorker::dot(along, up) - up * MathWorker::dot(along, right);
            // Avoid an enormous quad after extreme velocity/scale edits. The
            // particle remains the head; its tail extends behind its motion.
            float const trail_length = std::min(screen_speed * settings.velocity_stretch, 100.f);
            horizontal = across * half_size;
            vertical = along * (half_size + trail_length * 0.5f);
            center -= along * (trail_length * 0.5f);
            tail_color.w() *= 1.f / (1.f + trail_length / std::max(2.f * half_size, 0.001f));
            // Spin the procedural sprite inside the velocity-aligned quad,
            // rather than turning the tail away from the direction of travel.
            for (auto& uv : uvs)
            {
                auto const centered_uv = uv - float2(0.5f, 0.5f);
                uv = float2(centered_uv.x() * cosine - centered_uv.y() * sine,
                    centered_uv.x() * sine + centered_uv.y() * cosine) + float2(0.5f, 0.5f);
            }
        }
        vertices_.push_back({center - horizontal - vertical, tail_color, uvs[0], particle.age});
        vertices_.push_back({center - horizontal + vertical, color, uvs[1], particle.age});
        vertices_.push_back({center + horizontal + vertical, color, uvs[2], particle.age});
        vertices_.push_back({center + horizontal - vertical, tail_color, uvs[3], particle.age});
    }

    this->EnsureCapacity(static_cast<uint32_t>(draw_particles_.size()));
    {
        GraphicsBuffer::Mapper mapped(*vertices_buffer_, BA_Write_Only);
        std::memcpy(mapped.Pointer<void>(), static_cast<void const*>(vertices_.data()), vertices_.size() * sizeof(Vertex));
    }
    layout.NumVertices(static_cast<uint32_t>(vertices_.size()));
    layout.NumIndices(static_cast<uint32_t>(draw_particles_.size() * 6));
    particle_effect_.SetViewProjection(camera.ViewProjMatrix());
    particle_effect_.SetMode(settings.blend);
    particle_effect_.SetGlow(settings.glow_strength);
}
}
