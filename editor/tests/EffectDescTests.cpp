#include <render/EffectDesc.h>
#include <render/ParticleEffect.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include <utility>

using namespace RenderWorker;

TEST(EffectDescription, RejectsDuplicateParametersAndInvalidLayoutsBeforeLoading)
{
    auto desc = ParticleEffect::Describe();
    desc.parameters.push_back(desc.parameters.front());
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.parameters.front().constant_buffer = "invalid buffer";
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc.parameters.front().constant_buffer = desc.parameters.front().name;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.parameters.front().type = static_cast<EffectParameterType>(999);
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
}

TEST(EffectDescription, RejectsMissingAndConflictingStages)
{
    auto desc = ParticleEffect::Describe();
    desc.techniques.front().passes.front().entry_points[std::to_underlying(ShaderStage::Vertex)].clear();
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.techniques.front().passes.front().entry_points[std::to_underlying(ShaderStage::Compute)] = "ComputeMain";
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.techniques.front().passes.front().entry_points[std::to_underlying(ShaderStage::Hull)] = "HullMain";
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.techniques.front().passes.clear();
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
}

TEST(EffectDescription, SupportsDepthOnlyAndComputePassDescriptions)
{
    auto desc = ParticleEffect::Describe();
    auto& entries = desc.techniques.front().passes.front().entry_points;
    entries[std::to_underlying(ShaderStage::Pixel)].clear();
    EXPECT_NO_THROW(desc.Validate());
    entries[std::to_underlying(ShaderStage::Vertex)].clear();
    entries[std::to_underlying(ShaderStage::Compute)] = "ComputeMain";
    EXPECT_NO_THROW(desc.Validate());
}

TEST(EffectDescription, InvalidatesCacheForBindingShaderAndPipelineChanges)
{
    auto const original = ParticleEffect::Describe();
    auto const fingerprint = original.Fingerprint();
    EXPECT_EQ(fingerprint, ParticleEffect::Describe().Fingerprint());
    auto changed = original;
    changed.parameters.front().constant_buffer = "particle_frame";
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.parameters.back().sampler.filter = TFO_Min_Mag_Mip_Point;
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.techniques.front().passes.front().depth_stencil.depth_write_mask = true;
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.techniques.front().passes.front().blend.dest_blend[0] = ABF_One;
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.techniques.front().passes.front().entry_points[std::to_underlying(ShaderStage::Pixel)] = "OtherPS";
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.shader_files.push_back("ParticleHelpers.hlsli");
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.macros.emplace_back("PARTICLE_VARIANT", "1");
    EXPECT_NE(fingerprint, changed.Fingerprint());
    EXPECT_NE(original.CacheName("d3d_11_0"), original.CacheName("d3d_12"));
}

TEST(EffectDescription, RejectsAmbiguousNamesAndEmptySources)
{
    auto desc = ParticleEffect::Describe();
    desc.techniques.push_back(desc.techniques.front());
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.techniques.front().passes.push_back(desc.techniques.front().passes.front());
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.shader_files.clear();
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.name = "../Particle";
    EXPECT_THROW(desc.CacheName("d3d_12"), std::invalid_argument);
}

namespace
{
EffectDesc StructuredEffectDescription()
{
    auto desc = ParticleEffect::Describe();
    desc.structs = {
        {"FrameState", {{"position", EffectParameterType::Float4}, {"velocity", EffectParameterType::Float4}}},
        {"FrameGroup", {{"frames", EffectParameterType::Struct, "FrameState", 4}, {"indices", EffectParameterType::UInt2}}}
    };
    EffectParameterDesc parameter;
    parameter.name = "frame_groups";
    parameter.type = EffectParameterType::Struct;
    parameter.struct_type = "FrameGroup";
    parameter.array_size = 2;
    desc.parameters.push_back(parameter);
    return desc;
}
}

TEST(EffectDescription, InitialValuesChangeFingerprintAndCacheIdentity)
{
    auto translated = float4x4::Identity();
    translated(3, 0) = 5.0f;
    struct InitializerCase
    {
        EffectParameterType type;
        EffectInitialValue first;
        EffectInitialValue second;
    };
    InitializerCase const cases[] = {
        {EffectParameterType::Bool, false, true},
        {EffectParameterType::UInt, uint32_t{1}, uint32_t{2}},
        {EffectParameterType::Int, int32_t{-1}, int32_t{1}},
        {EffectParameterType::Float, 0.25f, 0.75f},
        {EffectParameterType::Float4, float4(1, 2, 3, 4), float4(1, 2, 3, 5)},
        {EffectParameterType::Int2, int2(-1, 2), int2(-1, 3)},
        {EffectParameterType::UInt3, uint3(1, 2, 3), uint3(1, 2, 4)},
        {EffectParameterType::Float4x4, float4x4::Identity(), translated}
    };
    for (auto const& value : cases)
    {
        SCOPED_TRACE(static_cast<int>(value.type));
        auto original = ParticleEffect::Describe();
        original.parameters.push_back({"initial_value_probe", value.type});
        auto const without_initializer = original.Fingerprint();
        original.parameters.back().initial_value = value.first;
        ASSERT_NO_THROW(original.Validate());
        EXPECT_NE(without_initializer, original.Fingerprint());
        auto changed = original;
        EXPECT_EQ(original.Fingerprint(), changed.Fingerprint());
        changed.parameters.back().initial_value = value.second;
        ASSERT_NO_THROW(changed.Validate());
        EXPECT_NE(original.Fingerprint(), changed.Fingerprint());
        EXPECT_NE(original.CacheName("d3d_12"), changed.CacheName("d3d_12"));
    }
}

