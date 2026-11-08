#include <world/ParticleSystem.h>
#include <world/SceneNode.h>

#include <algorithm>
#include <cmath>

namespace RenderWorker
{
namespace
{
constexpr float pi = 3.14159265358979323846f;
// Authoring values are floats, while the fixed-step clock is a double.
constexpr double time_epsilon = 1.e-6;

float FiniteClamp(float value, float fallback, float low, float high)
{
    return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
}

ParticleSettings Sanitize(ParticleSettings settings)
{
    ParticleSettings const defaults;
    settings.max_particles = std::clamp(settings.max_particles, 1u, 16384u);
    settings.burst_count = std::min(settings.burst_count, 16384u);
    settings.emission_rate = FiniteClamp(settings.emission_rate, defaults.emission_rate, 0.f, 50000.f);
    settings.lifetime_min = FiniteClamp(settings.lifetime_min, defaults.lifetime_min, 0.01f, 60.f);
    settings.lifetime_max = FiniteClamp(settings.lifetime_max, defaults.lifetime_max, settings.lifetime_min, 60.f);
    settings.speed_min = FiniteClamp(settings.speed_min, defaults.speed_min, 0.f, 1000.f);
    settings.speed_max = FiniteClamp(settings.speed_max, defaults.speed_max, settings.speed_min, 1000.f);
    // Fallbacks also need to respect a user-provided lower bound.
    settings.lifetime_max = std::max(settings.lifetime_min, settings.lifetime_max);
    settings.speed_max = std::max(settings.speed_min, settings.speed_max);
    settings.spread = FiniteClamp(settings.spread, defaults.spread, 0.f, pi);
    settings.radius = FiniteClamp(settings.radius, defaults.radius, 0.f, 1000.f);
    settings.start_size = FiniteClamp(settings.start_size, defaults.start_size, 0.001f, 100.f);
    settings.end_size = FiniteClamp(settings.end_size, defaults.end_size, 0.001f, 100.f);
    settings.duration = FiniteClamp(settings.duration, defaults.duration, 0.01f, 3600.f);
    settings.rotation_min = FiniteClamp(settings.rotation_min, 0.f, -2.f * pi, 2.f * pi);
    settings.rotation_max = std::max(settings.rotation_min,
        FiniteClamp(settings.rotation_max, 0.f, settings.rotation_min, 2.f * pi));
    settings.angular_speed_min = FiniteClamp(settings.angular_speed_min, 0.f, -20.f * pi, 20.f * pi);
    settings.angular_speed_max = std::max(settings.angular_speed_min,
        FiniteClamp(settings.angular_speed_max, 0.f, settings.angular_speed_min, 20.f * pi));
    settings.velocity_stretch = FiniteClamp(settings.velocity_stretch, 0.f, 0.f, 10.f);
    settings.emissive_intensity = FiniteClamp(settings.emissive_intensity, 1.f, 0.f, 16.f);
    settings.glow_strength = FiniteClamp(settings.glow_strength, 0.f, 0.f, 4.f);
    settings.flicker_frequency = FiniteClamp(settings.flicker_frequency, 0.f, 0.f, 60.f);
    settings.flicker_amount = FiniteClamp(settings.flicker_amount, 0.f, 0.f, 1.f);
    settings.distortion_strength = FiniteClamp(settings.distortion_strength, 0.f, 0.f, 64.f);
    for (int i = 0; i < 3; ++i)
    {
        settings.direction[i] = FiniteClamp(settings.direction[i], defaults.direction[i], -1.f, 1.f);
        settings.acceleration[i] = FiniteClamp(settings.acceleration[i], defaults.acceleration[i], -1000.f, 1000.f);
    }
    if (MathWorker::length(settings.direction) < 0.0001f)
        settings.direction = defaults.direction;
    // Normalize at spawn time, preserving stable serialized settings for dirty checks.
    for (int i = 0; i < 4; ++i)
    {
        auto const upper = i == 3 ? 1.f : 16.f;
        settings.start_color[i] = FiniteClamp(settings.start_color[i], defaults.start_color[i], 0.f, upper);
        settings.end_color[i] = FiniteClamp(settings.end_color[i], defaults.end_color[i], 0.f, upper);
    }
    if (settings.blend != ParticleBlendMode::Alpha && settings.blend != ParticleBlendMode::Additive &&
        settings.blend != ParticleBlendMode::Distortion)
        settings.blend = defaults.blend;
    if (settings.simulation_space != ParticleSimulationSpace::Local &&
        settings.simulation_space != ParticleSimulationSpace::World)
        settings.simulation_space = defaults.simulation_space;
    return settings;
}
}

ParticleSettings ParticleSettings::ForPreset(ParticlePreset preset)
{
    ParticleSettings settings;
    switch (preset)
    {
    case ParticlePreset::Smoke:
        settings.emission_rate = 32.f;
        settings.lifetime_min = 2.f;
        settings.lifetime_max = 3.8f;
        settings.speed_min = 0.45f;
        settings.speed_max = 0.8f;
        settings.spread = 0.3f;
        settings.radius = 0.3f;
        settings.start_size = 0.4f;
        settings.end_size = 1.6f;
        settings.acceleration = float3(0.12f, 0.18f, 0.f);
        settings.start_color = float4(0.24f, 0.27f, 0.3f, 0.42f);
        settings.end_color = float4(0.55f, 0.57f, 0.6f, 0.f);
        settings.blend = ParticleBlendMode::Alpha;
        break;
    case ParticlePreset::Sparks:
        settings.emission_rate = 65.f;
        settings.lifetime_min = 0.5f;
        settings.lifetime_max = 1.5f;
        settings.speed_min = 2.f;
        settings.speed_max = 5.f;
        settings.spread = 0.8f;
        settings.radius = 0.08f;
        settings.start_size = 0.09f;
        settings.end_size = 0.015f;
        settings.acceleration = float3(0.f, -5.f, 0.f);
        settings.start_color = float4(1.f, 0.85f, 0.35f, 1.f);
        settings.end_color = float4(1.f, 0.18f, 0.01f, 0.f);
        settings.velocity_stretch = 0.12f;
        break;
    case ParticlePreset::Explosion:
        settings.emission_rate = 0.f;
        settings.burst_count = 96;
        settings.looping = false;
        settings.duration = 0.05f;
        settings.simulation_space = ParticleSimulationSpace::World;
        settings.lifetime_min = 0.4f;
        settings.lifetime_max = 1.2f;
        settings.speed_min = 2.f;
        settings.speed_max = 6.f;
        settings.spread = pi;
        settings.radius = 0.05f;
        settings.start_size = 0.16f;
        settings.end_size = 0.02f;
        settings.acceleration = float3(0.f, -2.f, 0.f);
        settings.start_color = float4(1.f, 0.7f, 0.2f, 1.f);
        settings.end_color = float4(1.f, 0.08f, 0.005f, 0.f);
        settings.rotation_max = 2.f * pi;
        settings.velocity_stretch = 0.15f;
        break;
    case ParticlePreset::HeatHaze:
        settings.emission_rate = 15.f;
        settings.simulation_space = ParticleSimulationSpace::World;
        settings.blend = ParticleBlendMode::Distortion;
        settings.distortion_strength = 12.f;
        settings.lifetime_min = 1.5f;
        settings.lifetime_max = 2.5f;
        settings.speed_min = 0.2f;
        settings.speed_max = 0.6f;
        settings.spread = 0.25f;
        settings.radius = 0.25f;
        settings.start_size = 0.6f;
        settings.end_size = 1.4f;
        settings.acceleration = float3(0.f, 0.1f, 0.f);
        settings.start_color = float4(1.f, 1.f, 1.f, 0.3f);
        settings.end_color = float4(1.f, 1.f, 1.f, 0.f);
        settings.rotation_max = 2.f * pi;
        settings.angular_speed_min = -0.5f;
        settings.angular_speed_max = 0.5f;
        break;
    case ParticlePreset::Fire:
        break;
    }
    return settings;
}

bool ParticleSettings::operator==(ParticleSettings const& other) const
{
    return max_particles == other.max_particles && emission_rate == other.emission_rate &&
        lifetime_min == other.lifetime_min && lifetime_max == other.lifetime_max &&
        speed_min == other.speed_min && speed_max == other.speed_max && spread == other.spread &&
        radius == other.radius && start_size == other.start_size && end_size == other.end_size &&
        direction == other.direction && acceleration == other.acceleration &&
        start_color == other.start_color && end_color == other.end_color && blend == other.blend &&
        looping == other.looping && duration == other.duration && seed == other.seed &&
        simulation_space == other.simulation_space && burst_count == other.burst_count &&
        rotation_min == other.rotation_min && rotation_max == other.rotation_max &&
        angular_speed_min == other.angular_speed_min && angular_speed_max == other.angular_speed_max &&
        velocity_stretch == other.velocity_stretch && emissive_intensity == other.emissive_intensity &&
        glow_strength == other.glow_strength && flicker_frequency == other.flicker_frequency &&
        flicker_amount == other.flicker_amount && distortion_strength == other.distortion_strength;
}

ParticleSystem::ParticleSystem(ParticleSettings const& settings)
    : settings_(Sanitize(settings)), random_(settings_.seed)
{
    particles_.reserve(settings_.max_particles);
    next_burst_time_ = settings_.duration;
}

SceneComponentPtr ParticleSystem::Clone() const
{
    auto clone = std::make_shared<ParticleSystem>(settings_);
    clone->Enabled(Enabled());
    if (stopped_) clone->Stop();
    else if (!playing_) clone->Pause();
    return clone;
}

void ParticleSystem::Settings(ParticleSettings const& settings)
{
    auto const sanitized = Sanitize(settings);
    if (sanitized == settings_) return;
    settings_ = sanitized;
    Reset();
}

void ParticleSystem::Reset()
{
    particles_.clear();
    particles_.reserve(settings_.max_particles);
    random_.seed(settings_.seed);
    emitter_age_ = time_remainder_ = emission_remainder_ = 0.;
    next_burst_time_ = settings_.duration;
    initial_burst_pending_ = true;
}

void ParticleSystem::Play()
{
    if (stopped_ || (!settings_.looping && emitter_age_ + time_epsilon >= settings_.duration && particles_.empty()))
        Reset();
    stopped_ = false;
    playing_ = true;
}

void ParticleSystem::Pause() { playing_ = false; }

void ParticleSystem::Stop()
{
    playing_ = false;
    stopped_ = true;
    Reset();
}

void ParticleSystem::Restart()
{
    Reset();
    stopped_ = false;
    playing_ = true;
}

bool ParticleSystem::IsEmitting() const
{
    return Enabled() && playing_ &&
        ((settings_.emission_rate > 0.f && (settings_.looping || emitter_age_ + time_epsilon < settings_.duration)) ||
         (settings_.burst_count > 0 && (initial_burst_pending_ || settings_.looping)));
}

void ParticleSystem::MainThreadUpdate(float app_time, float elapsed_time)
{
    SceneComponent::MainThreadUpdate(app_time, elapsed_time);
    Update(elapsed_time);
}

void ParticleSystem::Update(float elapsed_time)
{
    if (!Enabled() || !playing_ || !std::isfinite(elapsed_time) || elapsed_time <= 0.f) return;
    if (initial_burst_pending_)
    {
        EmitBurst(settings_.burst_count);
        initial_burst_pending_ = false;
    }
    constexpr double step = 1. / 120.;
    time_remainder_ += std::min(static_cast<double>(elapsed_time), 1.);
    while (time_remainder_ + 1.e-9 >= step)
    {
        Step(step);
        time_remainder_ = std::max(0., time_remainder_ - step);
    }
    if (!settings_.looping && emitter_age_ + time_epsilon >= settings_.duration && particles_.empty())
        playing_ = false;
}

void ParticleSystem::Step(double elapsed_time)
{
    auto const dt = static_cast<float>(elapsed_time);
    for (auto& particle : particles_)
    {
        particle.age += dt;
        particle.position += particle.velocity * dt + particle.acceleration * (0.5f * dt * dt);
        particle.velocity += particle.acceleration * dt;
        particle.rotation = std::remainder(particle.rotation + particle.angular_speed * dt, 2.f * pi);
        auto const fraction = std::clamp(particle.age / particle.lifetime, 0.f, 1.f);
        particle.size = (settings_.start_size + (settings_.end_size - settings_.start_size) * fraction) * particle.size_scale;
        particle.color = settings_.start_color + (settings_.end_color - settings_.start_color) * fraction;
    }
    std::erase_if(particles_, [](Particle const& particle) { return particle.age >= particle.lifetime; });

    auto const emission_time = settings_.looping ? elapsed_time :
        std::clamp(static_cast<double>(settings_.duration) - emitter_age_, 0., elapsed_time);
    emitter_age_ += elapsed_time;
    emission_remainder_ += emission_time * settings_.emission_rate;
    auto const requested = static_cast<uint32_t>(emission_remainder_ + 1.e-7);
    emission_remainder_ = std::max(0., emission_remainder_ - requested);
    // Drop excess births at capacity; never accumulate a burst while the pool is full.
    EmitBurst(requested);
    if (settings_.looping && settings_.burst_count > 0)
    {
        while (next_burst_time_ <= emitter_age_ + time_epsilon)
        {
            EmitBurst(settings_.burst_count);
            next_burst_time_ += settings_.duration;
        }
    }
}

void ParticleSystem::EmitBurst(uint32_t count)
{
    if (!Enabled() || !playing_ || count == 0) return;
    count = std::min(count, settings_.max_particles - static_cast<uint32_t>(particles_.size()));
    if (count == 0) return;
    // The getter composes the current parent transforms directly, including
    // edits made before World calls SceneNode::UpdateTransforms this frame.
    auto const transform = settings_.simulation_space == ParticleSimulationSpace::World && BoundSceneNode()
        ? BoundSceneNode()->TransformToWorld() : float4x4::Identity();
    for (uint32_t i = 0; i < count; ++i) Spawn(transform);
}

float ParticleSystem::Random()
{
    // Explicit mapping keeps seeded previews reproducible across standard libraries.
    return static_cast<float>((random_() >> 8) * (1. / 16777216.));
}

void ParticleSystem::Spawn(float4x4 const& transform)
{
    auto const axis = MathWorker::normalize(settings_.direction);
    auto const reference = std::abs(axis.y()) < 0.99f ? float3(0.f, 1.f, 0.f) : float3(1.f, 0.f, 0.f);
    auto const tangent = MathWorker::normalize(MathWorker::cross(reference, axis));
    auto const bitangent = MathWorker::cross(axis, tangent);
    auto const disk_angle = Random() * 2.f * pi;
    auto const disk_radius = std::sqrt(Random()) * settings_.radius;
    auto const azimuth = Random() * 2.f * pi;
    auto const cosine = 1.f - Random() * (1.f - std::cos(settings_.spread));
    auto const sine = std::sqrt(std::max(0.f, 1.f - cosine * cosine));
    auto const speed = settings_.speed_min + Random() * (settings_.speed_max - settings_.speed_min);
    Particle particle;
    particle.position = (tangent * std::cos(disk_angle) + bitangent * std::sin(disk_angle)) * disk_radius;
    particle.velocity = (axis * cosine + (tangent * std::cos(azimuth) + bitangent * std::sin(azimuth)) * sine) * speed;
    particle.lifetime = settings_.lifetime_min + Random() * (settings_.lifetime_max - settings_.lifetime_min);
    particle.acceleration = settings_.acceleration;
    // Keep the old random sequence for legacy emitters with no rotation.
    particle.rotation = settings_.rotation_min == settings_.rotation_max ? settings_.rotation_min :
        settings_.rotation_min + Random() * (settings_.rotation_max - settings_.rotation_min);
    particle.angular_speed = settings_.angular_speed_min == settings_.angular_speed_max ? settings_.angular_speed_min :
        settings_.angular_speed_min + Random() * (settings_.angular_speed_max - settings_.angular_speed_min);
    if (settings_.simulation_space == ParticleSimulationSpace::World)
    {
        particle.position = MathWorker::transform_coord(particle.position, transform);
        // Directions have w=0: translation must not alter velocity or gravity.
        auto vector = [&](float3 const& value) {
            return float3(transform.Row(0)) * value.x() + float3(transform.Row(1)) * value.y() +
                float3(transform.Row(2)) * value.z();
        };
        particle.velocity = vector(particle.velocity);
        particle.acceleration = vector(particle.acceleration);
        particle.size_scale = std::max({MathWorker::length(float3(transform.Row(0))),
            MathWorker::length(float3(transform.Row(1))), MathWorker::length(float3(transform.Row(2)))});
    }
    particle.size = settings_.start_size * particle.size_scale;
    particle.color = settings_.start_color;
    particles_.push_back(particle);
}
}
