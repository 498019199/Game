#include <game/SceneParticles.h>
#include <gtest/gtest.h>
#include <limits>

using namespace CommonWorker;
using namespace RenderWorker;
namespace
{
JsonValue SceneWith(JsonValue item)
{
    JsonValue scene(JsonValueType::Object), effects(JsonValueType::Array);
    effects.AppendValue(std::move(item));
    scene.AppendValue("ParticleEffects", std::move(effects));
    return scene;
}
}

TEST(SceneParticles, PresetParametersAndTransformRoundTrip)
{
    for (auto preset : {ParticlePreset::Fire, ParticlePreset::Smoke, ParticlePreset::Sparks,
        ParticlePreset::Explosion, ParticlePreset::HeatHaze})
    {
        SceneParticles::Definition definition;
        definition.name = "Effect with spaces";
        definition.settings = ParticleSettings::ForPreset(preset);
        definition.settings.seed = 0xffffffffu;
        definition.settings.looping = false;
        definition.settings.rotation_min = -.5f;
        definition.settings.rotation_max = 1.f;
        definition.settings.angular_speed_min = -2.f;
        definition.settings.angular_speed_max = 3.f;
        definition.settings.emissive_intensity = 2.5f;
        definition.settings.glow_strength = .6f;
        definition.settings.flicker_frequency = 4.f;
        definition.settings.flicker_amount = .75f;
        definition.transform = MathWorker::scaling(2.f, 3.f, 4.f) * MathWorker::translation(5.f, 6.f, 7.f);
        auto result = SceneParticles::Read(SceneWith(SceneParticles::Write(definition)));
        ASSERT_EQ(result.size(), 1u);
        EXPECT_EQ(result[0].name, definition.name);
        EXPECT_TRUE(result[0].settings == definition.settings);
        for (size_t i = 0; i < 16; ++i) EXPECT_FLOAT_EQ(result[0].transform[i], definition.transform[i]);
    }
}

TEST(SceneParticles, LegacySceneHasNoEffects)
{
    JsonValue scene(JsonValueType::Object);
    scene.AppendValue("GameObjects", JsonValue(JsonValueType::Array));
    EXPECT_TRUE(SceneParticles::Read(scene).empty());
}

TEST(SceneParticles, UnknownEffectAndSettingsFieldsSurviveEdits)
{
    auto item = SceneParticles::Write(SceneParticles::Definition{});
    item.AppendValue("FutureEffectOption", JsonValue(std::string_view("keep")));
    item.Member("Settings")->AppendValue("FutureParticleOption", JsonValue(42));
    auto parsed = SceneParticles::Read(SceneWith(item));
    ASSERT_EQ(parsed.size(), 1u);
    parsed[0].settings.emission_rate = 120;
    auto saved = SceneParticles::Write(parsed[0]);
    EXPECT_EQ(saved.Member("FutureEffectOption")->ValueString(), "keep");
    EXPECT_EQ(saved.Member("Settings")->Member("FutureParticleOption")->ValueInt(), 42);
    EXPECT_FLOAT_EQ(saved.Member("Settings")->Member("EmissionRate")->ValueFloat(), 120);
    EXPECT_FLOAT_EQ(item.Member("Settings")->Member("EmissionRate")->ValueFloat(), 90);
}

TEST(SceneParticles, MalformedArraysTypesAndRangesAreRejected)
{
    JsonValue scene(JsonValueType::Object);
    scene.AppendValue("ParticleEffects", JsonValue(42));
    EXPECT_THROW(SceneParticles::Read(scene), std::runtime_error);
    EXPECT_THROW(SceneParticles::Read(SceneWith(JsonValue(42))), std::runtime_error);
    auto malformed = [](char const* key, JsonValue value, bool in_settings = true) {
        auto item = SceneParticles::Write(SceneParticles::Definition{});
        auto field = in_settings ? item.Member("Settings")->Member(key) : item.Member(key);
        *field = std::move(value);
        EXPECT_THROW(SceneParticles::Read(SceneWith(item)), std::runtime_error) << key;
    };
    malformed("Name", JsonValue(std::string_view("")), false);
    malformed("Matrix", JsonValue(JsonValueType::Array), false);
    malformed("Settings", JsonValue(1), false);
    malformed("MaxParticles", JsonValue(-1));
    malformed("MaxParticles", JsonValue(0));
    malformed("MaxParticles", JsonValue(16385));
    malformed("EmissionRate", JsonValue(-1.f));
    malformed("LifetimeMin", JsonValue(2.f)); // Exceeds the default maximum.
    malformed("Looping", JsonValue(1));
    malformed("Blend", JsonValue(std::string_view("invalid")));
    malformed("Direction", JsonValue(JsonValueType::Array));
    malformed("SpeedMin", JsonValue(std::numeric_limits<float>::infinity()));
}