TEST(EffectDescription, StructMemberOrderTypeAndArrayCountsInvalidateCache)
{
    auto const original = StructuredEffectDescription();
    ASSERT_NO_THROW(original.Validate());
    auto const fingerprint = original.Fingerprint();
    EXPECT_EQ(fingerprint, StructuredEffectDescription().Fingerprint());
    auto changed = original;
    std::swap(changed.structs.front().members[0], changed.structs.front().members[1]);
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.structs.front().members.front().type = EffectParameterType::Float3;
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.structs.back().members.front().array_size = 8;
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.parameters.back().array_size = 3;
    EXPECT_NE(fingerprint, changed.Fingerprint());
    EXPECT_NE(original.CacheName("d3d_12"), changed.CacheName("d3d_12"));
}

TEST(EffectDescription, TechniqueAndPassMacrosHaveDistinctCacheIdentity)
{
    auto original = ParticleEffect::Describe();
    original.macros.emplace_back("VARIANT", "0");
    original.techniques.front().macros.emplace_back("VARIANT", "1");
    original.techniques.front().passes.front().macros.emplace_back("VARIANT", "2");
    ASSERT_NO_THROW(original.Validate()); // The same macro may be overridden in a narrower scope.
    auto const fingerprint = original.Fingerprint();
    auto changed = original;
    changed.techniques.front().macros.front().second = "3";
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.techniques.front().passes.front().macros.front().second = "3";
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.techniques.front().macros.front().first = "OTHER_VARIANT";
    EXPECT_NE(fingerprint, changed.Fingerprint());
    changed = original;
    changed.techniques.front().passes.front().macros.front().first = "OTHER_VARIANT";
    EXPECT_NE(fingerprint, changed.Fingerprint());
    auto technique_scoped = ParticleEffect::Describe();
    technique_scoped.techniques.front().macros.emplace_back("VARIANT", "1");
    auto pass_scoped = ParticleEffect::Describe();
    pass_scoped.techniques.front().passes.front().macros.emplace_back("VARIANT", "1");
    EXPECT_NE(technique_scoped.Fingerprint(), pass_scoped.Fingerprint());
}

TEST(EffectDescription, RejectsUnknownStructTypesAndInvalidDependencies)
{
    auto desc = StructuredEffectDescription();
    desc.parameters.back().struct_type = "MissingGroup";
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.structs.back().members.front().struct_type = "MissingState";
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    std::swap(desc.structs[0], desc.structs[1]);
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.structs.front().members.front().type = static_cast<EffectParameterType>(999);
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.structs.front().members.front().type = EffectParameterType::Texture2D;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
}

TEST(EffectDescription, RejectsMismatchedAndUnsupportedInitializers)
{
    auto desc = ParticleEffect::Describe();
    desc.parameters.push_back({"initial_value_probe", EffectParameterType::Float});
    auto& parameter = desc.parameters.back();
    parameter.initial_value = 1.0f;
    ASSERT_NO_THROW(desc.Validate());
    parameter.initial_value = float4(1, 1, 1, 1);
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    parameter.type = EffectParameterType::UInt;
    parameter.initial_value = int32_t{1};
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    parameter.type = EffectParameterType::Int2;
    parameter.initial_value = float2(1, 2);
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    parameter.type = EffectParameterType::Float;
    parameter.initial_value = 1.0f;
    parameter.array_size = 2;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    parameter.array_size = 0;
    parameter.type = EffectParameterType::Texture2D;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.parameters.back().array_size = 0;
    desc.parameters.back().initial_value = 1.0f;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
}

TEST(EffectDescription, RejectsDuplicateStructMemberAndMacroNames)
{
    auto desc = StructuredEffectDescription();
    desc.structs.push_back(desc.structs.front());
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.structs.front().members.push_back(desc.structs.front().members.front());
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.parameters.back().name = desc.structs.front().name;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = StructuredEffectDescription();
    desc.parameters.back().constant_buffer = desc.structs.front().name;
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.macros = {{"VARIANT", "1"}, {"VARIANT", "2"}};
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.techniques.front().macros = {{"VARIANT", "1"}, {"VARIANT", "2"}};
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
    desc = ParticleEffect::Describe();
    desc.techniques.front().passes.front().macros = {{"VARIANT", "1"}, {"VARIANT", "2"}};
    EXPECT_THROW(desc.Validate(), std::invalid_argument);
}
