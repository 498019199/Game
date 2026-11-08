#include "SDL3MetalShaderCompiler.h"
#include "SDL3RenderEngine.h"
#include <render/RenderEffect.h>
#include <render/RenderFactory.h>
#include <common/Hash.h>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include <spirv_msl.hpp>
#include <algorithm>
#include <cctype>
#include <mutex>
#include <stdexcept>

namespace RenderWorker
{
namespace
{
uint32_t SemanticLocation(std::string semantic)
{
    std::transform(semantic.begin(), semantic.end(), semantic.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    size_t const digit = semantic.find_first_of("0123456789");
    uint32_t const index = digit == std::string::npos ? 0 : static_cast<uint32_t>(std::stoul(semantic.substr(digit)));
    std::string const name = semantic.substr(0, digit);
    // Matches SDL3RenderLayout::LocationFromUsage, also shared by VS outputs / PS inputs.
    if (name == "POSITION") return 0;
    if (name == "NORMAL") return 1;
    if (name == "COLOR") return 2 + index;
    if (name == "BLENDWEIGHT") return 4;
    if (name == "BLENDINDICES") return 5;
    if (name == "TEXCOORD") return 6 + index;
    if (name == "TANGENT") return 14;
    if (name == "BINORMAL") return 15;
    throw std::runtime_error("Unsupported Metal shader semantic: " + semantic);
}
}

std::string CompileSDL3MetalShader(RenderEffect const& effect, RenderTechnique const& tech,
    RenderPass const& pass, ShaderStage stage, std::string& entry, SDL3ShaderDesc& desc)
{
    static bool const initialized = glslang::InitializeProcess();
    static std::mutex compiler_mutex;
    std::lock_guard<std::mutex> lock(compiler_mutex);
    if (!initialized) throw std::runtime_error("glslang initialization failed");

    auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    auto const& caps = re.DeviceCaps();
    std::string defines;
    auto define = [&](std::string const& name, auto value) {
        defines += "#define " + name + " " + std::to_string(value) + "\n";
    };
    define("KLAYGE_SDL3", 1);
    define("KLAYGE_D3D11", 1); // Select modern HLSL syntax in the shared shader library.
    define("KLAYGE_FRAG_DEPTH", 1);
    define("KLAYGE_SHADER_MODEL", caps.max_shader_model.FullVersion());
    define("KLAYGE_MAX_TEX_ARRAY_LEN", caps.max_texture_array_length);
    define("KLAYGE_MAX_TEX_DEPTH", caps.max_texture_depth);
    define("KLAYGE_MAX_TEX_UNITS", caps.max_pixel_texture_units);
    define("KLAYGE_FLIPPING", re.RequiresFlipping() ? -1 : 1);
    define("KLAYGE_RENDER_TO_TEX_ARRAY", caps.render_to_texture_array_support);
    define("KLAYGE_EXPLICIT_MULTI_SAMPLE_SUPPORT", caps.explicit_multi_sample_support);
    define(stage == ShaderStage::Vertex ? "KLAYGE_VERTEX_SHADER" : "KLAYGE_PIXEL_SHADER", 1);
    for (uint32_t i = 0; i < tech.NumMacros(); ++i) {
        auto const& nv = tech.MacroByIndex(i); defines += "#define " + nv.first + " " + nv.second + "\n";
    }
    for (uint32_t i = 0; i < pass.NumMacros(); ++i) {
        auto const& nv = pass.MacroByIndex(i); defines += "#define " + nv.first + " " + nv.second + "\n";
    }

    EShLanguage const language = stage == ShaderStage::Vertex ? EShLangVertex : EShLangFragment;
    glslang::TShader shader(language);
    char const* source = effect.HLSLShaderText().c_str();
    shader.setStrings(&source, 1);
    shader.setPreamble(defines.c_str());
    shader.setEntryPoint(entry.c_str());
    shader.setSourceEntryPoint(entry.c_str());
    shader.setEnvInput(glslang::EShSourceHlsl, language, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    shader.setEnvTargetHlslFunctionality1(); // Preserve HLSL semantics for vertex/varying locations.
    shader.setAutoMapBindings(true);
    shader.setAutoMapLocations(true);
    auto const messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgReadHlsl | EShMsgHlslOffsets);
    if (!shader.parse(GetDefaultResources(), 100, false, messages))
        throw std::runtime_error(shader.getInfoLog());
    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages) || !program.mapIO()) throw std::runtime_error(program.getInfoLog());
    std::vector<uint32_t> spirv;
    glslang::SpvOptions options;
    options.disableOptimizer = true;
    glslang::GlslangToSpv(*program.getIntermediate(language), spirv, &options);
    spirv_cross::CompilerMSL compiler(std::move(spirv));
    auto active = compiler.get_active_interface_variables();
    auto const resources = compiler.get_shader_resources(active);
    compiler.set_enabled_interface_variables(std::move(active));
    auto const execution = stage == ShaderStage::Vertex ? spv::ExecutionModelVertex : spv::ExecutionModelFragment;

