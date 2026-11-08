#include <base/ZEngine.h>
#include <world/ParticleSystem.h>
#include <world/SceneNode.h>
#include <gtest/gtest.h>
#include <cmath>
#include <limits>

using namespace RenderWorker;
namespace
{
void Advance(ParticleSystem& system, int frames = 60, float delta = 1.f / 60.f)
{
    for (int i = 0; i < frames; ++i) system.Update(delta);
}
void ExpectSameParticles(std::vector<Particle> const& a, std::vector<Particle> const& b)
{
    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        EXPECT_EQ(a[i].position, b[i].position);
        EXPECT_EQ(a[i].velocity, b[i].velocity);
        EXPECT_EQ(a[i].color, b[i].color);
        EXPECT_EQ(a[i].size, b[i].size);
        EXPECT_EQ(a[i].age, b[i].age);
        EXPECT_EQ(a[i].lifetime, b[i].lifetime);
        EXPECT_EQ(a[i].rotation, b[i].rotation);
        EXPECT_EQ(a[i].angular_speed, b[i].angular_speed);
        EXPECT_EQ(a[i].size_scale, b[i].size_scale);
        EXPECT_EQ(a[i].acceleration, b[i].acceleration);
    }
}
}

TEST(ParticleSystem, PresetsEmitFiniteParticlesAndRespectCapacity)
{
    for (auto preset : {ParticlePreset::Fire, ParticlePreset::Smoke, ParticlePreset::Sparks})
    {
        auto settings = ParticleSettings::ForPreset(preset);
        settings.max_particles = 12;
        settings.emission_rate = 200;
        settings.looping = true;
        ParticleSystem system(settings);
        Advance(system, 120);
        ASSERT_FALSE(system.Particles().empty());
        EXPECT_LE(system.Particles().size(), 12u);
        for (auto const& p : system.Particles())
        {
            EXPECT_GE(p.age, 0);
            EXPECT_LT(p.age, p.lifetime);
            EXPECT_TRUE(std::isfinite(p.size));
            EXPECT_GT(p.size, 0);
            for (size_t i = 0; i < 3; ++i)
            {
                EXPECT_TRUE(std::isfinite(p.position[i]));
                EXPECT_TRUE(std::isfinite(p.velocity[i]));
            }
            for (size_t i = 0; i < 4; ++i) EXPECT_TRUE(std::isfinite(p.color[i]));
            EXPECT_GE(p.color.w(), 0);
            EXPECT_LE(p.color.w(), 1);
        }
    }
}

TEST(ParticleSystem, PauseFreezesSimulationAndPlayResumes)
{
    ParticleSystem system(ParticleSettings::ForPreset(ParticlePreset::Fire));
    Advance(system, 20);
    ASSERT_FALSE(system.Particles().empty());
    system.Pause();
    auto const paused = system.Particles();
    Advance(system);
    EXPECT_FALSE(system.IsPlaying());
    ExpectSameParticles(paused, system.Particles());
    system.Play();
    EXPECT_TRUE(system.IsPlaying());
    system.Update(1.f / 60.f);
    ASSERT_FALSE(system.Particles().empty());
    EXPECT_NE(paused.front().age, system.Particles().front().age);
}

TEST(ParticleSystem, StopClearsAndRestartReplaysTheSeed)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Sparks);
    settings.seed = 123456;
    ParticleSystem system(settings);
    Advance(system, 20);
    auto const first = system.Particles();
    ASSERT_FALSE(first.empty());
    system.Stop();
    EXPECT_FALSE(system.IsPlaying());
    EXPECT_TRUE(system.Particles().empty());
    Advance(system);
    EXPECT_TRUE(system.Particles().empty());
    system.Restart();
    Advance(system, 20);
    ExpectSameParticles(first, system.Particles());
}

TEST(ParticleSystem, NonLoopingEmissionFinishesAndRemainingParticlesExpire)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Fire);
    settings.looping = false;
    settings.duration = 0.2f;
    settings.lifetime_min = settings.lifetime_max = 0.3f;
    settings.emission_rate = 100;
    ParticleSystem system(settings);
    Advance(system, 5);
    ASSERT_FALSE(system.Particles().empty());
    Advance(system, 20);
    EXPECT_FALSE(system.IsEmitting());
    Advance(system, 60);
    EXPECT_TRUE(system.Particles().empty());
    EXPECT_FALSE(system.IsPlaying());
    system.Play();
    Advance(system, 5);
    EXPECT_FALSE(system.Particles().empty());
}

