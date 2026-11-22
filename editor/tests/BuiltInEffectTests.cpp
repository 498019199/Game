#include <render/BuiltInEffects.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

using namespace RenderWorker;

namespace
{
template <typename T>
T const& Named(std::vector<T> const& descriptions, std::string_view name)
{
    auto const it = std::find_if(descriptions.begin(), descriptions.end(),
        [name](T const& description) { return description.name == name; });
    if (it == descriptions.end())
        throw std::runtime_error("Missing effect descriptor: " + std::string(name));
    return *it;
}

using Layout = std::vector<std::pair<std::string, EffectParameterType>>;

Layout BufferLayout(EffectDesc const& desc, std::string_view buffer)
{
    Layout layout;
    for (auto const& parameter : desc.parameters)
    {
        if (parameter.constant_buffer == buffer)
            layout.emplace_back(parameter.name, parameter.type);
    }
    return layout;
}

size_t MacroCount(std::vector<std::pair<std::string, std::string>> const& macros,
    std::string_view name, std::string_view value)
{
    return std::count_if(macros.begin(), macros.end(), [=](auto const& macro)
        { return macro.first == name && macro.second == value; });
}

void ExpectFloat(EffectDesc const& desc, std::string_view name, float expected)
{
    SCOPED_TRACE(std::string(name));
    auto const& parameter = Named(desc.parameters, name);
    EXPECT_EQ(EffectParameterType::Float, parameter.type);
    auto const* value = std::get_if<float>(&parameter.initial_value);
    ASSERT_NE(nullptr, value);
    EXPECT_FLOAT_EQ(expected, *value);
}

void ExpectFloat3(EffectDesc const& desc, std::string_view name, float x, float y, float z)
{
    SCOPED_TRACE(std::string(name));
    auto const& parameter = Named(desc.parameters, name);
    EXPECT_EQ(EffectParameterType::Float3, parameter.type);
    auto const* value = std::get_if<float3>(&parameter.initial_value);
    ASSERT_NE(nullptr, value);
    EXPECT_FLOAT_EQ(x, (*value)[0]);
    EXPECT_FLOAT_EQ(y, (*value)[1]);
    EXPECT_FLOAT_EQ(z, (*value)[2]);
}
}

TEST(BuiltInEffects, EveryRegisteredEffectIsValidAndUsesDirectHlslSources)
{
    std::set<std::string> const expected{
        "CameraFrustum", "CameraProxy", "Data2ShaderFx", "EditorLitV1", "GBuffer",
        "LightSourceProxy", "MLShader", "PredefinedCBuffers", "RmlUi",
        "SimpleAlbedoNormal", "SkyBox", "SubSurface", "Terrain", "Particle"};
    std::set<std::string> actual;
    for (auto const name : BuiltInEffectNames())
    {
        SCOPED_TRACE(std::string(name));
        EXPECT_TRUE(actual.emplace(name).second) << "Duplicate registry entry";
        auto const desc = DescribeBuiltInEffect(name);
        EXPECT_EQ(name, desc.name);
        EXPECT_NO_THROW(desc.Validate());
        ASSERT_FALSE(desc.shader_files.empty());
        for (auto const& file : desc.shader_files)
        {
            EXPECT_TRUE(file.ends_with(".hlsl") || file.ends_with(".hlsli")) << file;
            EXPECT_EQ(std::string::npos, file.find(".shader")) << file;
        }
    }
    EXPECT_EQ(expected, actual);
}

TEST(BuiltInEffects, SharedMaterialAndMeshBuffersKeepTheirCpuBindingOrder)
{
    using Type = EffectParameterType;
    // The CPU predefined buffers bind by reflected offsets, so type and order are ABI.
    Layout const material{
        {"albedo_clr", Type::Float4}, {"metalness_glossiness_factor", Type::Float3},
        {"emissive_clr", Type::Float4}, {"albedo_map_enabled", Type::Int},
        {"normal_map_enabled", Type::Int}, {"height_map_parallax_enabled", Type::Int},
        {"height_map_tess_enabled", Type::Int}, {"occlusion_map_enabled", Type::Int},
        {"alpha_test_threshold", Type::Float}, {"normal_scale", Type::Float},
        {"occlusion_strength", Type::Float}, {"height_offset_scale", Type::Float2},
        {"tess_factors", Type::Float4}};
    Layout const mesh{{"pos_center", Type::Float3}, {"pos_extent", Type::Float3},
        {"tc_center", Type::Float2}, {"tc_extent", Type::Float2}};
    for (auto const name : {"PredefinedCBuffers", "GBuffer"})
    {
        SCOPED_TRACE(name);
        auto const desc = DescribeBuiltInEffect(name);
        EXPECT_EQ(material, BufferLayout(desc, "klayge_material"));
        EXPECT_EQ(mesh, BufferLayout(desc, "klayge_mesh"));
        for (auto const& parameter : desc.parameters)
        {
            if (parameter.constant_buffer == "klayge_material" || parameter.constant_buffer == "klayge_mesh")
                EXPECT_EQ(0u, parameter.array_size) << parameter.name;
        }
    }
}