    auto remap_interface = [&](auto const& variables) {
        for (auto const& var : variables) {
            if (compiler.has_decoration(var.id, spv::DecorationBuiltIn)) continue;
            auto const& semantic = compiler.get_decoration_string(var.id, spv::DecorationHlslSemanticGOOGLE);
            if (!semantic.empty()) compiler.set_decoration(var.id, spv::DecorationLocation, SemanticLocation(semantic));
        }
    };
    remap_interface(resources.stage_inputs);
    if (stage == ShaderStage::Vertex) remap_interface(resources.stage_outputs);

    // Separate descriptor sets prevent HLSL b0/t0/s0 aliases from colliding in the MSL remap.
    auto bind = [&](spirv_cross::Resource const& res, uint32_t set, uint32_t slot) {
        compiler.set_decoration(res.id, spv::DecorationDescriptorSet, set);
        compiler.set_decoration(res.id, spv::DecorationBinding, slot);
        spirv_cross::MSLResourceBinding binding{};
        binding.stage = execution;
        binding.desc_set = set;
        binding.binding = slot;
        binding.msl_buffer = slot;
        binding.msl_texture = slot;
        binding.msl_sampler = slot;
        compiler.add_msl_resource_binding(binding);
    };
    for (auto const& res : resources.uniform_buffers) {
        uint16_t const slot = static_cast<uint16_t>(desc.cb_desc.size());
        bind(res, 0, slot);
        auto const& type = compiler.get_type(res.base_type_id);
        auto& cb = desc.cb_desc.emplace_back();
        cb.name = res.name;
        cb.name_hash = RtHash(cb.name.c_str());
        cb.size = static_cast<uint32_t>((compiler.get_declared_struct_size(type) + 15) & ~size_t(15));
        cb.bind_point = slot;
        for (uint32_t i = 0; i < type.member_types.size(); ++i) {
            auto const& member = compiler.get_type(type.member_types[i]);
            auto& var = cb.var_desc.emplace_back();
            var.name = compiler.get_member_name(res.base_type_id, i);
            var.start_offset = compiler.type_struct_member_offset(type, i);
            var.rows = static_cast<uint8_t>(member.vecsize);
            var.columns = static_cast<uint8_t>(member.columns);
            var.elements = member.array.empty() ? 0 : static_cast<uint16_t>(member.array[0]);
        }
    }
    uint16_t texture_slot = 0;
    for (auto const& res : resources.separate_images) {
        bind(res, 1, texture_slot);
        desc.res_desc.push_back({res.name, 2, texture_slot++}); // D3D_SIT_TEXTURE
    }
    uint16_t sampler_slot = 0;
    for (auto const& res : resources.separate_samplers) {
        bind(res, 2, sampler_slot);
        desc.res_desc.push_back({res.name, 3, sampler_slot++}); // D3D_SIT_SAMPLER
    }
    if (!resources.storage_buffers.empty() || !resources.storage_images.empty())
        throw std::runtime_error("Metal graphics storage resources are not implemented");
    if (desc.cb_desc.size() > 4 || texture_slot > 16 || sampler_slot > 16)
        throw std::runtime_error("Shader exceeds SDL_GPU graphics resource limits");
    desc.num_uniform_buffers = static_cast<uint16_t>(desc.cb_desc.size());
    desc.num_srvs = texture_slot;
    desc.num_samplers = std::max(texture_slot, sampler_slot);
    spirv_cross::CompilerMSL::Options msl_options;
    msl_options.platform = spirv_cross::CompilerMSL::Options::macOS;
    msl_options.set_msl_version(2, 1);
    compiler.set_msl_options(msl_options);
    std::string msl = compiler.compile();
    entry = compiler.get_cleansed_entry_point_name(entry, execution);
    return msl;
}
}
