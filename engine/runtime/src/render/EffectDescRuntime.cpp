#include <algorithm>
#include <base/ZEngine.h>
#include <base/ResLoader.h>
#include <render/EffectDesc.h>
#include <render/RenderEffect.h>
#include <render/RenderFactory.h>
#include "RenderEffectInternal.h"

#include <filesystem>
#include <iterator>
#include <stdexcept>

namespace RenderWorker
{
namespace detail
{
#if ZENGINE_IS_DEV_PLATFORM
RenderEffectDataType EffectType(EffectParameterType type)
{
    static constexpr RenderEffectDataType types[] = {
        REDT_bool, REDT_uint, REDT_int, REDT_float, REDT_float2, REDT_float3, REDT_float4,
        REDT_float4x4, REDT_texture2D, REDT_textureCUBE, REDT_sampler,
        REDT_int2, REDT_int3, REDT_int4, REDT_uint2, REDT_uint3, REDT_uint4, REDT_struct
    };
    auto const index = std::to_underlying(type);
    if (index < 0 || static_cast<size_t>(index) >= std::size(types))
        throw std::invalid_argument("Unsupported EffectDesc parameter type");
    return types[index];
}
#endif
uint64_t EffectSourceFingerprint(std::string_view source)
{
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char value : source) { hash ^= value; hash *= 1099511628211ULL; }
    return hash;
}
std::string ReadEffectShaderSource(EffectDesc const& desc)
{
    desc.Validate();
    std::string text;
#if ZENGINE_IS_DEV_PLATFORM
    auto& loader = Context::Instance().ResLoaderInstance();
    for (auto const& file : desc.shader_files)
    {
        auto source = loader.Open(file);
        if (!source) throw std::runtime_error("EffectDesc " + desc.name + ": missing HLSL file " + file);
        auto label = std::filesystem::path(file).generic_string();
        text += "\n#line 1 \"" + label + "\"\n";
        text.append(std::istreambuf_iterator<char>(source->input_stream()), std::istreambuf_iterator<char>());
        text += '\n';
    }
#endif
    return text;
}
}

void RenderEffect::Load(EffectDesc const& desc)
{
    LoadDescription(desc, detail::ReadEffectShaderSource(desc));
}

void RenderEffect::LoadDescription(EffectDesc const& desc, std::string const& source_text)
{
    desc.Validate();
    auto& loader = Context::Instance().ResLoaderInstance();
    auto const platform = Context::Instance().RenderFactoryInstance().RenderEngineInstance().NativeShaderPlatformName();
    auto const cache_name = desc.CacheName(platform);
    auto const located_source = loader.Locate(desc.shader_files.front());
    auto const source_path = std::filesystem::path(located_source.empty() ? desc.shader_files.front() : located_source);
    auto const cache_path = (source_path.parent_path() / cache_name).string();
    auto reset = [&]
    {
        immutable_ = MakeSharedPtr<Immutable>();
        immutable_->res_name = desc.name + ".effect";
        immutable_->res_name_hash = HashValue(immutable_->res_name);
        params_.clear(); cbuffers_.clear(); shader_objs_.clear();
        hw_res_ready_ = false;
#if ZENGINE_IS_DEV_PLATFORM
        immutable_->timestamp = 0;
        immutable_->need_compile = false;
        immutable_->description_based = true;
        immutable_->description_hash = desc.Fingerprint();
        immutable_->source_hash = detail::EffectSourceFingerprint(source_text);
        immutable_->kfx_name = cache_path;
#endif
    };
    reset();
    auto cache = loader.Open(cache_path);
    if (!cache && cache_path != cache_name) cache = loader.Open(cache_name);
    if (cache)
    {
        uint32_t magic = 0;
        uint64_t description_hash = 0, source_hash = 0;
        cache->read(&magic, sizeof(magic));
        cache->read(&description_hash, sizeof(description_hash));
        cache->read(&source_hash, sizeof(source_hash));
        bool matches = LE2Native(magic) == detail::EffectDescCacheMagic && LE2Native(description_hash) == desc.Fingerprint();
#if ZENGINE_IS_DEV_PLATFORM
        matches = matches && LE2Native(source_hash) == immutable_->source_hash;
#endif
        if (matches && StreamIn(*cache)) return;
        reset(); // StreamIn may have populated a partial effect before rejecting it.
    }
#if ZENGINE_IS_DEV_PLATFORM
    immutable_->macros = desc.macros;
    immutable_->shader_descs.resize(1);
    immutable_->struct_types.reserve(desc.structs.size());
    for (auto const& structure : desc.structs)
        immutable_->struct_types.emplace_back().Load(structure);
    params_.reserve(desc.parameters.size());
    for (auto const& parameter : desc.parameters)
    {
        auto const index = static_cast<uint32_t>(params_.size());
        params_.emplace_back().Load(*this, parameter);
        if (parameter.type != EffectParameterType::Texture2D && parameter.type != EffectParameterType::TextureCube &&
            parameter.type != EffectParameterType::Sampler)
        {
            auto* buffer = CBufferByName(parameter.constant_buffer);
            if (!buffer)
            {
                buffer = cbuffers_.emplace_back(MakeSharedPtr<RenderEffectConstantBuffer>(*this)).get();
                buffer->Load(parameter.constant_buffer);
            }
            buffer->AddParameter(index);
        }
    }
    immutable_->shader_frags.emplace_back().LoadSource(source_text);
    GenHLSLShaderText();
    immutable_->techniques.reserve(desc.techniques.size());
    for (auto const& technique : desc.techniques)
    {
        auto const index = static_cast<uint32_t>(immutable_->techniques.size());
        immutable_->techniques.emplace_back().Load(*this, technique, index);
    }
    immutable_->need_compile = true;
#else
    throw std::runtime_error("Missing or incompatible baked effect: " + cache_name);
#endif
}