TEST(BuiltInEffects, SharedCameraBuffersPreserveStructMembersAndArrayExtents)
{
    using Type = EffectParameterType;
    Layout const model{{"model", Type::Float4x4}, {"inv_model", Type::Float4x4}};
    Layout const camera{{"num_cameras", Type::UInt}, {"camera_indices", Type::UInt4},
        {"cameras", Type::Struct}, {"prev_mvps", Type::Float4x4}};
    Layout const camera_info{{"model_view", Type::Float4x4}, {"mvp", Type::Float4x4},
        {"inv_mv", Type::Float4x4}, {"inv_mvp", Type::Float4x4},
        {"eye_pos", Type::Float3}, {"forward_vec", Type::Float3}, {"up_vec", Type::Float3}};
    for (auto const name : {"PredefinedCBuffers", "GBuffer"})
    {
        SCOPED_TRACE(name);
        auto const desc = DescribeBuiltInEffect(name);
        EXPECT_EQ(model, BufferLayout(desc, "klayge_model"));
        EXPECT_EQ(camera, BufferLayout(desc, "klayge_camera"));
        EXPECT_EQ(0u, Named(desc.parameters, "num_cameras").array_size);
        EXPECT_EQ(2u, Named(desc.parameters, "camera_indices").array_size);
        EXPECT_EQ(8u, Named(desc.parameters, "cameras").array_size);
        EXPECT_EQ("KlayGECameraInfo", Named(desc.parameters, "cameras").struct_type);
        EXPECT_EQ(8u, Named(desc.parameters, "prev_mvps").array_size);
        Layout members;
        for (auto const& member : Named(desc.structs, "KlayGECameraInfo").members)
        {
            members.emplace_back(member.name, member.type);
            EXPECT_EQ(0u, member.array_size) << member.name;
            EXPECT_TRUE(member.struct_type.empty()) << member.name;
        }
        EXPECT_EQ(camera_info, members);
    }
}

TEST(BuiltInEffects, MaterialEffectsRetainNonzeroNumericDefaults)
{
    auto const data = DescribeBuiltInEffect("Data2ShaderFx");
    EXPECT_EQ("WORLDVIEWPROJECTION", Named(data.parameters, "worldviewproj").semantic);
    ExpectFloat3(data, "light_dir", 0.35f, 0.85f, 0.45f);
    ExpectFloat(data, "detail_scale", 4.0f);
    ExpectFloat(data, "detail_strength", 1.0f);
    ExpectFloat(data, "detail2_scale", 8.0f);
    ExpectFloat(data, "detail2_strength", 1.0f);
    ExpectFloat(data, "cubemap_strength", 0.35f);
    ExpectFloat(data, "selfillum_strength", 1.0f);

    auto const ml = DescribeBuiltInEffect("MLShader");
    ExpectFloat3(ml, "light_dir", 0.35f, 0.85f, 0.45f);
    ExpectFloat3(ml, "emissive_color", 0.0f, 0.25f, 0.85f);
    ExpectFloat3(ml, "specular_color", 1.0f, 1.0f, 1.0f);
    ExpectFloat(ml, "selfillum_strength", 1.5f);
    ExpectFloat(ml, "specular_power", 30.0f);
    ExpectFloat(ml, "specular_strength", 1.0f);
    ExpectFloat(ml, "bloom_radius", 0.012f);
    ExpectFloat(ml, "bloom_strength", 1.25f);
    ExpectFloat(ml, "bloom_threshold", 0.05f);
}

