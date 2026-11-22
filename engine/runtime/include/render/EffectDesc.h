#pragma once

#include <base/ZEngine.h>
#include <render/RenderDeviceCaps.h>
#include <render/RenderStateObject.h>
#include <render/ShaderObject.h>

#include <array>
#include <string>
#include <variant>
#include <vector>

namespace RenderWorker
{
enum class EffectParameterType
{
    Bool, UInt, Int, Float, Float2, Float3, Float4, Float4x4,
    Texture2D, TextureCube, Sampler,
    Int2, Int3, Int4, UInt2, UInt3, UInt4, Struct
};

using EffectInitialValue = std::variant<std::monostate, bool, uint32_t, int32_t, float,
    float2, float3, float4, float4x4, int2, int3, int4, uint2, uint3, uint4>;

struct EffectParameterDesc
{
    std::string name;
    EffectParameterType type = EffectParameterType::Float;
    std::string constant_buffer = "global_cb"; // Numeric and struct parameters only.
    std::string semantic;
    SamplerStateDesc sampler; // Used only by Sampler parameters.
    uint32_t array_size = 0; // Zero denotes a non-array parameter.
    std::string struct_type; // Used only by Struct parameters.
    EffectInitialValue initial_value; // Arrays/structs have no explicit initializer.
};

struct EffectStructMemberDesc
{
    std::string name;
    EffectParameterType type = EffectParameterType::Float;
    std::string struct_type;
    uint32_t array_size = 0;
};

struct EffectStructDesc
{
    std::string name;
    std::vector<EffectStructMemberDesc> members;
};

struct PassDesc
{
    std::string name;
    std::array<std::string, ShaderStageNum> entry_points;
    RasterizerStateDesc rasterizer;
    DepthStencilStateDesc depth_stencil;
    BlendStateDesc blend;
    std::vector<std::pair<std::string, std::string>> macros;
};

struct TechniqueDesc
{
    std::string name;
    ShaderModel minimum_shader_model{0, 0};
    std::vector<PassDesc> passes;
    std::vector<std::pair<std::string, std::string>> macros;
};

struct ZENGINE_CORE_API EffectDesc
{
    std::string name;
    // Ordered HLSL files, concatenated after the generated parameter declarations.
    // List shared .hlsli files here too; every file participates in cache invalidation.
    std::vector<std::string> shader_files;
    std::vector<EffectParameterDesc> parameters;
    std::vector<std::pair<std::string, std::string>> macros;
    std::vector<TechniqueDesc> techniques;
    // Declare dependencies before structures that contain them.
    std::vector<EffectStructDesc> structs;

    void Validate() const; // Throws invalid_argument before creating GPU resources.
    uint64_t Fingerprint() const;
    std::string CacheName(std::string_view platform) const;
};
}