#if ZENGINE_IS_DEV_PLATFORM
void RenderEffectStructType::Load(EffectStructDesc const& desc)
{
    name_ = desc.name;
    name_hash_ = HashValue(name_);
    members_.clear();
    members_.reserve(desc.members.size());
    for (auto const& member : desc.members)
    {
        members_.push_back(StrcutMemberType{detail::EffectType(member.type), member.struct_type, member.name,
            MakeUniquePtr<std::string>(member.array_size ? std::to_string(member.array_size) : std::string())});
    }
}

void RenderEffectParameter::Load(RenderEffect const& effect, EffectParameterDesc const& desc)
{
    immutable_ = MakeSharedPtr<Immutable>();
    immutable_->type = detail::EffectType(desc.type);
    immutable_->name = desc.name;
    immutable_->name_hash = HashValue(desc.name);
    immutable_->semantic = desc.semantic;
    immutable_->semantic_hash = HashValue(desc.semantic);
    if (desc.array_size != 0)
        immutable_->array_size = MakeUniquePtr<std::string>(std::to_string(desc.array_size));
    var_ = detail::CreateVariable(effect, desc);
    if (desc.type == EffectParameterType::Sampler)
        *var_ = Context::Instance().RenderFactoryInstance().MakeSamplerStateObject(desc.sampler);
}

void RenderShaderFragment::LoadSource(std::string const& source)
{
    stage_ = ShaderStage::NumStages;
    ver_ = ShaderModel(0, 0);
    str_ = source;
}

void RenderTechnique::Load(RenderEffect& effect, TechniqueDesc const& desc, uint32_t tech_index)
{
    name_ = desc.name;
    name_hash_ = HashValue(name_);
    ver_ = desc.minimum_shader_model;
    is_validate_ = false;
    has_discard_ = has_tessellation_ = transparent_ = false;
    weight_ = 1;
    passes_.clear();
    macros_ = desc.macros.empty() ? nullptr :
        MakeSharedPtr<std::vector<std::pair<std::string, std::string>>>(desc.macros);
    auto const& caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
    if (ver_ > caps.max_shader_model) return;
    for (auto const& description : desc.passes)
    {
        auto has = [&](ShaderStage stage) { return !description.entry_points[std::to_underlying(stage)].empty(); };
        if ((has(ShaderStage::Geometry) && !caps.gs_support) || (has(ShaderStage::Hull) && !caps.hs_support) ||
            (has(ShaderStage::Domain) && !caps.ds_support) || (has(ShaderStage::Compute) && !caps.cs_support))
            return; // Keep the technique discoverable, but without unsupported passes.
    }
    for (auto const& description : desc.passes)
    {
        auto const index = static_cast<uint32_t>(passes_.size());
        auto& pass = *passes_.emplace_back(MakeSharedPtr<RenderPass>());
        pass.Load(effect, description, tech_index, index);
        transparent_ |= std::any_of(description.blend.blend_enable.begin(), description.blend.blend_enable.end(),
            [](bool enabled) { return enabled; });
    }
    if (transparent_) weight_ += 10000;
}

void RenderPass::Load(RenderEffect& effect, PassDesc const& desc, uint32_t tech_index, uint32_t pass_index)
{
    name_ = desc.name;
    name_hash_ = HashValue(name_);
    is_validate_ = false;
    macros_ = desc.macros.empty() ? nullptr :
        MakeSharedPtr<std::vector<std::pair<std::string, std::string>>>(desc.macros);
    std::string macro_identity;
    auto append_macro = [&](std::pair<std::string, std::string> const& macro)
    {
        macro_identity += std::to_string(macro.first.size()) + ":" + macro.first;
        macro_identity += std::to_string(macro.second.size()) + ":" + macro.second;
    };
    auto const& technique = *effect.TechniqueByIndex(tech_index);
    macro_identity += "technique:" + std::to_string(technique.NumMacros()) + ":";
    for (uint32_t i = 0; i < technique.NumMacros(); ++i) append_macro(technique.MacroByIndex(i));
    macro_identity += "pass:" + std::to_string(desc.macros.size()) + ":";
    for (auto const& macro : desc.macros) append_macro(macro);
    auto& factory = Context::Instance().RenderFactoryInstance();
    render_state_obj_ = factory.MakeRenderStateObject(desc.rasterizer, desc.depth_stencil, desc.blend);
    shader_obj_index_ = effect.AddShaderObject();
    shader_desc_ids_.fill(0);
    for (uint32_t index = 0; index < ShaderStageNum; ++index)
    {
        if (desc.entry_points[index].empty()) continue;
        ShaderDesc shader;
        shader.profile = "auto";
        shader.func_name = desc.entry_points[index];
        // Include the stage in identity: an entry point can be used in more than one stage.
        shader.macros_hash = detail::EffectSourceFingerprint(macro_identity + "stage:" + std::to_string(index));
        auto const id = shader_desc_ids_[index] = effect.AddShaderDesc(shader);
        auto& stored = effect.GetShaderDesc(id);
        auto const stage = static_cast<ShaderStage>(index);
        ShaderStageObjectPtr stage_object;
        if (stored.tech_pass_type == 0xFFFFFFFF)
        {
            stored.tech_pass_type = (tech_index << 16) | (pass_index << 8) | index;
            stage_object = factory.MakeShaderStageObject(stage);
        }
        else
        {
            auto const& existing = effect.TechniqueByIndex(stored.tech_pass_type >> 16)->Pass((stored.tech_pass_type >> 8) & 0xFF);
            stage_object = existing.GetShaderObject(effect)->Stage(stage);
        }
        GetShaderObject(effect)->AttachStage(stage, stage_object);
    }
}
#endif
}