TEST(ParticleSystem, ZeroEmissionAndInvalidTimeDoNotCreateParticles)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Smoke);
    settings.emission_rate = 0;
    ParticleSystem system(settings);
    Advance(system);
    EXPECT_TRUE(system.Particles().empty());
    settings.emission_rate = 100;
    system.Settings(settings);
    for (auto dt : {0.f, -1.f, std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()})
        system.Update(dt);
    EXPECT_TRUE(system.Particles().empty());
    Advance(system, 2);
    EXPECT_FALSE(system.Particles().empty());
}

TEST(ParticleSystem, InvalidSettingsAreSanitizedBeforeSimulation)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Fire);
    settings.max_particles = 0;
    settings.emission_rate = std::numeric_limits<float>::infinity();
    settings.lifetime_min = -5;
    settings.lifetime_max = std::numeric_limits<float>::quiet_NaN();
    settings.start_size = -2;
    settings.end_size = std::numeric_limits<float>::quiet_NaN();
    settings.direction = float3(0, 0, 0);
    settings.start_color = float4(-1, 20, 1, 5);
    ParticleSystem system(settings);
    auto const& sanitized = system.Settings();
    EXPECT_GE(sanitized.max_particles, 1u);
    EXPECT_TRUE(std::isfinite(sanitized.emission_rate));
    EXPECT_GE(sanitized.lifetime_min, 0.01f);
    EXPECT_GE(sanitized.lifetime_max, sanitized.lifetime_min);
    EXPECT_GT(sanitized.start_size, 0);
    EXPECT_TRUE(std::isfinite(sanitized.end_size));
    EXPECT_GE(sanitized.start_color.x(), 0);
    EXPECT_LE(sanitized.start_color.y(), 16);
    EXPECT_LE(sanitized.start_color.w(), 1);
    Advance(system);
    EXPECT_LE(system.Particles().size(), sanitized.max_particles);
}

TEST(ParticleSystem, SettingChangesResetSimulationButRetainPause)
{
    ParticleSystem system(ParticleSettings::ForPreset(ParticlePreset::Fire));
    Advance(system, 10);
    system.Pause();
    system.Settings(ParticleSettings::ForPreset(ParticlePreset::Smoke));
    EXPECT_FALSE(system.IsPlaying());
    EXPECT_TRUE(system.Particles().empty());
    system.Play();
    Advance(system, 20);
    EXPECT_FALSE(system.Particles().empty());
}

TEST(ParticleSystem, FixedStepsProduceSameResultAtDifferentFrameRates)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Sparks);
    ParticleSystem slow(settings), fast(settings);
    Advance(slow, 30, 1.f / 30.f);
    Advance(fast, 120, 1.f / 120.f);
    ASSERT_FALSE(slow.Particles().empty());
    ExpectSameParticles(slow.Particles(), fast.Particles());
}

TEST(ParticleSystem, DisabledComponentDoesNotAdvance)
{
    ParticleSystem system;
    Advance(system, 10);
    auto const particles = system.Particles();
    system.Enabled(false);
    Advance(system);
    ExpectSameParticles(particles, system.Particles());
    system.Enabled(true);
    Advance(system, 2);
    EXPECT_NE(particles.front().age, system.Particles().front().age);
}

TEST(ParticleSystem, BurstEmitsOnceThenCanRestartWithoutContinuousEmission)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Explosion);
    settings.burst_count = 17;
    settings.lifetime_min = settings.lifetime_max = 2;
    ParticleSystem system(settings);
    system.Update(0); // Invalid/empty ticks must not fire the pending burst.
    EXPECT_TRUE(system.Particles().empty());
    system.Update(1.f / 120.f);
    EXPECT_EQ(system.Particles().size(), 17u);
    Advance(system, 30);
    EXPECT_EQ(system.Particles().size(), 17u);
    EXPECT_FALSE(system.IsEmitting());
    Advance(system, 180);
    EXPECT_TRUE(system.Particles().empty());
    EXPECT_FALSE(system.IsPlaying());
    system.Play();
    system.Update(1.f / 120.f);
    EXPECT_EQ(system.Particles().size(), 17u);
}

TEST(ParticleSystem, LoopingBurstsFollowDurationAndAreFrameRateIndependent)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Explosion);
    settings.burst_count = 5;
    settings.duration = .1f;
    settings.looping = true;
    settings.lifetime_min = settings.lifetime_max = 5;
    settings.rotation_min = -1;
    settings.rotation_max = 1;
    settings.angular_speed_min = -3;
    settings.angular_speed_max = 3;
    ParticleSystem slow(settings), fast(settings);
    Advance(fast, 11, 1.f / 120.f);
    EXPECT_EQ(fast.Particles().size(), 5u);
    Advance(fast, 1, 1.f / 120.f);
    EXPECT_EQ(fast.Particles().size(), 10u);
    Advance(fast, 108, 1.f / 120.f);
    Advance(slow, 30, 1.f / 30.f);
    ExpectSameParticles(slow.Particles(), fast.Particles());
}

