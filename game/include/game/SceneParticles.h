#pragma once

#include <common/JsonDom.h>
#include <world/ParticleSystem.h>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

// Particle documents are independent of GPU resources, so malformed scene data
// can be rejected before replacing the live scene.
namespace SceneParticles
{
using namespace RenderWorker;
using CommonWorker::JsonValue;
using CommonWorker::JsonValueType;

struct Definition
{
    std::string name = "Particles";
    float4x4 transform = float4x4::Identity();
    ParticleSettings settings;
    JsonValue document {JsonValueType::Object};
};

inline void ValidateTransform(float4x4 const& matrix)
{
    for (float value : matrix) if (!std::isfinite(value)) throw std::runtime_error("Particle Matrix must be finite");
    float3 x(matrix(0,0), matrix(0,1), matrix(0,2));
    float3 y(matrix(1,0), matrix(1,1), matrix(1,2));
    float3 z(matrix(2,0), matrix(2,1), matrix(2,2));
    float const size = MathWorker::length(x) * MathWorker::length(y) * MathWorker::length(z);
    if (!std::isfinite(size) || size < 1e-12f || std::abs(MathWorker::dot(x, MathWorker::cross(y,z))) <= size * 1e-6f
        || std::abs(matrix(0,3)) > 1e-6f || std::abs(matrix(1,3)) > 1e-6f
        || std::abs(matrix(2,3)) > 1e-6f || std::abs(matrix(3,3) - 1.f) > 1e-6f)
        throw std::runtime_error("Particle Matrix must be an invertible affine transform");
}

inline float Number(JsonValue const& value)
{
    float result;
    switch (value.Type()) {
    case JsonValueType::Float: result = value.ValueFloat(); break;
    case JsonValueType::Int: result = static_cast<float>(value.ValueInt()); break;
    case JsonValueType::UInt: result = static_cast<float>(value.ValueUInt()); break;
    default: throw std::runtime_error("Particle parameter must be a number");
    }
    if (!std::isfinite(result)) throw std::runtime_error("Particle parameter must be finite");
    return result;
}

inline std::vector<Definition> Read(JsonValue const& scene)
{
    std::vector<Definition> result;
    auto effects = scene.Member("ParticleEffects");
    if (!effects) return result;
    if (effects->Type() != JsonValueType::Array) throw std::runtime_error("ParticleEffects must be an array");
    for (auto const& item : effects->ValueArray()) {
        if (item.Type() != JsonValueType::Object) throw std::runtime_error("Particle effect must be an object");
        Definition effect;
        effect.document = item;
        if (auto name = item.Member("Name")) {
            if (name->Type() != JsonValueType::String || name->ValueString().empty())
                throw std::runtime_error("Particle effect Name must be a non-empty string");
            effect.name = name->ValueString();
        }
        if (auto matrix = item.Member("Matrix")) {
            if (matrix->Type() != JsonValueType::Array || matrix->ValueArray().size() != 16)
                throw std::runtime_error("Particle Matrix must have 16 numbers");
            for (size_t i = 0; i < 16; ++i) effect.transform[i] = Number(matrix->ValueArray()[i]);
            ValidateTransform(effect.transform);
        }
        if (auto settings = item.Member("Settings")) {
            if (settings->Type() != JsonValueType::Object) throw std::runtime_error("Particle Settings must be an object");
            auto scalar = [&](char const* name, float& target, float minimum, float maximum) {
                if (auto value = settings->Member(name)) {
                    target = Number(*value);
                    if (target < minimum || target > maximum) throw std::runtime_error(std::string("Particle parameter out of range: ") + name);
                }
            };
            auto integer = [&](char const* name, uint32_t& target) {
                if (auto value = settings->Member(name)) {
                    if (value->Type() == JsonValueType::UInt) target = value->ValueUInt();
                    else if (value->Type() == JsonValueType::Int && value->ValueInt() >= 0) target = static_cast<uint32_t>(value->ValueInt());
                    else throw std::runtime_error(std::string("Particle parameter must be a non-negative integer: ") + name);
                }
            };
            auto vector = [&](char const* name, auto& target, size_t size) {
                if (auto value = settings->Member(name)) {
                    if (value->Type() != JsonValueType::Array || value->ValueArray().size() != size)
                        throw std::runtime_error(std::string("Invalid particle vector: ") + name);
                    for (size_t i = 0; i < size; ++i) target[i] = Number(value->ValueArray()[i]);
                }
            };
            auto& s = effect.settings;
            integer("MaxParticles", s.max_particles);
            integer("Seed", s.seed);
            integer("BurstCount", s.burst_count);
            if (s.max_particles < 1 || s.max_particles > 16384) throw std::runtime_error("MaxParticles must be between 1 and 16384");
            if (s.burst_count > 16384) throw std::runtime_error("BurstCount must not exceed 16384");
            scalar("EmissionRate", s.emission_rate, 0, 50000);
            scalar("LifetimeMin", s.lifetime_min, .01f, 60);
            scalar("LifetimeMax", s.lifetime_max, .01f, 60);
            scalar("SpeedMin", s.speed_min, 0, 1000);
            scalar("SpeedMax", s.speed_max, 0, 1000);
            scalar("Spread", s.spread, 0, 3.141593f);
            scalar("Radius", s.radius, 0, 1000);
            scalar("StartSize", s.start_size, .001f, 100);
            scalar("EndSize", s.end_size, .001f, 100);
            scalar("Duration", s.duration, .01f, 3600);
            constexpr float pi = 3.14159265358979323846f;
            scalar("RotationMin", s.rotation_min, -2.f * pi, 2.f * pi);
            scalar("RotationMax", s.rotation_max, -2.f * pi, 2.f * pi);
            scalar("AngularSpeedMin", s.angular_speed_min, -20.f * pi, 20.f * pi);
            scalar("AngularSpeedMax", s.angular_speed_max, -20.f * pi, 20.f * pi);
            scalar("VelocityStretch", s.velocity_stretch, 0, 10);
            scalar("EmissiveIntensity", s.emissive_intensity, 0, 16);
            scalar("GlowStrength", s.glow_strength, 0, 4);
            scalar("FlickerFrequency", s.flicker_frequency, 0, 60);
            scalar("FlickerAmount", s.flicker_amount, 0, 1);
            scalar("DistortionStrength", s.distortion_strength, 0, 64);
            vector("Direction", s.direction, 3);
            vector("Acceleration", s.acceleration, 3);
            vector("StartColor", s.start_color, 4);
            vector("EndColor", s.end_color, 4);
            for (size_t i = 0; i < 3; ++i) {
                if (s.direction[i] < -1 || s.direction[i] > 1 || s.acceleration[i] < -1000 || s.acceleration[i] > 1000)
                    throw std::runtime_error("Particle direction or acceleration is out of range");
            }
            if (MathWorker::length(s.direction) < .0001f) throw std::runtime_error("Particle direction must be non-zero");
            for (size_t i = 0; i < 4; ++i) {
                float const maximum = i == 3 ? 1.f : 16.f;
                if (s.start_color[i] < 0 || s.start_color[i] > maximum || s.end_color[i] < 0 || s.end_color[i] > maximum)
                    throw std::runtime_error("Particle color is out of range");
            }
            if (s.lifetime_min > s.lifetime_max || s.speed_min > s.speed_max ||
                s.rotation_min > s.rotation_max || s.angular_speed_min > s.angular_speed_max)
                throw std::runtime_error("Particle minimum must not exceed maximum");
            if (auto looping = settings->Member("Looping")) {
                if (looping->Type() != JsonValueType::Bool) throw std::runtime_error("Looping must be a boolean");
                s.looping = looping->ValueBool();
            }
            if (auto blend = settings->Member("Blend")) {
                if (blend->Type() != JsonValueType::String) throw std::runtime_error("Blend must be Alpha, Additive or Distortion");
                if (blend->ValueString() == "Alpha") s.blend = ParticleBlendMode::Alpha;
                else if (blend->ValueString() == "Additive") s.blend = ParticleBlendMode::Additive;
                else if (blend->ValueString() == "Distortion") s.blend = ParticleBlendMode::Distortion;
                else throw std::runtime_error("Blend must be Alpha, Additive or Distortion");
            }
            if (auto space = settings->Member("SimulationSpace")) {
                if (space->Type() != JsonValueType::String) throw std::runtime_error("SimulationSpace must be Local or World");
                if (space->ValueString() == "Local") s.simulation_space = ParticleSimulationSpace::Local;
                else if (space->ValueString() == "World") s.simulation_space = ParticleSimulationSpace::World;
                else throw std::runtime_error("SimulationSpace must be Local or World");
            }
        }
        result.push_back(std::move(effect));
    }
    return result;
}

inline JsonValue Write(Definition const& effect)
{
    ValidateTransform(effect.transform);
    JsonValue item = effect.document;
    auto set = [](JsonValue& object, char const* name, JsonValue value) {
        if (auto field = object.Member(name)) *field = std::move(value);
        else object.AppendValue(name, std::move(value));
    };
    auto vector = [](auto const& values, size_t size) {
        JsonValue array(JsonValueType::Array);
        for (size_t i = 0; i < size; ++i) {
            if (!std::isfinite(values[i])) throw std::runtime_error("Cannot save non-finite particle values");
            array.AppendValue(JsonValue(values[i]));
        }
        return array;
    };
    set(item, "Name", JsonValue(std::string_view(effect.name)));
    set(item, "Matrix", vector(effect.transform, 16));
    auto old = item.Member("Settings");
    JsonValue settings = old ? *old : JsonValue(JsonValueType::Object);
    auto const& s = effect.settings;
    set(settings, "MaxParticles", JsonValue(s.max_particles));
    set(settings, "EmissionRate", JsonValue(s.emission_rate));
    set(settings, "LifetimeMin", JsonValue(s.lifetime_min));
    set(settings, "LifetimeMax", JsonValue(s.lifetime_max));
    set(settings, "SpeedMin", JsonValue(s.speed_min));
    set(settings, "SpeedMax", JsonValue(s.speed_max));
    set(settings, "Spread", JsonValue(s.spread));
    set(settings, "Radius", JsonValue(s.radius));
    set(settings, "StartSize", JsonValue(s.start_size));
    set(settings, "EndSize", JsonValue(s.end_size));
    set(settings, "Duration", JsonValue(s.duration));
    set(settings, "Looping", JsonValue(s.looping));
    set(settings, "Seed", JsonValue(s.seed));
    set(settings, "BurstCount", JsonValue(s.burst_count));
    set(settings, "RotationMin", JsonValue(s.rotation_min));
    set(settings, "RotationMax", JsonValue(s.rotation_max));
    set(settings, "AngularSpeedMin", JsonValue(s.angular_speed_min));
    set(settings, "AngularSpeedMax", JsonValue(s.angular_speed_max));
    set(settings, "VelocityStretch", JsonValue(s.velocity_stretch));
    set(settings, "EmissiveIntensity", JsonValue(s.emissive_intensity));
    set(settings, "GlowStrength", JsonValue(s.glow_strength));
    set(settings, "FlickerFrequency", JsonValue(s.flicker_frequency));
    set(settings, "FlickerAmount", JsonValue(s.flicker_amount));
    set(settings, "DistortionStrength", JsonValue(s.distortion_strength));
    set(settings, "SimulationSpace", JsonValue(std::string_view(s.simulation_space == ParticleSimulationSpace::Local ? "Local" : "World")));
    set(settings, "Blend", JsonValue(std::string_view(s.blend == ParticleBlendMode::Alpha ? "Alpha" :
        s.blend == ParticleBlendMode::Additive ? "Additive" : "Distortion")));
    set(settings, "Direction", vector(s.direction, 3));
    set(settings, "Acceleration", vector(s.acceleration, 3));
    set(settings, "StartColor", vector(s.start_color, 4));
    set(settings, "EndColor", vector(s.end_color, 4));
    set(item, "Settings", std::move(settings));
    return item;
}
}
