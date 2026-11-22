#include <base/ZEngine.h>
#include <render/BuiltInEffects.h>
#include <render/ParticleEffect.h>

#include <algorithm>
#include <bitset>
#include <stdexcept>

namespace RenderWorker
{
namespace
{
using Included = std::bitset<21>;

void SetMacro(std::vector<std::pair<std::string, std::string>>& macros,
    std::string_view name, std::string_view value)
{
    auto it = std::find_if(macros.begin(), macros.end(),
        [&](auto const& item) { return item.first == name; });
    if (it == macros.end()) macros.emplace_back(name, value);
    else it->second = value;
}

TechniqueDesc InheritTechnique(EffectDesc const& desc, std::string_view name)
{
    auto it = std::find_if(desc.techniques.begin(), desc.techniques.end(),
        [&](auto const& technique) { return technique.name == name; });
    if (it == desc.techniques.end()) throw std::logic_error("Unknown base technique: " + std::string(name));
    return *it;
}

void AppendCameraFrustum(EffectDesc& desc, Included& included);
void AppendCameraProxy(EffectDesc& desc, Included& included);
void AppendData2ShaderFx(EffectDesc& desc, Included& included);
void AppendDeferredRenderingUtil(EffectDesc& desc, Included& included);
void AppendEditorLitV1(EffectDesc& desc, Included& included);
void AppendGBuffer(EffectDesc& desc, Included& included);
void AppendLighting(EffectDesc& desc, Included& included);
void AppendLightSourceProxy(EffectDesc& desc, Included& included);
void AppendMaterial(EffectDesc& desc, Included& included);
void AppendMesh(EffectDesc& desc, Included& included);
void AppendMLShader(EffectDesc& desc, Included& included);
void AppendModelCamera(EffectDesc& desc, Included& included);
void AppendPostProcess(EffectDesc& desc, Included& included);
void AppendPredefinedCBuffers(EffectDesc& desc, Included& included);
void AppendQuaternion(EffectDesc& desc, Included& included);
void AppendRmlUi(EffectDesc& desc, Included& included);
void AppendSimpleAlbedoNormal(EffectDesc& desc, Included& included);
void AppendSkyBox(EffectDesc& desc, Included& included);
void AppendSubSurface(EffectDesc& desc, Included& included);
void AppendTerrain(EffectDesc& desc, Included& included);
void Appendutil(EffectDesc& desc, Included& included);

void AppendCameraFrustum(EffectDesc& desc, Included& included)
{
    if (included.test(0)) return;
    included.set(0);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    AppendModelCamera(desc, included);
    desc.shader_files.emplace_back("CameraFrustum.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "CameraFrustum";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "CameraFrustumVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "CameraFrustumPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendCameraProxy(EffectDesc& desc, Included& included)
{
    if (included.test(1)) return;
    included.set(1);
    AppendQuaternion(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    AppendModelCamera(desc, included);
    desc.shader_files.emplace_back("CameraProxy.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "CameraProxy";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "CameraProxyVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "CameraProxyPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendData2ShaderFx(EffectDesc& desc, Included& included)
{
    if (included.test(2)) return;
    included.set(2);
    Appendutil(desc, included);
    AppendQuaternion(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"worldviewproj", EffectParameterType::Float4x4};
        p.semantic = "WORLDVIEWPROJECTION";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"light_dir", EffectParameterType::Float3};
        p.initial_value = float3(0.35f, 0.85f, 0.45f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"eye_pos_os", EffectParameterType::Float3};
        p.initial_value = float3(0.f, 0.f, 0.f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_blend", EffectParameterType::Float};
        p.initial_value = 0.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"debug_albedo_only", EffectParameterType::Float};
        p.initial_value = 0.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"debug_mask1_ra", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail_scale", EffectParameterType::Float};
        p.initial_value = 4.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail_strength", EffectParameterType::Float};
        p.initial_value = 1.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail2_scale", EffectParameterType::Float};
        p.initial_value = 8.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail2_strength", EffectParameterType::Float};
        p.initial_value = 1.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"cubemap_strength", EffectParameterType::Float};
        p.initial_value = 0.35f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"selfillum_strength", EffectParameterType::Float};
        p.initial_value = 1.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail2_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail_mask_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"cubemap_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"selfillum_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"translucency_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"mask1_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"mask2_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail2_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"detail_mask_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"cubemap_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"translucency_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"mask1_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"mask2_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"linear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"cubemap_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("Data2ShaderFx.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "Data2ShaderFxTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "Data2ShaderFxVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "Data2ShaderFxPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendDeferredRenderingUtil(EffectDesc& desc, Included& included)
{
    if (included.test(3)) return;
    included.set(3);
    AppendLighting(desc, included);
    desc.shader_files.emplace_back("DeferredRenderingUtil.hlsli");
}

void AppendEditorLitV1(EffectDesc& desc, Included& included)
{
    if (included.test(4)) return;
    included.set(4);
    Appendutil(desc, included);
    AppendQuaternion(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"worldviewproj", EffectParameterType::Float4x4};
        p.semantic = "WORLDVIEWPROJECTION";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"eye_pos_os", EffectParameterType::Float3};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"linear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("EditorLitV1.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "EditorLitV1Tech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "EditorLitVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "EditorLitPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendGBuffer(EffectDesc& desc, Included& included)
{
    if (included.test(5)) return;
    included.set(5);
    AppendDeferredRenderingUtil(desc, included);
    AppendQuaternion(desc, included);
    Appendutil(desc, included);
    AppendLighting(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    AppendModelCamera(desc, included);
    {
        EffectParameterDesc p{"object_id", EffectParameterType::Float4};
        p.constant_buffer = "per_frame";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"frame_size", EffectParameterType::Int2};
        p.constant_buffer = "per_frame";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"half_exposure_x_framerate", EffectParameterType::Float};
        p.constant_buffer = "per_frame";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"motion_blur_radius", EffectParameterType::Float};
        p.constant_buffer = "per_frame";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"opaque_depth_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"point_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"bilinear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Linear_Mip_Point;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"trilinear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"aniso_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Anisotropic;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        p.sampler.max_anisotropy = 4;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("GBuffer.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "GBufferTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "GBufferVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GBufferPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferTech");
        technique.name = "GBufferAlphaTestTech";
        SetMacro(technique.macros, "ALPHA_TEST_MODE", "1");
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GBufferPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferTech");
        technique.name = "GBufferAlphaBlendBackTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.depth_stencil.depth_func = CF_Greater;
            pass.rasterizer.cull_mode = CM_Front;
            pass.depth_stencil.front_stencil_enable = true;
            pass.depth_stencil.front_stencil_func = CF_AlwaysPass;
            pass.depth_stencil.front_stencil_ref = 0;
            pass.depth_stencil.front_stencil_pass = SOP_Replace;
            pass.depth_stencil.back_stencil_enable = true;
            pass.depth_stencil.back_stencil_func = CF_AlwaysPass;
            pass.depth_stencil.back_stencil_ref = 0;
            pass.depth_stencil.back_stencil_pass = SOP_Replace;
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GBufferAlphaBlendPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaBlendBackTech");
        technique.name = "GBufferAlphaBlendFrontTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.depth_stencil.depth_func = CF_Less;
            pass.rasterizer.cull_mode = CM_Back;
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferTech");
        technique.name = "GBufferMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaTestTech");
        technique.name = "GBufferAlphaTestMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaBlendBackTech");
        technique.name = "GBufferAlphaBlendBackMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaBlendFrontTech");
        technique.name = "GBufferAlphaBlendFrontMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferMultiViewTech");
        technique.name = "GBufferMultiViewNoVpRtTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaTestMultiViewTech");
        technique.name = "GBufferAlphaTestMultiViewNoVpRtTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaBlendBackMultiViewTech");
        technique.name = "GBufferAlphaBlendBackMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferAlphaBlendFrontMultiViewTech");
        technique.name = "GBufferAlphaBlendFrontMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GBufferTech");
        technique.name = "GenReflectiveShadowMapTech";
        SetMacro(technique.macros, "SKIP_MOTION_VEC", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenReflectiveShadowMapTech");
        technique.name = "GenReflectiveShadowMapAlphaTestTech";
        SetMacro(technique.macros, "ALPHA_TEST_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenReflectiveShadowMapTech");
        technique.name = "GenReflectiveShadowMapMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenReflectiveShadowMapAlphaTestTech");
        technique.name = "GenReflectiveShadowMapAlphaTestMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenReflectiveShadowMapMultiViewTech");
        technique.name = "GenReflectiveShadowMapMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenReflectiveShadowMapAlphaTestMultiViewTech");
        technique.name = "GenReflectiveShadowMapAlphaTestMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "GenShadowMapTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.blend.color_write_mask[0] = 0;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "GenShadowMapVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GenShadowMapPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenShadowMapTech");
        technique.name = "GenShadowMapAlphaTestTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GenShadowMapAlphaTestPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenShadowMapTech");
        technique.name = "GenShadowMapMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenShadowMapAlphaTestTech");
        technique.name = "GenShadowMapAlphaTestMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenShadowMapMultiViewTech");
        technique.name = "GenShadowMapMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GenShadowCubemapNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenShadowMapAlphaTestMultiViewTech");
        technique.name = "GenShadowMapAlphaTestMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GenShadowCubemapNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "GenCascadedShadowMapTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.rasterizer.depth_clip_enable = false;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "GenShadowMapVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GenShadowMapPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenCascadedShadowMapTech");
        technique.name = "GenCascadedShadowMapAlphaTestTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GenShadowMapAlphaTestPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenCascadedShadowMapTech");
        technique.name = "GenCascadedShadowMapMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenCascadedShadowMapAlphaTestTech");
        technique.name = "GenCascadedShadowMapAlphaTestMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenCascadedShadowMapMultiViewTech");
        technique.name = "GenCascadedShadowMapMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GenShadowCubemapNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "GenCascadedShadowMapAlphaTestMultiViewTech");
        technique.name = "GenCascadedShadowMapAlphaTestMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GenShadowCubemapNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "SpecialShadingTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.depth_stencil.depth_enable = true;
            pass.depth_stencil.depth_func = CF_Equal;
            pass.depth_stencil.depth_write_mask = false;
            pass.blend.blend_enable[0] = true;
            pass.blend.blend_op[0] = BOP_Add;
            pass.blend.src_blend[0] = ABF_One;
            pass.blend.dest_blend[0] = ABF_One;
            pass.blend.blend_op_alpha[0] = BOP_Add;
            pass.blend.src_blend_alpha[0] = ABF_Src_Alpha;
            pass.blend.dest_blend_alpha[0] = ABF_Zero;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "GBufferVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SpecialShadingPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingTech");
        technique.name = "SpecialShadingAlphaBlendBackTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Front;
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SpecialShadingAlphaBlendPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingAlphaBlendBackTech");
        technique.name = "SpecialShadingAlphaBlendFrontTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingTech");
        technique.name = "SpecialShadingMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingAlphaBlendBackTech");
        technique.name = "SpecialShadingAlphaBlendBackMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingAlphaBlendFrontTech");
        technique.name = "SpecialShadingAlphaBlendFrontMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingMultiViewTech");
        technique.name = "SpecialShadingMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingAlphaBlendBackMultiViewTech");
        technique.name = "SpecialShadingAlphaBlendBackMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SpecialShadingAlphaBlendFrontMultiViewTech");
        technique.name = "SpecialShadingAlphaBlendFrontMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "SelectModeTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "GBufferVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SelectModePS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SelectModeTech");
        technique.name = "SelectModeMultiViewTech";
        SetMacro(technique.macros, "MULTI_VIEW_MODE", "1");
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique = InheritTechnique(desc, "SelectModeMultiViewTech");
        technique.name = "SelectModeMultiViewNoVpRtTech";
        {
            auto it = std::find_if(technique.passes.begin(), technique.passes.end(),
                [](auto const& p) { return p.name == "p0"; });
            if (it == technique.passes.end())
            {
                technique.passes.emplace_back();
                it = std::prev(technique.passes.end());
            }
            auto& pass = *it;
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Geometry)] = "GBufferNoVpRtGS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendLighting(EffectDesc& desc, Included& included)
{
    if (included.test(6)) return;
    included.set(6);
    SetMacro(desc.macros, "MAX_SHININESS", "8192.0f");
    desc.shader_files.emplace_back("Lighting.hlsli");
}