TEST(SceneParticles, NonFiniteTransformCannotBeSaved)
{
    SceneParticles::Definition definition;
    definition.transform[0] = std::numeric_limits<float>::quiet_NaN();
    EXPECT_THROW(SceneParticles::Write(definition), std::runtime_error);
}

TEST(SceneParticles, MalformedNewParticleSettingsAreRejected)
{
    auto malformed = [](char const* key, JsonValue value) {
        auto item = SceneParticles::Write(SceneParticles::Definition{});
        auto field = item.Member("Settings")->Member(key);
        ASSERT_NE(field, nullptr) << key;
        *field = std::move(value);
        EXPECT_THROW(SceneParticles::Read(SceneWith(item)), std::runtime_error) << key;
    };
    malformed("BurstCount", JsonValue(-1));
    malformed("BurstCount", JsonValue(16385u));
    malformed("BurstCount", JsonValue(std::numeric_limits<uint32_t>::max()));
    malformed("BurstCount", JsonValue(1.5f));
    malformed("BurstCount", JsonValue(true));
    malformed("SimulationSpace", JsonValue(std::string_view("invalid")));
    malformed("SimulationSpace", JsonValue(1));

    struct Range { char const* key; float minimum, maximum; };
    constexpr float pi = 3.14159265358979323846f;
    Range const ranges[] = {
        {"RotationMin", -2.f * pi, 2.f * pi},
        {"RotationMax", -2.f * pi, 2.f * pi},
        {"AngularSpeedMin", -20.f * pi, 20.f * pi},
        {"AngularSpeedMax", -20.f * pi, 20.f * pi},
        {"VelocityStretch", 0.f, 10.f},
        {"EmissiveIntensity", 0.f, 16.f},
        {"GlowStrength", 0.f, 4.f},
        {"FlickerFrequency", 0.f, 60.f},
        {"FlickerAmount", 0.f, 1.f},
        {"DistortionStrength", 0.f, 64.f},
    };
    for (auto const& range : ranges)
    {
        SCOPED_TRACE(range.key);
        malformed(range.key, JsonValue(range.minimum - 1.f));
        malformed(range.key, JsonValue(range.maximum + 1.f));
        malformed(range.key, JsonValue(std::numeric_limits<float>::infinity()));
        malformed(range.key, JsonValue(-std::numeric_limits<float>::infinity()));
        malformed(range.key, JsonValue(std::numeric_limits<float>::quiet_NaN()));
        malformed(range.key, JsonValue(std::string_view("1")));
    }
    // These values are individually valid, but invert the default [0, 0] ranges.
    malformed("RotationMin", JsonValue(.5f));
    malformed("RotationMax", JsonValue(-.5f));
    malformed("AngularSpeedMin", JsonValue(.5f));
    malformed("AngularSpeedMax", JsonValue(-.5f));
}

TEST(SceneParticles, SingularAndProjectiveTransformsAreRejected)
{
    auto item = SceneParticles::Write(SceneParticles::Definition{});
    item.Member("Matrix")->ValueIndex(0, JsonValue(0.f));
    EXPECT_THROW(SceneParticles::Read(SceneWith(item)), std::runtime_error);
    item = SceneParticles::Write(SceneParticles::Definition{});
    item.Member("Matrix")->ValueIndex(3, JsonValue(1.f));
    EXPECT_THROW(SceneParticles::Read(SceneWith(item)), std::runtime_error);
}

TEST(SceneParticles, MissingNewFieldsPreserveLegacyParticleBehavior)
{
    JsonValue item(JsonValueType::Object), settings(JsonValueType::Object);
    settings.AppendValue("EmissionRate", JsonValue(12.f));
    settings.AppendValue("Blend", JsonValue(std::string_view("Additive")));
    item.AppendValue("Settings", std::move(settings));
    auto parsed = SceneParticles::Read(SceneWith(std::move(item)));
    ASSERT_EQ(parsed.size(), 1u);
    auto const& s = parsed.front().settings;
    EXPECT_EQ(s.simulation_space, ParticleSimulationSpace::Local);
    EXPECT_EQ(s.burst_count, 0u);
    EXPECT_EQ(s.rotation_min, 0.f);
    EXPECT_EQ(s.angular_speed_max, 0.f);
    EXPECT_EQ(s.velocity_stretch, 0.f);
    EXPECT_EQ(s.emissive_intensity, 1.f);
    EXPECT_EQ(s.glow_strength, 0.f);
    EXPECT_EQ(s.flicker_frequency, 0.f);
    EXPECT_EQ(s.distortion_strength, 0.f);
}
