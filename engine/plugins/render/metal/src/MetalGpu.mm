#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include "MetalGpu.h"
#include <algorithm>
#include <memory>
#include <string>

namespace RenderWorker::metal
{
struct Device
{
    id<MTLDevice> metal;
    id<MTLCommandQueue> queue;
    CAMetalLayer *layer;
    NSView *view;
};
struct Texture
{
    id<MTLTexture> object;
    TextureFormat format;
};
struct Buffer
{
    id<MTLBuffer> object;
};
struct TransferBuffer
{
    id<MTLBuffer> object;
};
struct Shader
{
    id<MTLFunction> function;
};
struct Sampler
{
    id<MTLSamplerState> state;
};
struct GraphicsPipeline
{
    id<MTLRenderPipelineState> state;
    id<MTLDepthStencilState> depth;
    MTLPrimitiveType primitive;
    MTLCullMode cull;
    MTLWinding winding;
    MTLTriangleFillMode fill;
};
struct RenderPass
{
    MTLRenderPassDescriptor *descriptor;
    id<MTLRenderCommandEncoder> encoder;
    MTLPrimitiveType primitive;
    id<MTLBuffer> indices;
    NSUInteger offset;
    MTLIndexType index_type;
};
struct CopyPass
{
    id<MTLBlitCommandEncoder> encoder;
};
struct CommandBuffer
{
    Device *device;
    id<MTLCommandBuffer> commands;
    id<CAMetalDrawable> drawable;
    Texture backbuffer;
    RenderPass render;
    CopyPass copy;
};
struct Fence
{
    id<MTLCommandBuffer> commands;
};
static thread_local std::string error;
const char *LastError()
{
    return error.c_str();
}

struct Pixel
{
    MTLPixelFormat format;
    NSUInteger bytes, block;
};
static Pixel Describe(TextureFormat format)
{
    switch (format)
    {
#define PIXEL_FORMAT(id, metal, bytes, block)                                                                    \
    case TEXTUREFORMAT_##id:                                                                                           \
        return {MTLPixelFormat##metal, bytes, block};
#include "PixelFormats.inc"
#undef PIXEL_FORMAT
    default:
        return {MTLPixelFormatInvalid, 0, 1};
    }
}
static MTLVertexFormat Attribute(VertexElementFormat format)
{
    switch (format)
    {
#define VERTEX_FORMAT(id, metal)                                                                                 \
    case VERTEXELEMENTFORMAT_##id:                                                                                     \
        return MTLVertexFormat##metal;
#include "VertexFormats.inc"
#undef VERTEX_FORMAT
    default:
        return MTLVertexFormatInvalid;
    }
}
Device *CreateDevice(void *window, unsigned sync, bool)
{
    auto device = std::make_unique<Device>();
    device->metal = MTLCreateSystemDefaultDevice();
    device->queue = [device->metal newCommandQueue];
    if (!device->queue)
    {
        error = "Metal command queue unavailable";
        return nullptr;
    }
    device->view = [(__bridge NSWindow *)window contentView];
    auto layer = [CAMetalLayer layer];
    layer.device = device->metal;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = NO;
    layer.displaySyncEnabled = sync != 0;
    device->view.wantsLayer = YES;
    device->view.layer = layer;
    device->layer = layer;
    return device.release();
}
void DestroyDevice(Device *d)
{
    WaitForIdle(d);
    if (d->view.layer == d->layer)
        d->view.layer = nil;
    delete d;
}
void *NativeDevice(Device *d)
{
    return (__bridge void *)d->metal;
}
void *TextureHandle(Texture *t)
{
    return t ? (__bridge void *)t->object : nullptr;
}
void *NativeCommands(CommandBuffer *c)
{
    return c ? (__bridge void *)c->commands : nullptr;
}
void *NativePassDescriptor(RenderPass *p)
{
    return (__bridge void *)p->descriptor;
}
void *NativeEncoder(RenderPass *p)
{
    return p ? (__bridge void *)p->encoder : nullptr;
}
ShaderFormat GetShaderFormats(Device *)
{
    return SHADERFORMAT_MSL;
}
TextureFormat GetSwapchainTextureFormat(Device *, void *)
{
    return TEXTUREFORMAT_B8G8R8A8_UNORM;
}
bool TextureSupportsFormat(Device *d, TextureFormat f, TextureType, TextureUsageFlags)
{
    if (f == TEXTUREFORMAT_D24_UNORM_S8_UINT)
        return d->metal.depth24Stencil8PixelFormatSupported;
    auto info = Describe(f);
    if (info.block == 4 && !d->metal.supportsBCTextureCompression)
        return false;
    return info.format != MTLPixelFormatInvalid;
}
Texture *CreateTexture(Device *d, const TextureCreateInfo *desc)
{
    auto td = [MTLTextureDescriptor new];
    constexpr MTLTextureType types[] = {MTLTextureType2D, MTLTextureType2DArray, MTLTextureType3D, MTLTextureTypeCube,
                                        MTLTextureTypeCubeArray};
    td.textureType = types[desc->type];
    td.width = desc->width;
    td.height = desc->height;
    td.depth = desc->type == TEXTURETYPE_3D ? desc->layer_count_or_depth : 1;
    if (desc->type == TEXTURETYPE_2D_ARRAY)
        td.arrayLength = desc->layer_count_or_depth;
    if (desc->type == TEXTURETYPE_CUBE_ARRAY)
        td.arrayLength = desc->layer_count_or_depth / 6;
    td.mipmapLevelCount = desc->num_levels;
    td.pixelFormat = Describe(desc->format).format;
    td.storageMode = MTLStorageModePrivate;
    td.usage = MTLTextureUsageUnknown;
    if (desc->usage & TEXTUREUSAGE_SAMPLER)
        td.usage |= MTLTextureUsageShaderRead;
    if (desc->usage & (TEXTUREUSAGE_COLOR_TARGET | TEXTUREUSAGE_DEPTH_STENCIL_TARGET))
        td.usage |= MTLTextureUsageRenderTarget;
    auto tex = [d->metal newTextureWithDescriptor:td];
    return tex ? new Texture{tex, desc->format} : nullptr;
}
Buffer *CreateBuffer(Device *d, const BufferCreateInfo *i)
{
    auto b = [d->metal newBufferWithLength:i->size options:MTLResourceStorageModePrivate];
    return b ? new Buffer{b} : nullptr;
}
TransferBuffer *CreateTransferBuffer(Device *d, const TransferBufferCreateInfo *i)
{
    auto b = [d->metal newBufferWithLength:i->size options:MTLResourceStorageModeShared];
    return b ? new TransferBuffer{b} : nullptr;
}
void *MapTransferBuffer(Device *, TransferBuffer *b, bool)
{
    return b->object.contents;
}
void UnmapTransferBuffer(Device *, TransferBuffer *)
{
}
void ReleaseTexture(Device *, Texture *r)
{
    delete r;
}
void ReleaseBuffer(Device *, Buffer *r)
{
    delete r;
}
void ReleaseTransferBuffer(Device *, TransferBuffer *r)
{
    delete r;
}
void ReleaseShader(Device *, Shader *r)
{
    delete r;
}
void ReleaseSampler(Device *, Sampler *r)
{
    delete r;
}
void ReleaseGraphicsPipeline(Device *, GraphicsPipeline *r)
{
    delete r;
}
Shader *CreateShader(Device *d, const ShaderCreateInfo *i)
{
    NSError *err = nil;
    auto source = [NSString stringWithUTF8String:reinterpret_cast<const char *>(i->code)];
    auto library = [d->metal newLibraryWithSource:source options:nil error:&err];
    if (!library)
    {
        error = err.localizedDescription.UTF8String;
        return nullptr;
    }
    auto f = [library newFunctionWithName:[NSString stringWithUTF8String:i->entrypoint]];
    if (!f)
    {
        error = std::string("Missing Metal entry point ") + i->entrypoint;
        return nullptr;
    }
    return new Shader{f};
}
static MTLCompareFunction Comparison(CompareOp op)
{
    return op == COMPAREOP_INVALID ? MTLCompareFunctionAlways : static_cast<MTLCompareFunction>(op - 1);
}
Sampler *CreateSampler(Device *d, const SamplerCreateInfo *i)
{
    MTLSamplerAddressMode const modes[] = {MTLSamplerAddressModeRepeat, MTLSamplerAddressModeMirrorRepeat,
                                           MTLSamplerAddressModeClampToEdge};
    auto desc = [MTLSamplerDescriptor new];
    desc.sAddressMode = modes[i->address_mode_u];
    desc.tAddressMode = modes[i->address_mode_v];
    desc.rAddressMode = modes[i->address_mode_w];
    desc.minFilter = (MTLSamplerMinMagFilter)i->min_filter;
    desc.magFilter = (MTLSamplerMinMagFilter)i->mag_filter;
    desc.mipFilter =
        i->mipmap_mode == SAMPLERMIPMAPMODE_LINEAR ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;
    desc.maxAnisotropy = i->enable_anisotropy ? NSUInteger(i->max_anisotropy) : 1;
    desc.lodMinClamp = std::max(0.f, i->min_lod);
    desc.lodMaxClamp = std::max(desc.lodMinClamp, i->max_lod);
    if (i->enable_compare)
        desc.compareFunction = Comparison(i->compare_op);
    auto obj = [d->metal newSamplerStateWithDescriptor:desc];
    return obj ? new Sampler{obj} : nullptr;
}
static MTLBlendFactor Factor(BlendFactor f)
{
    static constexpr MTLBlendFactor factors[] = {MTLBlendFactorZero,
                                                 MTLBlendFactorZero,
                                                 MTLBlendFactorOne,
                                                 MTLBlendFactorSourceColor,
                                                 MTLBlendFactorOneMinusSourceColor,
                                                 MTLBlendFactorDestinationColor,
                                                 MTLBlendFactorOneMinusDestinationColor,
                                                 MTLBlendFactorSourceAlpha,
                                                 MTLBlendFactorOneMinusSourceAlpha,
                                                 MTLBlendFactorDestinationAlpha,
                                                 MTLBlendFactorOneMinusDestinationAlpha,
                                                 MTLBlendFactorBlendColor,
                                                 MTLBlendFactorOneMinusBlendColor,
                                                 MTLBlendFactorSourceAlphaSaturated};
    return factors[f];
}
GraphicsPipeline *CreateGraphicsPipeline(Device *d, const GraphicsPipelineCreateInfo *i)
{
    auto desc = [MTLRenderPipelineDescriptor new];
    desc.vertexFunction = i->vertex_shader->function;
    desc.fragmentFunction = i->fragment_shader->function;
    auto vertex = [MTLVertexDescriptor vertexDescriptor];
    auto const &input = i->vertex_input_state;
    for (unsigned n = 0; n < input.num_vertex_buffers; ++n)
    {
        auto const &b = input.vertex_buffer_descriptions[n];
        auto layout = vertex.layouts[30 - b.slot];
        layout.stride = b.pitch;
        layout.stepFunction = b.input_rate == VERTEXINPUTRATE_INSTANCE ? MTLVertexStepFunctionPerInstance
                                                                       : MTLVertexStepFunctionPerVertex;
        layout.stepRate = 1;
    }
    for (unsigned n = 0; n < input.num_vertex_attributes; ++n)
    {
        auto const &a = input.vertex_attributes[n];
        auto attr = vertex.attributes[a.location];
        attr.format = Attribute(a.format);
        attr.offset = a.offset;
        attr.bufferIndex = 30 - a.buffer_slot;
    }
    desc.vertexDescriptor = vertex;
    auto const &targets = i->target_info;
    for (unsigned n = 0; n < targets.num_color_targets; ++n)
    {
        auto const &color = targets.color_target_descriptions[n];
        auto a = desc.colorAttachments[n];
        a.pixelFormat = Describe(color.format).format;
        auto const &b = color.blend_state;
        a.blendingEnabled = b.enable_blend;
        if (b.enable_blend)
        {
            a.rgbBlendOperation = (MTLBlendOperation)(b.color_blend_op - 1);
            a.alphaBlendOperation = (MTLBlendOperation)(b.alpha_blend_op - 1);
            a.sourceRGBBlendFactor = Factor(b.src_color_blendfactor);
            a.destinationRGBBlendFactor = Factor(b.dst_color_blendfactor);
            a.sourceAlphaBlendFactor = Factor(b.src_alpha_blendfactor);
            a.destinationAlphaBlendFactor = Factor(b.dst_alpha_blendfactor);
        }
        if (b.enable_color_write_mask)
        {
            unsigned mask = b.color_write_mask;
            a.writeMask = ((mask & 1) ? MTLColorWriteMaskRed : 0) | ((mask & 2) ? MTLColorWriteMaskGreen : 0) |
                          ((mask & 4) ? MTLColorWriteMaskBlue : 0) | ((mask & 8) ? MTLColorWriteMaskAlpha : 0);
        }
    }
    auto depth = [MTLDepthStencilDescriptor new];
    if (targets.has_depth_stencil_target)
    {
        desc.depthAttachmentPixelFormat = Describe(targets.depth_stencil_format).format;
        if (targets.depth_stencil_format == TEXTUREFORMAT_D24_UNORM_S8_UINT ||
            targets.depth_stencil_format == TEXTUREFORMAT_D32_FLOAT_S8_UINT)
            desc.stencilAttachmentPixelFormat = desc.depthAttachmentPixelFormat;
        depth.depthCompareFunction = i->depth_stencil_state.enable_depth_test
                                         ? Comparison(i->depth_stencil_state.compare_op)
                                         : MTLCompareFunctionAlways;
        depth.depthWriteEnabled = i->depth_stencil_state.enable_depth_write;
    }
    NSError *err = nil;
    auto state = [d->metal newRenderPipelineStateWithDescriptor:desc error:&err];
    if (!state)
    {
        error = err.localizedDescription.UTF8String;
        return nullptr;
    }
    constexpr MTLPrimitiveType primitives[] = {MTLPrimitiveTypeTriangle, MTLPrimitiveTypeTriangleStrip,
                                               MTLPrimitiveTypeLine, MTLPrimitiveTypeLineStrip, MTLPrimitiveTypePoint};
    return new GraphicsPipeline{
        state,
        [d->metal newDepthStencilStateWithDescriptor:depth],
        primitives[i->primitive_type],
        (MTLCullMode)i->rasterizer_state.cull_mode,
        i->rasterizer_state.front_face == FRONTFACE_CLOCKWISE ? MTLWindingClockwise : MTLWindingCounterClockwise,
        i->rasterizer_state.fill_mode == FILLMODE_LINE ? MTLTriangleFillModeLines : MTLTriangleFillModeFill};
}
CommandBuffer *AcquireCommandBuffer(Device *d)
{
    auto commands = [d->queue commandBuffer];
    return commands ? new CommandBuffer{d, commands} : nullptr;
}
bool WaitAndAcquireSwapchainTexture(CommandBuffer *c, void *, Texture **tex, uint32_t *w, uint32_t *h)
{
    auto d = c->device;
    NSSize size = [d->view convertSizeToBacking:d->view.bounds.size];
    *tex = nullptr;
    if (size.width < 1 || size.height < 1 || d->view.window.miniaturized)
        return true;
    if (!CGSizeEqualToSize(d->layer.drawableSize, size))
        d->layer.drawableSize = size;
    c->drawable = [d->layer nextDrawable];
    if (!c->drawable)
        return true;
    c->backbuffer = {c->drawable.texture, TEXTUREFORMAT_B8G8R8A8_UNORM};
    *w = c->backbuffer.object.width;
    *h = c->backbuffer.object.height;
    *tex = &c->backbuffer;
    return true;
}
static MTLLoadAction Load(LoadOp op)
{
    return op == LOADOP_CLEAR ? MTLLoadActionClear : op == LOADOP_LOAD ? MTLLoadActionLoad : MTLLoadActionDontCare;
}
RenderPass *BeginRenderPass(CommandBuffer *c, const ColorTargetInfo *colors, uint32_t n,
                            const DepthStencilTargetInfo *ds)
{
    auto desc = [MTLRenderPassDescriptor renderPassDescriptor];
    for (unsigned j = 0; j < n; ++j)
    {
        auto a = desc.colorAttachments[j];
        auto const &src = colors[j];
        a.texture = src.texture->object;
        a.level = src.mip_level;
        a.slice = src.layer_or_depth_plane;
        a.loadAction = Load(src.load_op);
        a.storeAction = MTLStoreActionStore;
        auto v = src.clear_color;
        a.clearColor = MTLClearColorMake(v.r, v.g, v.b, v.a);
    }
    if (ds)
    {
        auto a = desc.depthAttachment;
        a.texture = ds->texture->object;
        a.loadAction = Load(ds->load_op);
        a.storeAction = MTLStoreActionStore;
        a.clearDepth = ds->clear_depth;
        auto fmt = ds->texture->format;
        if (fmt == TEXTUREFORMAT_D24_UNORM_S8_UINT || fmt == TEXTUREFORMAT_D32_FLOAT_S8_UINT)
        {
            auto stencil = desc.stencilAttachment;
            stencil.texture = a.texture;
            stencil.clearStencil = ds->clear_stencil;
            stencil.loadAction = Load(ds->stencil_load_op);
            stencil.storeAction = MTLStoreActionStore;
        }
    }
    c->render.descriptor = desc;
    c->render.encoder = [c->commands renderCommandEncoderWithDescriptor:desc];
    return c->render.encoder ? &c->render : nullptr;
}
void BindGraphicsPipeline(RenderPass *p, GraphicsPipeline *s)
{
    auto e = p->encoder;
    [e setRenderPipelineState:s->state];
    [e setDepthStencilState:s->depth];
    [e setCullMode:s->cull];
    [e setFrontFacingWinding:s->winding];
    [e setTriangleFillMode:s->fill];
    p->primitive = s->primitive;
}
void SetViewport(RenderPass *p, const Viewport *v)
{
    [p->encoder setViewport:MTLViewport{v->x, v->y, v->w, v->h, v->min_depth, v->max_depth}];
}
void SetScissor(RenderPass *p, const Rect *r)
{
    [p->encoder setScissorRect:MTLScissorRect{NSUInteger(r->x), NSUInteger(r->y), NSUInteger(r->w), NSUInteger(r->h)}];
}
void BindVertexBuffers(RenderPass *p, uint32_t first, const BufferBinding *b, uint32_t n)
{
    for (unsigned i = 0; i < n; ++i)
        [p->encoder setVertexBuffer:b[i].buffer->object offset:b[i].offset atIndex:30 - first - i];
}
void BindIndexBuffer(RenderPass *p, const BufferBinding *b, IndexElementSize size)
{
    p->indices = b->buffer->object;
    p->offset = b->offset;
    p->index_type = size == INDEXELEMENTSIZE_16BIT ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
}
void BindVertexSamplers(RenderPass *p, uint32_t first, const TextureSamplerBinding *b, uint32_t n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        [p->encoder setVertexTexture:b[i].texture->object atIndex:first + i];
        [p->encoder setVertexSamplerState:b[i].sampler->state atIndex:first + i];
    }
}
void BindFragmentSamplers(RenderPass *p, uint32_t first, const TextureSamplerBinding *b, uint32_t n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        [p->encoder setFragmentTexture:b[i].texture->object atIndex:first + i];
        [p->encoder setFragmentSamplerState:b[i].sampler->state atIndex:first + i];
    }
}
void PushVertexUniformData(CommandBuffer *c, uint32_t slot, const void *data, uint32_t bytes)
{
    auto b = [c->device->metal newBufferWithBytes:data length:bytes options:MTLResourceStorageModeShared];
    [c->render.encoder setVertexBuffer:b offset:0 atIndex:slot];
}
void PushFragmentUniformData(CommandBuffer *c, uint32_t slot, const void *data, uint32_t bytes)
{
    auto b = [c->device->metal newBufferWithBytes:data length:bytes options:MTLResourceStorageModeShared];
    [c->render.encoder setFragmentBuffer:b offset:0 atIndex:slot];
}
void DrawPrimitives(RenderPass *p, uint32_t vertices, uint32_t instances, uint32_t first, uint32_t instance)
{
    [p->encoder drawPrimitives:p->primitive
                   vertexStart:first
                   vertexCount:vertices
                 instanceCount:instances
                  baseInstance:instance];
}
void DrawIndexedPrimitives(RenderPass *p, uint32_t indices, uint32_t instances, uint32_t first, int32_t base,
                           uint32_t instance)
{
    [p->encoder drawIndexedPrimitives:p->primitive
                           indexCount:indices
                            indexType:p->index_type
                          indexBuffer:p->indices
                    indexBufferOffset:p->offset + first * (p->index_type == MTLIndexTypeUInt16 ? 2 : 4)
                        instanceCount:instances
                           baseVertex:base
                         baseInstance:instance];
}
void EndRenderPass(RenderPass *p)
{
    [p->encoder endEncoding];
    p->encoder = nil;
}
CopyPass *BeginCopyPass(CommandBuffer *c)
{
    c->copy.encoder = [c->commands blitCommandEncoder];
    return &c->copy;
}
void EndCopyPass(CopyPass *p)
{
    [p->encoder endEncoding];
    p->encoder = nil;
}
void UploadToBuffer(CopyPass *p, const TransferBufferLocation *s, const BufferRegion *d, bool)
{
    [p->encoder copyFromBuffer:s->transfer_buffer->object
                  sourceOffset:s->offset
                      toBuffer:d->buffer->object
             destinationOffset:d->offset
                          size:d->size];
}
static NSUInteger Pitch(unsigned width, Pixel f)
{
    return (width + f.block - 1) / f.block * f.bytes;
}
void UploadToTexture(CopyPass *p, const TextureTransferInfo *s, const TextureRegion *d, bool)
{
    auto f = Describe(d->texture->format);
    auto pitch = Pitch(s->pixels_per_row ? s->pixels_per_row : d->w, f);
    NSUInteger rows = ((s->rows_per_layer ? s->rows_per_layer : d->h) + f.block - 1) / f.block;
    [p->encoder copyFromBuffer:s->transfer_buffer->object
                  sourceOffset:s->offset
             sourceBytesPerRow:pitch
           sourceBytesPerImage:pitch * rows
                    sourceSize:MTLSizeMake(d->w, d->h, d->d)
                     toTexture:d->texture->object
              destinationSlice:d->layer
              destinationLevel:d->mip_level
             destinationOrigin:MTLOriginMake(d->x, d->y, d->z)];
}
void DownloadFromTexture(CopyPass *p, const TextureRegion *s, const TextureTransferInfo *d)
{
    auto f = Describe(s->texture->format);
    auto pitch = Pitch(d->pixels_per_row ? d->pixels_per_row : s->w, f);
    NSUInteger rows = ((d->rows_per_layer ? d->rows_per_layer : s->h) + f.block - 1) / f.block;
    [p->encoder copyFromTexture:s->texture->object
                     sourceSlice:s->layer
                     sourceLevel:s->mip_level
                    sourceOrigin:MTLOriginMake(s->x, s->y, s->z)
                      sourceSize:MTLSizeMake(s->w, s->h, s->d)
                        toBuffer:d->transfer_buffer->object
               destinationOffset:d->offset
          destinationBytesPerRow:pitch
        destinationBytesPerImage:pitch * rows];
}
void CopyTextureToTexture(CopyPass *p, const TextureLocation *s, const TextureLocation *d, uint32_t w, uint32_t h,
                          uint32_t depth, bool)
{
    [p->encoder copyFromTexture:s->texture->object
                    sourceSlice:s->layer
                    sourceLevel:s->mip_level
                   sourceOrigin:MTLOriginMake(s->x, s->y, s->z)
                     sourceSize:MTLSizeMake(w, h, depth)
                      toTexture:d->texture->object
               destinationSlice:d->layer
               destinationLevel:d->mip_level
              destinationOrigin:MTLOriginMake(d->x, d->y, d->z)];
}
bool SubmitCommandBuffer(CommandBuffer *c)
{
    if (c->drawable)
        [c->commands presentDrawable:c->drawable];
    [c->commands commit];
    delete c;
    return true;
}
Fence *SubmitCommandBufferAndAcquireFence(CommandBuffer *c)
{
    auto f = new Fence{c->commands};
    SubmitCommandBuffer(c);
    return f;
}
bool CancelCommandBuffer(CommandBuffer *c)
{
    delete c;
    return true;
}
bool WaitForIdle(Device *d)
{
    auto cmd = [d->queue commandBuffer];
    [cmd commit];
    [cmd waitUntilCompleted];
    return cmd.status == MTLCommandBufferStatusCompleted;
}
bool WaitForFences(Device *, bool, Fence *const *fences, uint32_t n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        [fences[i]->commands waitUntilCompleted];
        if (fences[i]->commands.error)
            return false;
    }
    return true;
}
void ReleaseFence(Device *, Fence *f)
{
    delete f;
}
}