void AppendLightSourceProxy(EffectDesc& desc, Included& included)
{
    if (included.test(7)) return;
    included.set(7);
    AppendLighting(desc, included);
    AppendGBuffer(desc, included);
    AppendModelCamera(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"light_is_projective", EffectParameterType::Int2};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"projective_map_2d_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"projective_map_cube_tex", EffectParameterType::TextureCube};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"skybox_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Linear_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        p.sampler.addr_mode_w = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"linear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Linear_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("LightSourceProxy.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "LightSourceProxy";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "LightSourceProxyVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "LightSourceProxyPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendMaterial(EffectDesc& desc, Included& included)
{
    if (included.test(8)) return;
    included.set(8);
    {
        EffectParameterDesc p{"albedo_clr", EffectParameterType::Float4};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"metalness_glossiness_factor", EffectParameterType::Float3};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"emissive_clr", EffectParameterType::Float4};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"albedo_map_enabled", EffectParameterType::Int};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_map_enabled", EffectParameterType::Int};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"height_map_parallax_enabled", EffectParameterType::Int};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"height_map_tess_enabled", EffectParameterType::Int};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"occlusion_map_enabled", EffectParameterType::Int};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"alpha_test_threshold", EffectParameterType::Float};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_scale", EffectParameterType::Float};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"occlusion_strength", EffectParameterType::Float};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"height_offset_scale", EffectParameterType::Float2};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"tess_factors", EffectParameterType::Float4};
        p.constant_buffer = "klayge_material";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"albedo_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"metalness_glossiness_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"emissive_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"height_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"occlusion_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
}

