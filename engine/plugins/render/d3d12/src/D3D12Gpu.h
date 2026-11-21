/* GPU descriptor declarations adapted from SDL_gpu.h.
Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#pragma once
#include <common/macro.h>
#include <cstddef>
#include <cstdint>

#ifdef ZENGINE_BUILD_D3D12
#define ZENGINE_D3D12_API ZENGINE_SYMBOL_EXPORT
#else
#define ZENGINE_D3D12_API ZENGINE_SYMBOL_IMPORT
#endif

namespace RenderWorker::d3d12
{
struct Rect
{
    int x, y, w, h;
};
struct Color
{
    float r, g, b, a;
};
struct Device;
struct Buffer;
struct TransferBuffer;
struct Texture;
struct Sampler;
struct Shader;
struct GraphicsPipeline;
struct CommandBuffer;
struct RenderPass;
struct CopyPass;
struct Fence;
enum PrimitiveType
{
    PRIMITIVETYPE_TRIANGLELIST,
    PRIMITIVETYPE_TRIANGLESTRIP,
    PRIMITIVETYPE_LINELIST,
    PRIMITIVETYPE_LINESTRIP,
    PRIMITIVETYPE_POINTLIST
};
enum LoadOp
{
    LOADOP_LOAD,
    LOADOP_CLEAR,
    LOADOP_DONT_CARE
};
enum StoreOp
{
    STOREOP_STORE,
    STOREOP_DONT_CARE,
    STOREOP_RESOLVE,
    STOREOP_RESOLVE_AND_STORE
};
enum IndexElementSize
{
    INDEXELEMENTSIZE_16BIT,
    INDEXELEMENTSIZE_32BIT
};
enum TextureFormat
{
    TEXTUREFORMAT_INVALID,
    TEXTUREFORMAT_R8_UNORM,
    TEXTUREFORMAT_R8G8_UNORM,
    TEXTUREFORMAT_R8G8B8A8_UNORM,
    TEXTUREFORMAT_B8G8R8A8_UNORM,
    TEXTUREFORMAT_R10G10B10A2_UNORM,
    TEXTUREFORMAT_D16_UNORM,
    TEXTUREFORMAT_D24_UNORM_S8_UINT,
    TEXTUREFORMAT_D32_FLOAT_S8_UINT,
    TEXTUREFORMAT_D32_FLOAT,
    TEXTUREFORMAT_R16_FLOAT,
    TEXTUREFORMAT_R16G16_FLOAT,
    TEXTUREFORMAT_R16G16B16A16_FLOAT,
    TEXTUREFORMAT_R32_FLOAT,
    TEXTUREFORMAT_R32G32_FLOAT,
    TEXTUREFORMAT_R32G32B32A32_FLOAT,
    TEXTUREFORMAT_BC1_RGBA_UNORM,
    TEXTUREFORMAT_BC2_RGBA_UNORM,
    TEXTUREFORMAT_BC3_RGBA_UNORM,
    TEXTUREFORMAT_BC4_R_UNORM,
    TEXTUREFORMAT_BC5_RG_UNORM,
    TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
    TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB,
    TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB,
    TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB,
    TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB,
};
typedef uint32_t TextureUsageFlags;
enum TextureType
{
    TEXTURETYPE_2D,
    TEXTURETYPE_2D_ARRAY,
    TEXTURETYPE_3D,
    TEXTURETYPE_CUBE,
    TEXTURETYPE_CUBE_ARRAY
};
enum SampleCount
{
    SAMPLECOUNT_1,
    SAMPLECOUNT_2,
    SAMPLECOUNT_4,
    SAMPLECOUNT_8
};
typedef uint32_t BufferUsageFlags;
enum TransferBufferUsage
{
    TRANSFERBUFFERUSAGE_UPLOAD,
    TRANSFERBUFFERUSAGE_DOWNLOAD
};
enum ShaderStage
{
    SHADERSTAGE_VERTEX,
    SHADERSTAGE_FRAGMENT
};
typedef uint32_t ShaderFormat;
enum VertexElementFormat
{
    VERTEXELEMENTFORMAT_INVALID,
    VERTEXELEMENTFORMAT_FLOAT,
    VERTEXELEMENTFORMAT_FLOAT2,
    VERTEXELEMENTFORMAT_FLOAT3,
    VERTEXELEMENTFORMAT_FLOAT4,
    VERTEXELEMENTFORMAT_HALF4,
    VERTEXELEMENTFORMAT_SHORT2_NORM,
    VERTEXELEMENTFORMAT_SHORT4_NORM,
    VERTEXELEMENTFORMAT_USHORT2_NORM,
    VERTEXELEMENTFORMAT_USHORT4_NORM,
    VERTEXELEMENTFORMAT_BYTE4_NORM,
    VERTEXELEMENTFORMAT_UBYTE4_NORM,
    VERTEXELEMENTFORMAT_UBYTE4,
    VERTEXELEMENTFORMAT_USHORT4,
};
enum VertexInputRate
{
    VERTEXINPUTRATE_VERTEX,
    VERTEXINPUTRATE_INSTANCE
};
enum FillMode
{
    FILLMODE_FILL,
    FILLMODE_LINE
};
enum CullMode
{
    CULLMODE_NONE,
    CULLMODE_FRONT,
    CULLMODE_BACK
};
enum FrontFace
{
    FRONTFACE_COUNTER_CLOCKWISE,
    FRONTFACE_CLOCKWISE
};
enum CompareOp
{
    COMPAREOP_INVALID,
    COMPAREOP_NEVER,
    COMPAREOP_LESS,
    COMPAREOP_EQUAL,
    COMPAREOP_LESS_OR_EQUAL,
    COMPAREOP_GREATER,
    COMPAREOP_NOT_EQUAL,
    COMPAREOP_GREATER_OR_EQUAL,
    COMPAREOP_ALWAYS
};
enum StencilOp
{
    STENCILOP_INVALID,
    STENCILOP_KEEP,
    STENCILOP_ZERO,
    STENCILOP_REPLACE,
    STENCILOP_INCREMENT_AND_CLAMP,
    STENCILOP_DECREMENT_AND_CLAMP,
    STENCILOP_INVERT,
    STENCILOP_INCREMENT_AND_WRAP,
    STENCILOP_DECREMENT_AND_WRAP
};
enum BlendOp
{
    BLENDOP_INVALID,
    BLENDOP_ADD,
    BLENDOP_SUBTRACT,
    BLENDOP_REVERSE_SUBTRACT,
    BLENDOP_MIN,
    BLENDOP_MAX
};
enum BlendFactor
{
    BLENDFACTOR_INVALID,
    BLENDFACTOR_ZERO,
    BLENDFACTOR_ONE,
    BLENDFACTOR_SRC_COLOR,
    BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    BLENDFACTOR_DST_COLOR,
    BLENDFACTOR_ONE_MINUS_DST_COLOR,
    BLENDFACTOR_SRC_ALPHA,
    BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    BLENDFACTOR_DST_ALPHA,
    BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    BLENDFACTOR_CONSTANT_COLOR,
    BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR,
    BLENDFACTOR_SRC_ALPHA_SATURATE
};
typedef uint8_t ColorComponentFlags;
enum Filter
{
    FILTER_NEAREST,
    FILTER_LINEAR
};
enum SamplerMipmapMode
{
    SAMPLERMIPMAPMODE_NEAREST,
    SAMPLERMIPMAPMODE_LINEAR
};
enum SamplerAddressMode
{
    SAMPLERADDRESSMODE_REPEAT,
    SAMPLERADDRESSMODE_MIRRORED_REPEAT,
    SAMPLERADDRESSMODE_CLAMP_TO_EDGE
};
struct Viewport
{
    float x;
    float y;
    float w;
    float h;
    float min_depth;
    float max_depth;
};
struct TextureTransferInfo
{
    TransferBuffer *transfer_buffer;
    uint32_t offset;
    uint32_t pixels_per_row;
    uint32_t rows_per_layer;
};
struct TransferBufferLocation
{
    TransferBuffer *transfer_buffer;
    uint32_t offset;
};
struct TextureLocation
{
    Texture *texture;
    uint32_t mip_level;
    uint32_t layer;
    uint32_t x;
    uint32_t y;
    uint32_t z;
};
struct TextureRegion
{
    Texture *texture;
    uint32_t mip_level;
    uint32_t layer;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t w;
    uint32_t h;
    uint32_t d;
};
struct BufferRegion
{
    Buffer *buffer;
    uint32_t offset;
    uint32_t size;
};
struct SamplerCreateInfo
{
    Filter min_filter;
    Filter mag_filter;
    SamplerMipmapMode mipmap_mode;
    SamplerAddressMode address_mode_u;
    SamplerAddressMode address_mode_v;
    SamplerAddressMode address_mode_w;
    float mip_lod_bias;
    float max_anisotropy;
    CompareOp compare_op;
    float min_lod;
    float max_lod;
    bool enable_anisotropy;
    bool enable_compare;
};
struct VertexBufferDescription
{
    uint32_t slot;
    uint32_t pitch;
    VertexInputRate input_rate;
    uint32_t instance_step_rate;
};
struct VertexAttribute
{
    uint32_t location;
    uint32_t buffer_slot;
    VertexElementFormat format;
    uint32_t offset;
};
struct VertexInputState
{
    const VertexBufferDescription *vertex_buffer_descriptions;
    uint32_t num_vertex_buffers;
    const VertexAttribute *vertex_attributes;
    uint32_t num_vertex_attributes;
};
struct StencilOpState
{
    StencilOp fail_op;
    StencilOp pass_op;
    StencilOp depth_fail_op;
    CompareOp compare_op;
};
struct ColorTargetBlendState
{
    BlendFactor src_color_blendfactor;
    BlendFactor dst_color_blendfactor;
    BlendOp color_blend_op;
    BlendFactor src_alpha_blendfactor;
    BlendFactor dst_alpha_blendfactor;
    BlendOp alpha_blend_op;
    ColorComponentFlags color_write_mask;
    bool enable_blend;
    bool enable_color_write_mask;
};
struct ShaderCreateInfo
{
    size_t code_size;
    const uint8_t *code;
    const char *entrypoint;
    ShaderFormat format;
    ShaderStage stage;
    uint32_t num_samplers;
    uint32_t num_storage_textures;
    uint32_t num_storage_buffers;
    uint32_t num_uniform_buffers;
};
struct TextureCreateInfo
{
    TextureType type;
    TextureFormat format;
    TextureUsageFlags usage;
    uint32_t width;
    uint32_t height;
    uint32_t layer_count_or_depth;
    uint32_t num_levels;
    SampleCount sample_count;
    float clear_color[4]{};
};
struct BufferCreateInfo
{
    BufferUsageFlags usage;
    uint32_t size;
};
struct TransferBufferCreateInfo
{
    TransferBufferUsage usage;
    uint32_t size;
};
struct RasterizerState
{
    FillMode fill_mode;
    CullMode cull_mode;
    FrontFace front_face;
    float depth_bias_constant_factor;
    float depth_bias_clamp;
    float depth_bias_slope_factor;
    bool enable_depth_bias;
    bool enable_depth_clip;
};
struct MultisampleState
{
    SampleCount sample_count;
    uint32_t sample_mask;
    bool enable_mask;
    bool enable_alpha_to_coverage;
};
struct DepthStencilState
{
    CompareOp compare_op;
    StencilOpState back_stencil_state;
    StencilOpState front_stencil_state;
    uint8_t compare_mask;
    uint8_t write_mask;
    bool enable_depth_test;
    bool enable_depth_write;
    bool enable_stencil_test;
};
struct ColorTargetDescription
{
    TextureFormat format;
    ColorTargetBlendState blend_state;
};
struct GraphicsPipelineTargetInfo
{
    const ColorTargetDescription *color_target_descriptions;
    uint32_t num_color_targets;
    TextureFormat depth_stencil_format;
    bool has_depth_stencil_target;
};
struct GraphicsPipelineCreateInfo
{
    Shader *vertex_shader;
    Shader *fragment_shader;
    VertexInputState vertex_input_state;
    PrimitiveType primitive_type;
    RasterizerState rasterizer_state;
    MultisampleState multisample_state;
    DepthStencilState depth_stencil_state;
    GraphicsPipelineTargetInfo target_info;
};
struct ColorTargetInfo
{
    Texture *texture;
    uint32_t mip_level;
    uint32_t layer_or_depth_plane;
    Color clear_color;
    LoadOp load_op;
    StoreOp store_op;
    Texture *resolve_texture;
    uint32_t resolve_mip_level;
    uint32_t resolve_layer;
    bool cycle;
    bool cycle_resolve_texture;
};
struct DepthStencilTargetInfo
{
    Texture *texture;
    float clear_depth;
    LoadOp load_op;
    StoreOp store_op;
    LoadOp stencil_load_op;
    StoreOp stencil_store_op;
    bool cycle;
    uint8_t clear_stencil;
    uint8_t mip_level;
    uint8_t layer;
};
struct BufferBinding
{
    Buffer *buffer;
    uint32_t offset;
};
struct TextureSamplerBinding
{
    Texture *texture;
    Sampler *sampler;
};
inline constexpr auto TEXTUREUSAGE_SAMPLER = (1u << 0);
inline constexpr auto TEXTUREUSAGE_COLOR_TARGET = (1u << 1);
inline constexpr auto TEXTUREUSAGE_DEPTH_STENCIL_TARGET = (1u << 2);
inline constexpr auto BUFFERUSAGE_VERTEX = (1u << 0);
inline constexpr auto BUFFERUSAGE_INDEX = (1u << 1);
inline constexpr auto SHADERFORMAT_DXBC = (1u << 2);
inline constexpr auto SHADERFORMAT_DXIL = (1u << 3);
ZENGINE_D3D12_API void DestroyDevice(Device *device);
ZENGINE_D3D12_API ShaderFormat GetShaderFormats(Device *device);
ZENGINE_D3D12_API GraphicsPipeline *CreateGraphicsPipeline(Device *device, const GraphicsPipelineCreateInfo *createinfo);
ZENGINE_D3D12_API Sampler *CreateSampler(Device *device, const SamplerCreateInfo *createinfo);
ZENGINE_D3D12_API Shader *CreateShader(Device *device, const ShaderCreateInfo *createinfo);
ZENGINE_D3D12_API Texture *CreateTexture(Device *device, const TextureCreateInfo *createinfo);
ZENGINE_D3D12_API Buffer *CreateBuffer(Device *device, const BufferCreateInfo *createinfo);
ZENGINE_D3D12_API TransferBuffer *CreateTransferBuffer(Device *device, const TransferBufferCreateInfo *createinfo);
ZENGINE_D3D12_API void ReleaseTexture(Device *device, Texture *texture);
ZENGINE_D3D12_API void ReleaseSampler(Device *device, Sampler *sampler);
ZENGINE_D3D12_API void ReleaseBuffer(Device *device, Buffer *buffer);
ZENGINE_D3D12_API void ReleaseTransferBuffer(Device *device, TransferBuffer *transfer_buffer);
ZENGINE_D3D12_API void ReleaseShader(Device *device, Shader *shader);
ZENGINE_D3D12_API void ReleaseGraphicsPipeline(Device *device, GraphicsPipeline *graphics_pipeline);
ZENGINE_D3D12_API CommandBuffer *AcquireCommandBuffer(Device *device);
ZENGINE_D3D12_API void PushVertexUniformData(CommandBuffer *command_buffer, uint32_t slot_index, const void *data,
                                           uint32_t length);
ZENGINE_D3D12_API void PushFragmentUniformData(CommandBuffer *command_buffer, uint32_t slot_index, const void *data,
                                             uint32_t length);
ZENGINE_D3D12_API RenderPass *BeginRenderPass(CommandBuffer *command_buffer, const ColorTargetInfo *color_target_infos,
                                            uint32_t num_color_targets,
                                            const DepthStencilTargetInfo *depth_stencil_target_info);
ZENGINE_D3D12_API void BindGraphicsPipeline(RenderPass *render_pass, GraphicsPipeline *graphics_pipeline);
ZENGINE_D3D12_API void SetViewport(RenderPass *render_pass, const Viewport *viewport);
ZENGINE_D3D12_API void SetScissor(RenderPass *render_pass, const Rect *scissor);
ZENGINE_D3D12_API void BindVertexBuffers(RenderPass *render_pass, uint32_t first_slot, const BufferBinding *bindings,
                                       uint32_t num_bindings);
ZENGINE_D3D12_API void BindIndexBuffer(RenderPass *render_pass, const BufferBinding *binding,
                                     IndexElementSize index_element_size);
ZENGINE_D3D12_API void BindVertexSamplers(RenderPass *render_pass, uint32_t first_slot,
                                        const TextureSamplerBinding *texture_sampler_bindings, uint32_t num_bindings);
ZENGINE_D3D12_API void BindFragmentSamplers(RenderPass *render_pass, uint32_t first_slot,
                                          const TextureSamplerBinding *texture_sampler_bindings, uint32_t num_bindings);
ZENGINE_D3D12_API void DrawIndexedPrimitives(RenderPass *render_pass, uint32_t num_indices, uint32_t num_instances,
                                           uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
ZENGINE_D3D12_API void DrawPrimitives(RenderPass *render_pass, uint32_t num_vertices, uint32_t num_instances,
                                    uint32_t first_vertex, uint32_t first_instance);
ZENGINE_D3D12_API void EndRenderPass(RenderPass *render_pass);
ZENGINE_D3D12_API void *MapTransferBuffer(Device *device, TransferBuffer *transfer_buffer, bool cycle);
ZENGINE_D3D12_API void UnmapTransferBuffer(Device *device, TransferBuffer *transfer_buffer);
ZENGINE_D3D12_API CopyPass *BeginCopyPass(CommandBuffer *command_buffer);
ZENGINE_D3D12_API void UploadToTexture(CopyPass *copy_pass, const TextureTransferInfo *source,
                                     const TextureRegion *destination, bool cycle);
ZENGINE_D3D12_API void UploadToBuffer(CopyPass *copy_pass, const TransferBufferLocation *source,
                                    const BufferRegion *destination, bool cycle);
ZENGINE_D3D12_API void CopyTextureToTexture(CopyPass *copy_pass, const TextureLocation *source,
                                          const TextureLocation *destination, uint32_t w, uint32_t h, uint32_t d,
                                          bool cycle);
ZENGINE_D3D12_API void DownloadFromTexture(CopyPass *copy_pass, const TextureRegion *source,
                                         const TextureTransferInfo *destination);
ZENGINE_D3D12_API void EndCopyPass(CopyPass *copy_pass);
ZENGINE_D3D12_API TextureFormat GetSwapchainTextureFormat(Device *device, void *window);
ZENGINE_D3D12_API bool WaitAndAcquireSwapchainTexture(CommandBuffer *command_buffer, void *window,
                                                    Texture **swapchain_texture, uint32_t *swapchain_texture_width,
                                                    uint32_t *swapchain_texture_height);
ZENGINE_D3D12_API bool SubmitCommandBuffer(CommandBuffer *command_buffer);
ZENGINE_D3D12_API Fence *SubmitCommandBufferAndAcquireFence(CommandBuffer *command_buffer);
ZENGINE_D3D12_API bool CancelCommandBuffer(CommandBuffer *command_buffer);
ZENGINE_D3D12_API bool WaitForIdle(Device *device);
ZENGINE_D3D12_API bool WaitForFences(Device *device, bool wait_all, Fence *const *fences, uint32_t num_fences);
ZENGINE_D3D12_API void ReleaseFence(Device *device, Fence *fence);
ZENGINE_D3D12_API bool TextureSupportsFormat(Device *device, TextureFormat format, TextureType type,
                                           TextureUsageFlags usage);
ZENGINE_D3D12_API Device *CreateDevice(void *native_window, unsigned sync_interval, bool debug);
ZENGINE_D3D12_API const char *LastError();
ZENGINE_D3D12_API void *NativeDevice(Device *);
ZENGINE_D3D12_API void *TextureHandle(Texture *);
ZENGINE_D3D12_API void *NativeCommands(CommandBuffer *);
ZENGINE_D3D12_API void *NativeEncoder(RenderPass *);
}