TEST(ParticleSystem, ManualBurstHonorsPlaybackAndCapacity)
{
    auto settings = ParticleSettings::ForPreset(ParticlePreset::Fire);
    settings.emission_rate = 0;
    settings.max_particles = 8;
    ParticleSystem system(settings);
    system.EmitBurst(3);
    EXPECT_EQ(system.Particles().size(), 3u);
    system.Pause();
    system.EmitBurst(8);
    EXPECT_EQ(system.Particles().size(), 3u);
    system.Play();
    system.Enabled(false);
    system.EmitBurst(8);
    EXPECT_EQ(system.Particles().size(), 3u);
    system.Enabled(true);
    system.EmitBurst(std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(system.Particles().size(), 8u);
    system.Stop();
    system.EmitBurst(3);
    EXPECT_TRUE(system.Particles().empty());
}

TEST(ParticleSystem, WorldParticlesKeepTheirBirthTransformWhenEmitterMoves)
{
    ParticleSettings settings;
    settings.simulation_space = ParticleSimulationSpace::World;
    settings.emission_rate = 0;
    settings.radius = 0;
    settings.speed_min = settings.speed_max = 0;
    settings.acceleration = float3::Zero();
    settings.start_size = settings.end_size = 1;
    auto system = std::make_shared<ParticleSystem>(settings);
    auto node = std::make_shared<SceneNode>(system, SceneNode::SOA_Moveable);
    node->TransformToParent(MathWorker::scaling(2.f, 3.f, 4.f) * MathWorker::translation(3.f, 2.f, 1.f));
    system->EmitBurst(1);
    ASSERT_EQ(system->Particles().size(), 1u);
    EXPECT_EQ(system->Particles()[0].position, float3(3, 2, 1));
    EXPECT_FLOAT_EQ(system->Particles()[0].size, 4);
    node->TransformToParent(MathWorker::translation(9.f, 8.f, 7.f));
    system->MainThreadUpdate(1, 1.f / 120.f);
    system->EmitBurst(1);
    ASSERT_EQ(system->Particles().size(), 2u);
    EXPECT_EQ(system->Particles()[0].position, float3(3, 2, 1));
    EXPECT_FLOAT_EQ(system->Particles()[0].size, 4);
    EXPECT_EQ(system->Particles()[1].position, float3(9, 8, 7));
    EXPECT_FLOAT_EQ(system->Particles()[1].size, 1);
}

TEST(ParticleSystem, WorldVelocityAndAccelerationIncludeParentTransformAtBirth)
{
    ParticleSettings settings;
    settings.simulation_space = ParticleSimulationSpace::World;
    settings.emission_rate = settings.radius = settings.spread = 0;
    settings.direction = float3(1, 0, 0);
    settings.speed_min = settings.speed_max = 2;
    settings.acceleration = float3(0, 1, 0);
    auto system = std::make_shared<ParticleSystem>(settings);
    auto parent = std::make_shared<SceneNode>(SceneNode::SOA_Moveable);
    auto node = std::make_shared<SceneNode>(system, SceneNode::SOA_Moveable);
    parent->AddChild(node);
    parent->TransformToParent(MathWorker::scaling(2.f, 3.f, 4.f));
    system->EmitBurst(1);
    ASSERT_EQ(system->Particles().size(), 1u);
    // Direction normalization uses the engine's approximate reciprocal square root.
    EXPECT_NEAR(system->Particles()[0].velocity.x(), 4, 1e-4f);
    EXPECT_FLOAT_EQ(system->Particles()[0].velocity.y(), 0);
    EXPECT_FLOAT_EQ(system->Particles()[0].velocity.z(), 0);
    EXPECT_EQ(system->Particles()[0].acceleration, float3(0, 3, 0));
    parent->TransformToParent(float4x4::Identity());
    system->Update(1.f / 120.f);
    EXPECT_NEAR(system->Particles()[0].velocity.y(), 3.f / 120.f, 1e-6f);
}

TEST(ParticleSystem, SpinIntegratesOverTimeAndPauses)
{
    ParticleSettings settings;
    settings.emission_rate = 0;
    settings.rotation_min = settings.rotation_max = .2f;
    settings.angular_speed_min = settings.angular_speed_max = 2;
    settings.lifetime_min = settings.lifetime_max = 5;
    ParticleSystem system(settings);
    system.EmitBurst(1);
    system.Update(.25f);
    ASSERT_EQ(system.Particles().size(), 1u);
    EXPECT_NEAR(system.Particles()[0].rotation, .7f, 1e-5f);
    system.Pause();
    system.Update(1);
    EXPECT_NEAR(system.Particles()[0].rotation, .7f, 1e-5f);
}
