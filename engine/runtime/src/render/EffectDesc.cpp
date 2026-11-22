#include <algorithm>
#include <render/EffectDesc.h>

#include <cctype>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

namespace RenderWorker
{
namespace
{
bool Identifier(std::string const& value)
{
    auto first = [](unsigned char c) { return std::isalpha(c) || c == '_'; };
    return !value.empty() && first(value.front()) &&
        std::all_of(value.begin() + 1, value.end(), [&](unsigned char c) { return first(c) || std::isdigit(c); });
}
void Require(bool condition, std::string const& message)
{
    if (!condition) throw std::invalid_argument("EffectDesc: " + message);
}
bool Numeric(EffectParameterType type)
{
    return type != EffectParameterType::Texture2D && type != EffectParameterType::TextureCube &&
        type != EffectParameterType::Sampler;
}
bool InitialValueMatches(EffectParameterDesc const& parameter)
{
    auto const& value = parameter.initial_value;
    if (std::holds_alternative<std::monostate>(value)) return true;
    if (parameter.array_size != 0) return false;
    switch (parameter.type)
    {
    case EffectParameterType::Bool: return std::holds_alternative<bool>(value);
    case EffectParameterType::UInt: return std::holds_alternative<uint32_t>(value);
    case EffectParameterType::Int: return std::holds_alternative<int32_t>(value);
    case EffectParameterType::Float: return std::holds_alternative<float>(value);
    case EffectParameterType::Float2: return std::holds_alternative<float2>(value);
    case EffectParameterType::Float3: return std::holds_alternative<float3>(value);
    case EffectParameterType::Float4: return std::holds_alternative<float4>(value);
    case EffectParameterType::Float4x4: return std::holds_alternative<float4x4>(value);
    case EffectParameterType::Int2: return std::holds_alternative<int2>(value);
    case EffectParameterType::Int3: return std::holds_alternative<int3>(value);
    case EffectParameterType::Int4: return std::holds_alternative<int4>(value);
    case EffectParameterType::UInt2: return std::holds_alternative<uint2>(value);
    case EffectParameterType::UInt3: return std::holds_alternative<uint3>(value);
    case EffectParameterType::UInt4: return std::holds_alternative<uint4>(value);
    default: return false;
    }
}
void ValidateMacros(std::vector<std::pair<std::string, std::string>> const& macros, size_t limit)
{
    Require(macros.size() <= limit, "too many macros");
    std::unordered_set<std::string> names;
    for (auto const& [key, value] : macros)
    {
        Require(Identifier(key) && key.size() < 256 && value.size() < 256 && value.find_first_of("\r\n") == std::string::npos,
            "invalid macro: " + key);
        Require(names.insert(key).second, "duplicate macro: " + key);
    }
}
class FingerprintBuilder
{
public:
    void Bytes(void const* data, size_t size)
    {
        auto bytes = static_cast<unsigned char const*>(data);
        for (size_t i = 0; i < size; ++i) { hash_ ^= bytes[i]; hash_ *= 1099511628211ULL; }
    }
    template <typename T> void Pod(T const& value) { Bytes(&value, sizeof(value)); }
    void Text(std::string const& value)
    {
        Pod(static_cast<uint64_t>(value.size()));
        Bytes(value.data(), value.size());
    }
    uint64_t Value() const { return hash_; }
private:
    uint64_t hash_ = 14695981039346656037ULL;
};
}

void EffectDesc::Validate() const
{
    Require(Identifier(name) && name.size() < 128, "invalid effect name: " + name);
    Require(!shader_files.empty(), name + " has no HLSL source");
    for (auto const& file : shader_files)
        Require(!file.empty() && file.find_first_of("\"\r\n") == std::string::npos, "invalid source path");
    Require(structs.size() <= 65535, "too many structs");
    std::unordered_set<std::string> struct_names;
    for (auto const& structure : structs)
    {
        Require(Identifier(structure.name) && structure.name.size() < 256, "invalid struct name: " + structure.name);
        Require(!struct_names.contains(structure.name), "duplicate struct: " + structure.name);
        Require(!structure.members.empty() && structure.members.size() <= 255, "invalid member count: " + structure.name);
        std::unordered_set<std::string> members;
        for (auto const& member : structure.members)
        {
            Require(Identifier(member.name) && member.name.size() < 256, "invalid member name: " + member.name);
            Require(members.insert(member.name).second, "duplicate member: " + member.name);
            Require(member.type >= EffectParameterType::Bool && member.type <= EffectParameterType::Struct && Numeric(member.type),
                "invalid struct member type: " + member.name);
            Require(member.array_size <= 65535, "array too large: " + member.name);
            if (member.type == EffectParameterType::Struct)
                Require(struct_names.contains(member.struct_type), "struct dependency must be declared first: " + member.struct_type);
            else
                Require(member.struct_type.empty(), "unexpected struct type: " + member.name);
        }
        struct_names.insert(structure.name);
    }
    Require(parameters.size() <= 65535, "too many parameters");
    std::unordered_set<std::string> names, buffers;
    for (auto const& parameter : parameters)
    {
        Require(Identifier(parameter.name) && parameter.name.size() < 256, "invalid parameter name: " + parameter.name);
        Require(names.insert(parameter.name).second, "duplicate parameter: " + parameter.name);
        Require(!struct_names.contains(parameter.name), "struct/parameter name collision: " + parameter.name);
        Require(parameter.type >= EffectParameterType::Bool && parameter.type <= EffectParameterType::Struct,
            "unsupported parameter type: " + parameter.name);
        Require(parameter.semantic.size() < 256, "semantic is too long: " + parameter.name);
        Require(parameter.array_size <= 65535, "array too large: " + parameter.name);
        Require(InitialValueMatches(parameter), "initializer does not match parameter type: " + parameter.name);
        if (parameter.type == EffectParameterType::Struct)
            Require(struct_names.contains(parameter.struct_type), "unknown struct type: " + parameter.struct_type);
        else
            Require(parameter.struct_type.empty(), "unexpected struct type: " + parameter.name);
        if (Numeric(parameter.type))
        {
            Require(Identifier(parameter.constant_buffer) && parameter.constant_buffer.size() < 256,
                "invalid constant buffer: " + parameter.constant_buffer);
            buffers.insert(parameter.constant_buffer);
        }
        else
            Require(parameter.array_size == 0, "resource arrays are unsupported: " + parameter.name);
    }
    for (auto const& buffer : buffers)
        Require(!names.contains(buffer) && !struct_names.contains(buffer), "buffer name collision: " + buffer);
    ValidateMacros(macros, 65535);
    names.clear();
    Require(!techniques.empty() && techniques.size() <= 65535, "invalid technique count");
    for (auto const& technique : techniques)
    {
        Require(!technique.name.empty() && technique.name.size() < 256, "invalid technique name");
        Require(names.insert(technique.name).second, "duplicate technique: " + technique.name);
        ValidateMacros(technique.macros, 255);
        Require(!technique.passes.empty() && technique.passes.size() <= 255, "invalid pass count: " + technique.name);
        std::unordered_set<std::string> pass_names;
        for (auto const& pass : technique.passes)
        {
            Require(!pass.name.empty() && pass.name.size() < 256, "invalid pass name");
            Require(pass_names.insert(pass.name).second, "duplicate pass: " + pass.name);
            ValidateMacros(pass.macros, 255);
            auto has = [&](ShaderStage stage) { return !pass.entry_points[std::to_underlying(stage)].empty(); };
            for (auto const& entry : pass.entry_points)
                Require(entry.empty() || (Identifier(entry) && entry.size() < 256), "invalid shader entry: " + entry);
            if (has(ShaderStage::Compute))
                Require(std::count_if(pass.entry_points.begin(), pass.entry_points.end(), [](auto const& s) { return !s.empty(); }) == 1,
                    "compute and graphics stages cannot share a pass: " + pass.name);
            else
            {
                Require(has(ShaderStage::Vertex), "graphics pass has no vertex shader: " + pass.name);
                Require(has(ShaderStage::Hull) == has(ShaderStage::Domain), "hull/domain stages must be paired: " + pass.name);
            }
        }
    }
}

uint64_t EffectDesc::Fingerprint() const
{
    Validate();
    FingerprintBuilder hash;
    hash.Text("EffectDesc-v2");
    hash.Text(name);
    hash.Pod(static_cast<uint64_t>(shader_files.size()));
    for (auto const& file : shader_files) hash.Text(file);
    hash.Pod(static_cast<uint64_t>(structs.size()));
    for (auto const& structure : structs)
    {
        hash.Text(structure.name);
        hash.Pod(static_cast<uint64_t>(structure.members.size()));
        for (auto const& member : structure.members)
        {
            hash.Text(member.name); hash.Pod(member.type);
            hash.Text(member.struct_type); hash.Pod(member.array_size);
        }
    }
    hash.Pod(static_cast<uint64_t>(parameters.size()));
    for (auto const& parameter : parameters)
    {
        hash.Text(parameter.name); hash.Pod(parameter.type);
        hash.Text(parameter.constant_buffer); hash.Text(parameter.semantic);
        hash.Pod(parameter.array_size); hash.Text(parameter.struct_type);
        hash.Pod(static_cast<uint32_t>(parameter.initial_value.index()));
        std::visit([&](auto const& value)
        {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_arithmetic_v<T>) hash.Pod(value);
            else if constexpr (!std::is_same_v<T, std::monostate>)
                for (auto const& component : value) hash.Pod(component);
        }, parameter.initial_value);
        // Render state descriptors are fully initialized packed records, also
        // used byte-for-byte by the existing render-state cache and KFX format.
        if (parameter.type == EffectParameterType::Sampler) hash.Pod(parameter.sampler);
    }
    hash.Pod(static_cast<uint64_t>(macros.size()));
    for (auto const& [key, value] : macros) { hash.Text(key); hash.Text(value); }
    hash.Pod(static_cast<uint64_t>(techniques.size()));
    for (auto const& technique : techniques)
    {
        hash.Text(technique.name); hash.Pod(technique.minimum_shader_model.FullVersion());
        hash.Pod(static_cast<uint64_t>(technique.macros.size()));
        for (auto const& [key, value] : technique.macros) { hash.Text(key); hash.Text(value); }
        hash.Pod(static_cast<uint64_t>(technique.passes.size()));
        for (auto const& pass : technique.passes)
        {
            hash.Text(pass.name);
            for (auto const& entry : pass.entry_points) hash.Text(entry);
            hash.Pod(pass.rasterizer); hash.Pod(pass.depth_stencil); hash.Pod(pass.blend);
            hash.Pod(static_cast<uint64_t>(pass.macros.size()));
            for (auto const& [key, value] : pass.macros) { hash.Text(key); hash.Text(value); }
        }
    }
    return hash.Value();
}

std::string EffectDesc::CacheName(std::string_view platform) const
{
    return name + ".effect." + std::to_string(Fingerprint()) + "." + std::string(platform) + ".kfx";
}
}