void AppendMesh(EffectDesc& desc, Included& included)
{
    if (included.test(9)) return;
    included.set(9);
    {
        EffectParameterDesc p{"pos_center", EffectParameterType::Float3};
        p.constant_buffer = "klayge_mesh";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"pos_extent", EffectParameterType::Float3};
        p.constant_buffer = "klayge_mesh";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"tc_center", EffectParameterType::Float2};
        p.constant_buffer = "klayge_mesh";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"tc_extent", EffectParameterType::Float2};
        p.constant_buffer = "klayge_mesh";
        desc.parameters.push_back(std::move(p));
    }
}

void AppendMLShader(EffectDesc& desc, Included& included)
{
    if (included.test(10)) return;
    included.set(10);
    Appendutil(desc, included);
    AppendQuaternion(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"worldviewproj", EffectParameterType::Float4x4};
        p.semantic = "WORLDVIEWPROJECTION";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"light_dir", EffectParameterType::Float3};
        p.initial_value = float3(0.35f, 0.85f, 0.45f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"emissive_color", EffectParameterType::Float3};
        p.initial_value = float3(0.f, 0.25f, 0.85f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"selfillum_strength", EffectParameterType::Float};
        p.initial_value = 1.5f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"specular_color", EffectParameterType::Float3};
        p.initial_value = float3(1.f, 1.f, 1.f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"specular_power", EffectParameterType::Float};
        p.initial_value = 30.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"specular_strength", EffectParameterType::Float};
        p.initial_value = 1.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"bloom_radius", EffectParameterType::Float};
        p.initial_value = 0.012f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"bloom_strength", EffectParameterType::Float};
        p.initial_value = 1.25f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"bloom_threshold", EffectParameterType::Float};
        p.initial_value = 0.05f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"mask1_map_enabled", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"mask1_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"linear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("MLShader.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "MLTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "SimpleAlbedoNormalVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SimpleAlbedoNormalPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendModelCamera(EffectDesc& desc, Included& included)
{
    if (included.test(11)) return;
    included.set(11);
    {
        EffectStructDesc structure;
        structure.name = "KlayGECameraInfo";
        structure.members.push_back({"model_view", EffectParameterType::Float4x4, "", 0});
        structure.members.push_back({"mvp", EffectParameterType::Float4x4, "", 0});
        structure.members.push_back({"inv_mv", EffectParameterType::Float4x4, "", 0});
        structure.members.push_back({"inv_mvp", EffectParameterType::Float4x4, "", 0});
        structure.members.push_back({"eye_pos", EffectParameterType::Float3, "", 0});
        structure.members.push_back({"forward_vec", EffectParameterType::Float3, "", 0});
        structure.members.push_back({"up_vec", EffectParameterType::Float3, "", 0});
        desc.structs.push_back(std::move(structure));
    }
    {
        EffectParameterDesc p{"model", EffectParameterType::Float4x4};
        p.constant_buffer = "klayge_model";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"inv_model", EffectParameterType::Float4x4};
        p.constant_buffer = "klayge_model";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"num_cameras", EffectParameterType::UInt};
        p.constant_buffer = "klayge_camera";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"camera_indices", EffectParameterType::UInt4};
        p.constant_buffer = "klayge_camera";
        p.array_size = 2;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"cameras", EffectParameterType::Struct};
        p.constant_buffer = "klayge_camera";
        p.struct_type = "KlayGECameraInfo";
        p.array_size = 8;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"prev_mvps", EffectParameterType::Float4x4};
        p.constant_buffer = "klayge_camera";
        p.array_size = 8;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("ModelCamera.hlsli");
}

