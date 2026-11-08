#pragma once

#include <world/SceneComponent.h>

#include <cstdint>
#include <random>
#include <vector>

namespace RenderWorker
{
enum class ParticlePreset { Fire, Smoke, Sparks, Explosion, HeatHaze };
enum class ParticleBlendMode { Alpha, Additive, Distortion };
enum class ParticleSimulationSpace { Local, World };

// Emission properties are authored in emitter-local coordinates. World-space
// particles snapshot the emitter transform when born, then move independently.
struct ZENGINE_CORE_API ParticleSettings
{
    uint32_t max_particles = 512;
    float emission_rate = 90.f;
    float lifetime_min = 0.5f, lifetime_max = 1.1f;
    float speed_min = 1.2f, speed_max = 2.4f;
    float spread = 0.22f, radius = 0.25f;
    float start_size = 0.45f, end_size = 0.06f;
    float3 direction {0.f, 1.f, 0.f};
    float3 acceleration {0.f, 0.7f, 0.f};
    float4 start_color {1.f, 0.65f, 0.12f, 0.8f};
    float4 end_color {0.65f, 0.04f, 0.005f, 0.f};
    ParticleBlendMode blend = ParticleBlendMode::Additive;
    ParticleSimulationSpace simulation_space = ParticleSimulationSpace::Local;
    uint32_t burst_count = 0;
    float rotation_min = 0.f, rotation_max = 0.f;
    float angular_speed_min = 0.f, angular_speed_max = 0.f;
    float velocity_stretch = 0.f;
    float emissive_intensity = 1.f;
    float glow_strength = 0.f;
    float flicker_frequency = 0.f, flicker_amount = 0.f;
    float distortion_strength = 0.f;
    bool looping = true;
    float duration = 2.f;
    uint32_t seed = 1337;

    static ParticleSettings ForPreset(ParticlePreset preset);
    bool operator==(ParticleSettings const& other) const;
};

struct Particle
{
    float3 position {0.f, 0.f, 0.f};
    float3 velocity {0.f, 0.f, 0.f};
    float age = 0.f, lifetime = 1.f;
    float size = 1.f;
    float4 color {1.f, 1.f, 1.f, 1.f};
    float rotation = 0.f, angular_speed = 0.f;
    float3 acceleration {0.f, 0.f, 0.f};
    float size_scale = 1.f;
};

// CPU simulation has no renderer dependency, so it is also usable in headless tools.
class ZENGINE_CORE_API ParticleSystem final : public SceneComponent
{
public:
    NANO_RTTI_REGISTER_RUNTIME_CLASS(SceneComponent)

    explicit ParticleSystem(ParticleSettings const& settings = {});
    SceneComponentPtr Clone() const override;
    void MainThreadUpdate(float app_time, float elapsed_time) override;

    ParticleSettings const& Settings() const { return settings_; }
    // Sanitizes input, resets simulation and preserves the current playback state.
    void Settings(ParticleSettings const& settings);
    std::vector<Particle> const& Particles() const { return particles_; }

    // Uses 120 Hz fixed steps; ignores invalid deltas and caps a stall at one second.
    void Update(float elapsed_time);
    void Play();
    void Pause();
    void Stop();
    void Restart();
    // Adds a bounded burst while playing; does not change the automatic schedule.
    void EmitBurst(uint32_t count);
    bool IsPlaying() const { return playing_; }
    bool IsEmitting() const;

private:
    void Reset();
    void Step(double dt);
    void Spawn(float4x4 const& transform);
    float Random();

    ParticleSettings settings_;
    std::vector<Particle> particles_;
    std::mt19937 random_;
    double time_remainder_ = 0., emission_remainder_ = 0., emitter_age_ = 0.;
    double next_burst_time_ = 0.;
    bool initial_burst_pending_ = true;
    bool playing_ = true, stopped_ = false;
};
}