TEST(BuiltInEffects, GBufferVariantsRetainInheritedBlendDepthAndShaderStates)
{
    auto const desc = DescribeBuiltInEffect("GBuffer");
    for (auto const name : {"SpecialShadingAlphaBlendBackMultiViewNoVpRtTech",
        "SpecialShadingAlphaBlendFrontMultiViewNoVpRtTech"})
    {
        SCOPED_TRACE(name);
        auto const& technique = Named(desc.techniques, name);
        ASSERT_EQ(1u, technique.passes.size());
        auto const& pass = technique.passes.front();
        EXPECT_EQ(std::string_view(name).find("Back") != std::string_view::npos ? CM_Front : CM_Back,
            pass.rasterizer.cull_mode);
        EXPECT_TRUE(pass.depth_stencil.depth_enable);
        EXPECT_EQ(CF_Equal, pass.depth_stencil.depth_func);
        EXPECT_FALSE(pass.depth_stencil.depth_write_mask);
        EXPECT_TRUE(pass.blend.blend_enable[0]);
        EXPECT_EQ(BOP_Add, pass.blend.blend_op[0]);
        EXPECT_EQ(ABF_One, pass.blend.src_blend[0]);
        EXPECT_EQ(ABF_One, pass.blend.dest_blend[0]);
        EXPECT_EQ(BOP_Add, pass.blend.blend_op_alpha[0]);
        EXPECT_EQ(ABF_Src_Alpha, pass.blend.src_blend_alpha[0]);
        EXPECT_EQ(ABF_Zero, pass.blend.dest_blend_alpha[0]);
        EXPECT_EQ("GBufferVS", pass.entry_points[std::to_underlying(ShaderStage::Vertex)]);
        EXPECT_EQ("SpecialShadingAlphaBlendPS", pass.entry_points[std::to_underlying(ShaderStage::Pixel)]);
        EXPECT_EQ("GBufferNoVpRtGS", pass.entry_points[std::to_underlying(ShaderStage::Geometry)]);
        EXPECT_EQ(1u, MacroCount(technique.macros, "MULTI_VIEW_MODE", "1"));
    }
    auto const& shadow = Named(desc.techniques, "GenShadowMapAlphaTestMultiViewNoVpRtTech");
    ASSERT_EQ(1u, shadow.passes.size());
    EXPECT_EQ(CM_None, shadow.passes.front().rasterizer.cull_mode);
    EXPECT_EQ(0u, shadow.passes.front().blend.color_write_mask[0]);
    EXPECT_EQ("GenShadowMapVS", shadow.passes.front().entry_points[std::to_underlying(ShaderStage::Vertex)]);
    EXPECT_EQ("GenShadowMapAlphaTestPS", shadow.passes.front().entry_points[std::to_underlying(ShaderStage::Pixel)]);
    EXPECT_EQ("GenShadowCubemapNoVpRtGS", shadow.passes.front().entry_points[std::to_underlying(ShaderStage::Geometry)]);
}

TEST(BuiltInEffects, GBufferMultiViewRetainsAllInheritedTechniqueMacros)
{
    auto const desc = DescribeBuiltInEffect("GBuffer");
    auto const& variant = Named(desc.techniques, "GenReflectiveShadowMapAlphaTestMultiViewNoVpRtTech");
    EXPECT_EQ(1u, MacroCount(variant.macros, "SKIP_MOTION_VEC", "1"));
    EXPECT_EQ(1u, MacroCount(variant.macros, "ALPHA_TEST_MODE", "1"));
    EXPECT_EQ(1u, MacroCount(variant.macros, "MULTI_VIEW_MODE", "1"));
    ASSERT_EQ(1u, variant.passes.size());
    auto const& pass = variant.passes.front();
    EXPECT_EQ(CM_Back, pass.rasterizer.cull_mode);
    EXPECT_EQ("GBufferVS", pass.entry_points[std::to_underlying(ShaderStage::Vertex)]);
    EXPECT_EQ("GBufferPS", pass.entry_points[std::to_underlying(ShaderStage::Pixel)]);
    EXPECT_EQ("GBufferNoVpRtGS", pass.entry_points[std::to_underlying(ShaderStage::Geometry)]);
    EXPECT_TRUE(Named(desc.techniques, "GBufferTech").macros.empty());
}

TEST(BuiltInEffects, SubSurfaceDepthFallbackMacroRemainsLocalToItsPass)
{
    auto const desc = DescribeBuiltInEffect("SubSurface");
    EXPECT_EQ(0u, MacroCount(desc.macros, "NO_DEPTH_TEXTURE", "1"));
    for (auto const& technique : desc.techniques)
    {
        SCOPED_TRACE(technique.name);
        EXPECT_EQ(0u, MacroCount(technique.macros, "NO_DEPTH_TEXTURE", "1"));
        for (auto const& pass : technique.passes)
        {
            auto const expected = technique.name == "SubSurfaceTechWODepthTexture" ? 1u : 0u;
            EXPECT_EQ(expected, MacroCount(pass.macros, "NO_DEPTH_TEXTURE", "1"));
        }
    }
    auto const& fallback = Named(desc.techniques, "SubSurfaceTechWODepthTexture");
    ASSERT_EQ(1u, fallback.passes.size());
    EXPECT_EQ("SubSurfacePS", fallback.passes.front().entry_points[std::to_underlying(ShaderStage::Pixel)]);
}

TEST(BuiltInEffects, SkyBoxKeepsCubeTexturesAndTheFormerImplicitSampler)
{
    auto const desc = DescribeBuiltInEffect("SkyBox");
    EXPECT_EQ(EffectParameterType::TextureCube, Named(desc.parameters, "skybox_tex").type);
    EXPECT_EQ(EffectParameterType::TextureCube, Named(desc.parameters, "skybox_C_tex").type);
    auto const& sampler = Named(desc.parameters, "skybox_sampler");
    EXPECT_EQ(EffectParameterType::Sampler, sampler.type);
    EXPECT_EQ(TFO_Min_Mag_Linear_Mip_Point, sampler.sampler.filter);
    EXPECT_EQ(TAM_Clamp, sampler.sampler.addr_mode_u);
    EXPECT_EQ(TAM_Clamp, sampler.sampler.addr_mode_v);
    EXPECT_EQ(TAM_Clamp, sampler.sampler.addr_mode_w);
}