void AppendPostProcess(EffectDesc& desc, Included& included)
{
    if (included.test(12)) return;
    included.set(12);
    Appendutil(desc, included);
    {
        EffectParameterDesc p{"pp_mvp", EffectParameterType::Float4x4};
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("PostProcess.hlsli");
}

void AppendPredefinedCBuffers(EffectDesc& desc, Included& included)
{
    if (included.test(13)) return;
    included.set(13);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    AppendModelCamera(desc, included);
    desc.shader_files.emplace_back("PredefinedCBuffers.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "PredefinedCBuffersNoopTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "PredefinedCBuffersNoopVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "PredefinedCBuffersNoopPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendQuaternion(EffectDesc& desc, Included& included)
{
    if (included.test(14)) return;
    included.set(14);
    desc.shader_files.emplace_back("Quaternion.hlsli");
}

void AppendRmlUi(EffectDesc& desc, Included& included)
{
    if (included.test(15)) return;
    included.set(15);
    AppendModelCamera(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"Transform", EffectParameterType::Float4x4};
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"rml_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"rml_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("RmlUi.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "RmlUiTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.depth_stencil.depth_write_mask = false;
            pass.depth_stencil.depth_func = CF_AlwaysPass;
            pass.blend.blend_enable[0] = true;
            pass.blend.src_blend[0] = ABF_One;
            pass.blend.dest_blend[0] = ABF_Inv_Src_Alpha;
            pass.blend.src_blend_alpha[0] = ABF_One;
            pass.blend.dest_blend_alpha[0] = ABF_Inv_Src_Alpha;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "RmlUiVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "RmlUiPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "RmlUiScissorTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.depth_stencil.depth_write_mask = false;
            pass.depth_stencil.depth_func = CF_AlwaysPass;
            pass.blend.blend_enable[0] = true;
            pass.blend.src_blend[0] = ABF_One;
            pass.blend.dest_blend[0] = ABF_Inv_Src_Alpha;
            pass.blend.src_blend_alpha[0] = ABF_One;
            pass.blend.dest_blend_alpha[0] = ABF_Inv_Src_Alpha;
            pass.rasterizer.scissor_enable = true;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "RmlUiScissorVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "RmlUiScissorPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendSimpleAlbedoNormal(EffectDesc& desc, Included& included)
{
    if (included.test(16)) return;
    included.set(16);
    Appendutil(desc, included);
    AppendQuaternion(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"worldviewproj", EffectParameterType::Float4x4};
        p.semantic = "WORLDVIEWPROJECTION";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"light_dir", EffectParameterType::Float3};
        p.initial_value = float3(0.35f, 0.85f, 0.45f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_blend", EffectParameterType::Float};
        p.initial_value = 0.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"debug_albedo_only", EffectParameterType::Float};
        p.initial_value = 0.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"linear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("SimpleAlbedoNormal.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "SimpleAlbedoNormalTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "SimpleAlbedoNormalVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SimpleAlbedoNormalPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendSkyBox(EffectDesc& desc, Included& included)
{
    if (included.test(17)) return;
    included.set(17);
    Appendutil(desc, included);
    {
        EffectParameterDesc p{"inv_mvp", EffectParameterType::Float4x4};
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"depth_far", EffectParameterType::Float};
        p.initial_value = 0.f;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"skybox_compressed", EffectParameterType::Int};
        p.initial_value = int32_t(0);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"skybox_tex", EffectParameterType::TextureCube};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"skybox_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Linear_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        p.sampler.addr_mode_w = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"skybox_C_tex", EffectParameterType::TextureCube};
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("SkyBox.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "GBufferSkyBoxTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.depth_stencil.depth_write_mask = false;
            pass.depth_stencil.depth_func = CF_Equal;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "GBufferSkyBoxVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "GBufferSkyBoxPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "SkyBoxTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_None;
            pass.depth_stencil.depth_write_mask = false;
            pass.depth_stencil.depth_func = CF_LessEqual;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "SkyBoxVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SkyBoxPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendSubSurface(EffectDesc& desc, Included& included)
{
    if (included.test(18)) return;
    included.set(18);
    AppendLighting(desc, included);
    Appendutil(desc, included);
    AppendQuaternion(desc, included);
    AppendMaterial(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"worldviewproj", EffectParameterType::Float4x4};
        p.semantic = "WORLDVIEWPROJECTION";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"eye_pos", EffectParameterType::Float3};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"light_pos", EffectParameterType::Float3};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"light_color", EffectParameterType::Float3};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"light_falloff", EffectParameterType::Float3};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"extinction_coefficient", EffectParameterType::Float3};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"material_thickness", EffectParameterType::Float};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"sigma_t", EffectParameterType::Float};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"far_plane", EffectParameterType::Float2};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"near_q", EffectParameterType::Float2};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"back_face_depth_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"linear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"point_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("SubSurface.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "BackFaceDepthTechWODepthTexture";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Front;
            pass.depth_stencil.depth_func = CF_Greater;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "BackFaceDepthVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "BackFaceDepthPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "BackFaceDepthTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Front;
            pass.depth_stencil.depth_func = CF_Greater;
            pass.blend.color_write_mask[0] = 0;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "BackFaceDepthVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "BackFaceDepthPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "SubSurfaceTech";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "SubSurfaceVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SubSurfacePS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "SubSurfaceTechWODepthTexture";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.rasterizer.cull_mode = CM_Back;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "SubSurfaceVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "SubSurfacePS";
            SetMacro(pass.macros, "NO_DEPTH_TEXTURE", "1");
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void AppendTerrain(EffectDesc& desc, Included& included)
{
    if (included.test(19)) return;
    included.set(19);
    Appendutil(desc, included);
    AppendPostProcess(desc, included);
    AppendMesh(desc, included);
    {
        EffectParameterDesc p{"mvp", EffectParameterType::Float4x4};
        p.constant_buffer = "per_frame";
        p.initial_value = float4x4::Zero();
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"inv_far", EffectParameterType::Float};
        p.constant_buffer = "per_frame";
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"LightDir", EffectParameterType::Float3};
        p.initial_value = float3(0.f, 10.f, 3.f);
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"grass_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"height_map_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_map_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"src_tex", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"tex_with_alpha", EffectParameterType::Texture2D};
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"point_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"trilinear_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Mip_Linear;
        p.sampler.addr_mode_u = TAM_Wrap;
        p.sampler.addr_mode_v = TAM_Wrap;
        p.sampler.addr_mode_w = TAM_Wrap;
        desc.parameters.push_back(std::move(p));
    }
    {
        EffectParameterDesc p{"normal_map_sampler", EffectParameterType::Sampler};
        p.sampler.filter = TFO_Min_Mag_Linear_Mip_Point;
        p.sampler.addr_mode_u = TAM_Clamp;
        p.sampler.addr_mode_v = TAM_Clamp;
        desc.parameters.push_back(std::move(p));
    }
    desc.shader_files.emplace_back("Terrain.hlsl");
    {
        TechniqueDesc technique;
        technique.name = "Terrain";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "TerrainVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "TerrainPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
    {
        TechniqueDesc technique;
        technique.name = "Blend";
        {
            auto& pass = technique.passes.emplace_back();
            pass.name = "p0";
            pass.depth_stencil.depth_write_mask = false;
            pass.entry_points[static_cast<size_t>(ShaderStage::Vertex)] = "PostProcessVS";
            pass.entry_points[static_cast<size_t>(ShaderStage::Pixel)] = "BlendPS";
        }
        desc.techniques.push_back(std::move(technique));
    }
}

void Appendutil(EffectDesc& desc, Included& included)
{
    if (included.test(20)) return;
    included.set(20);
    desc.shader_files.emplace_back("util.hlsli");
}

} // namespace

EffectDesc CameraFrustumEffect::Describe()
{
    EffectDesc desc;
    desc.name = "CameraFrustum";
    Included included;
    AppendCameraFrustum(desc, included);
    return desc;
}

EffectDesc CameraProxyEffect::Describe()
{
    EffectDesc desc;
    desc.name = "CameraProxy";
    Included included;
    AppendCameraProxy(desc, included);
    return desc;
}

EffectDesc Data2ShaderFxEffect::Describe()
{
    EffectDesc desc;
    desc.name = "Data2ShaderFx";
    Included included;
    AppendData2ShaderFx(desc, included);
    return desc;
}

EffectDesc EditorLitV1Effect::Describe()
{
    EffectDesc desc;
    desc.name = "EditorLitV1";
    Included included;
    AppendEditorLitV1(desc, included);
    return desc;
}

EffectDesc GBufferEffect::Describe()
{
    EffectDesc desc;
    desc.name = "GBuffer";
    Included included;
    AppendGBuffer(desc, included);
    return desc;
}

EffectDesc LightSourceProxyEffect::Describe()
{
    EffectDesc desc;
    desc.name = "LightSourceProxy";
    Included included;
    AppendLightSourceProxy(desc, included);
    return desc;
}

EffectDesc MLShaderEffect::Describe()
{
    EffectDesc desc;
    desc.name = "MLShader";
    Included included;
    AppendMLShader(desc, included);
    return desc;
}

EffectDesc PredefinedCBuffersEffect::Describe()
{
    EffectDesc desc;
    desc.name = "PredefinedCBuffers";
    Included included;
    AppendPredefinedCBuffers(desc, included);
    return desc;
}

EffectDesc RmlUiEffect::Describe()
{
    EffectDesc desc;
    desc.name = "RmlUi";
    Included included;
    AppendRmlUi(desc, included);
    return desc;
}

EffectDesc SimpleAlbedoNormalEffect::Describe()
{
    EffectDesc desc;
    desc.name = "SimpleAlbedoNormal";
    Included included;
    AppendSimpleAlbedoNormal(desc, included);
    return desc;
}

EffectDesc SkyBoxEffect::Describe()
{
    EffectDesc desc;
    desc.name = "SkyBox";
    Included included;
    AppendSkyBox(desc, included);
    return desc;
}

EffectDesc SubSurfaceEffect::Describe()
{
    EffectDesc desc;
    desc.name = "SubSurface";
    Included included;
    AppendSubSurface(desc, included);
    return desc;
}

EffectDesc TerrainEffect::Describe()
{
    EffectDesc desc;
    desc.name = "Terrain";
    Included included;
    AppendTerrain(desc, included);
    return desc;
}

std::span<std::string_view const> BuiltInEffectNames()
{
    static constexpr std::string_view names[] = {
        "CameraFrustum",
        "CameraProxy",
        "Data2ShaderFx",
        "EditorLitV1",
        "GBuffer",
        "LightSourceProxy",
        "MLShader",
        "Particle",
        "PredefinedCBuffers",
        "RmlUi",
        "SimpleAlbedoNormal",
        "SkyBox",
        "SubSurface",
        "Terrain",
    };
    return names;
}

EffectDesc DescribeBuiltInEffect(std::string_view name)
{
    if (name == "CameraFrustum") return CameraFrustumEffect::Describe();
    if (name == "CameraProxy") return CameraProxyEffect::Describe();
    if (name == "Data2ShaderFx") return Data2ShaderFxEffect::Describe();
    if (name == "EditorLitV1") return EditorLitV1Effect::Describe();
    if (name == "GBuffer") return GBufferEffect::Describe();
    if (name == "LightSourceProxy") return LightSourceProxyEffect::Describe();
    if (name == "MLShader") return MLShaderEffect::Describe();
    if (name == "Particle") return ParticleEffect::Describe();
    if (name == "PredefinedCBuffers") return PredefinedCBuffersEffect::Describe();
    if (name == "RmlUi") return RmlUiEffect::Describe();
    if (name == "SimpleAlbedoNormal") return SimpleAlbedoNormalEffect::Describe();
    if (name == "SkyBox") return SkyBoxEffect::Describe();
    if (name == "SubSurface") return SubSurfaceEffect::Describe();
    if (name == "Terrain") return TerrainEffect::Describe();
    throw std::invalid_argument("Unknown built-in effect: " + std::string(name));
}
} // namespace RenderWorker
