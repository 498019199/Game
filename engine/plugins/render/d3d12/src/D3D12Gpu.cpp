#include "D3D12Gpu.h"
#include "D3D12Interop.h"
#include <common/ErrorHandling.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <memory>
#include <vector>
#include <string>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <algorithm>
#include <array>
#include <deque>
#include <cstring>
#include <unordered_map>

namespace RenderWorker::d3d12
{
using Microsoft::WRL::ComPtr;
using Resource = ComPtr<ID3D12Resource>;
static thread_local std::string error;
const char *LastError()
{
    return error.c_str();
}
static void Check(HRESULT hr, const char *operation)
{
    if (FAILED(hr))
    {
        error = std::string(operation) + " failed with HRESULT " + std::to_string(static_cast<unsigned>(hr));
        TMSG(error);
    }
}
struct Pixel
{
    DXGI_FORMAT format;
    unsigned bytes, block;
};
static Pixel Describe(TextureFormat f)
{
    switch (f)
    {
#define PIXEL_FORMAT(id, dxgi, bytes, block)                                                                    \
    case TEXTUREFORMAT_##id:                                                                                           \
        return {DXGI_FORMAT_##dxgi, bytes, block};
#include "PixelFormats.inc"
#undef PIXEL_FORMAT
    default:
        return {DXGI_FORMAT_UNKNOWN, 0, 1};
    }
}
static DXGI_FORMAT Attribute(VertexElementFormat f)
{
    switch (f)
    {
#define VERTEX_FORMAT(id, dxgi)                                                                                 \
    case VERTEXELEMENTFORMAT_##id:                                                                                     \
        return DXGI_FORMAT_##dxgi;
#include "VertexFormats.inc"
#undef VERTEX_FORMAT
    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}
struct Heap
{
    ComPtr<ID3D12DescriptorHeap> object;
    UINT stride{}, capacity{}, used{};
    D3D12_CPU_DESCRIPTOR_HANDLE Cpu(unsigned i) const
    {
        return {object->GetCPUDescriptorHandleForHeapStart().ptr + SIZE_T(i) * stride};
    }
    D3D12_GPU_DESCRIPTOR_HANDLE Gpu(unsigned i) const
    {
        return {object->GetGPUDescriptorHandleForHeapStart().ptr + UINT64(i) * stride};
    }
    void Init(ID3D12Device *d, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT size, bool visible = false)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{
            type, size, visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0};
        Check(d->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&object)), "CreateDescriptorHeap");
        capacity = size;
        stride = d->GetDescriptorHandleIncrementSize(type);
    }
};
struct Texture
{
    Resource object;
    TextureCreateInfo info{};
    struct Device *owner{};
    UINT editor_slot = UINT_MAX;
};
struct Submission
{
    uint64_t fence;
    std::vector<ComPtr<IUnknown>> retained;
    std::vector<UINT> retired_editor_slots;
};
struct Device
{
    ComPtr<ID3D12Device> gpu;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<IDXGISwapChain3> swap;
    ComPtr<ID3D12Fence> timeline;
    ComPtr<ID3D12RootSignature> root;
    HANDLE event{};
    uint64_t next{};
    unsigned width{}, height{}, sync{};
    DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
    std::array<Texture, 2> backbuffers;
    Heap ui;
    std::vector<UINT> free_ui;
    std::deque<Submission> in_flight;
    std::array<uint64_t, 2> backbuffer_fences{};
    std::vector<UINT> retired_editor_slots;
    std::vector<ComPtr<IUnknown>> retired_editor_resources;
    ~Device()
    {
        if (event)
            CloseHandle(event);
    }
};
struct Buffer
{
    Resource object;
    UINT size;
};
struct TransferBuffer
{
    std::vector<unsigned char> bytes;
    Resource readback;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
    unsigned rows{}, depth{}, dest_offset{}, row_pitch{}, layer_pitch{}, row_bytes{};
};
struct Shader
{
    std::vector<uint8_t> code;
};
struct Sampler
{
    D3D12_SAMPLER_DESC desc;
};
struct GraphicsPipeline
{
    ComPtr<ID3D12PipelineState> state;
    D3D12_PRIMITIVE_TOPOLOGY topology;
    std::array<UINT, 16> strides{};
};
struct StageBindings
{
    std::array<D3D12_CONSTANT_BUFFER_VIEW_DESC, 14> cb{};
    std::array<TextureSamplerBinding, 16> images{};
};
struct CommandBuffer;
struct RenderPass
{
    CommandBuffer *command{};
    GraphicsPipeline *pipeline{};
    std::array<StageBindings, 2> stages{};
    std::vector<ID3D12Resource *> targets;
};
struct CopyPass
{
    CommandBuffer *command;
};
struct CommandBuffer
{
    Device *d{};
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList> list;
    std::vector<ComPtr<IUnknown>> retained;
    std::unordered_map<ID3D12Resource *, D3D12_RESOURCE_STATES> states;
    Heap views, samplers;
    Texture *back{};
    RenderPass render;
    CopyPass copy;
};
struct Fence
{
    Device *d;
    uint64_t value;
};
static void Wait(Device *d, uint64_t value)
{
    if (d->timeline->GetCompletedValue() < value)
    {
        Check(d->timeline->SetEventOnCompletion(value, d->event), "SetEventOnCompletion");
        if (WaitForSingleObject(d->event, INFINITE) != WAIT_OBJECT_0)
            TMSG("D3D12 GPU wait failed");
    }
}
static void Reclaim(Device *d)
{
    uint64_t completed = d->timeline->GetCompletedValue();
    while (!d->in_flight.empty() && d->in_flight.front().fence <= completed)
    {
        auto &slots = d->in_flight.front().retired_editor_slots;
        d->free_ui.insert(d->free_ui.end(), slots.begin(), slots.end());
        d->in_flight.pop_front();
    }
}
static uint64_t Signal(Device *d)
{
    auto value = ++d->next;
    Check(d->queue->Signal(d->timeline.Get(), value), "Signal");
    return value;
}
static void CreateRoot(Device *d)
{
    std::array<D3D12_DESCRIPTOR_RANGE, 6> ranges{};
    std::array<D3D12_ROOT_PARAMETER, 6> params{};
    for (unsigned stage = 0; stage < 2; ++stage)
    {
        for (unsigned type = 0; type < 3; ++type)
        {
            unsigned i = stage * 3 + type;
            ranges[i] = {type == 0   ? D3D12_DESCRIPTOR_RANGE_TYPE_CBV
                         : type == 1 ? D3D12_DESCRIPTOR_RANGE_TYPE_SRV
                                     : D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
                         type == 0 ? 14u : 16u, 0, stage * 2 + (type == 0 ? 1u : 0u), 0};
            params[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[i].DescriptorTable = {1, &ranges[i]};
            params[i].ShaderVisibility = stage ? D3D12_SHADER_VISIBILITY_PIXEL : D3D12_SHADER_VISIBILITY_VERTEX;
        }
    }
    D3D12_ROOT_SIGNATURE_DESC desc{UINT(params.size()), params.data(), 0, nullptr,
                                   D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};
    ComPtr<ID3DBlob> blob, messages;
    Check(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &messages), "SerializeRootSignature");
    Check(d->gpu->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&d->root)),
          "CreateRootSignature");
}
static void GetBackbuffers(Device *d)
{
    for (UINT i = 0; i < d->backbuffers.size(); ++i)
    {
        auto &t = d->backbuffers[i];
        Check(d->swap->GetBuffer(i, IID_PPV_ARGS(&t.object)), "GetBuffer");
        t.info.format = TEXTUREFORMAT_B8G8R8A8_UNORM;
        t.info.width = d->width;
        t.info.height = d->height;
        t.info.num_levels = 1;
    }
}
Device *CreateDevice(void *handle, unsigned sync, bool debug)
{
    try
    {
        if (debug)
        {
            ComPtr<ID3D12Debug> layer;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&layer))))
                layer->EnableDebugLayer();
        }
        auto d = std::make_unique<Device>();
        ComPtr<IDXGIFactory4> factory;
        Check(CreateDXGIFactory1(IID_PPV_ARGS(&factory)), "CreateDXGIFactory");
        Check(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d->gpu)), "D3D12CreateDevice");
        D3D12_COMMAND_QUEUE_DESC queue{};
        Check(d->gpu->CreateCommandQueue(&queue, IID_PPV_ARGS(&d->queue)), "CreateCommandQueue");
        Check(d->gpu->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d->timeline)), "CreateFence");
        d->event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!d->event)
            TMSG("CreateEvent failed");
        RECT r{};
        auto hwnd = static_cast<HWND>(handle);
        GetClientRect(hwnd, &r);
        d->width = std::max<LONG>(1, r.right);
        d->height = std::max<LONG>(1, r.bottom);
        d->sync = std::min(sync, 4u);
        DXGI_SWAP_CHAIN_DESC1 sd{};
        sd.Width = d->width;
        sd.Height = d->height;
        sd.Format = d->format;
        sd.SampleDesc.Count = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 2;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        ComPtr<IDXGISwapChain1> swap;
        Check(factory->CreateSwapChainForHwnd(d->queue.Get(), hwnd, &sd, nullptr, nullptr, &swap),
              "CreateSwapChainForHwnd");
        Check(swap.As(&d->swap), "Query swapchain");
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
        GetBackbuffers(d.get());
        CreateRoot(d.get());
        d->ui.Init(d->gpu.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8192, true);
        return d.release();
    }
    catch (std::exception const &e)
    {
        error = e.what();
        return nullptr;
    }
}
void DestroyDevice(Device *d)
{
    WaitForIdle(d);
    delete d;
}
bool WaitForIdle(Device *d)
{
    Wait(d, Signal(d));
    Reclaim(d);
    return true;
}
ShaderFormat GetShaderFormats(Device *)
{
    return SHADERFORMAT_DXIL;
}
void *NativeDevice(Device *d)
{
    return d->gpu.Get();
}
void *TextureHandle(Texture *t)
{
    return t ? t->object.Get() : nullptr;
}
void *NativeCommands(CommandBuffer *c)
{
    return c ? c->list.Get() : nullptr;
}
void *NativeEncoder(RenderPass *p)
{
    return NativeCommands(p->command);
}
ID3D12CommandQueue *NativeQueue(Device *d)
{
    return d->queue.Get();
}
ID3D12DescriptorHeap *EditorHeap(Device *d)
{
    return d->ui.object.Get();
}
void AllocateEditorView(Device *d, D3D12_CPU_DESCRIPTOR_HANDLE &cpu, D3D12_GPU_DESCRIPTOR_HANDLE &gpu)
{
    unsigned slot;
    if (!d->free_ui.empty())
    {
        slot = d->free_ui.back();
        d->free_ui.pop_back();
    }
    else
    {
        if (d->ui.used == d->ui.capacity)
            TMSG("Editor texture descriptors exhausted");
        slot = d->ui.used++;
    }
    cpu = d->ui.Cpu(slot);
    gpu = d->ui.Gpu(slot);
}
void FreeEditorView(Device *d, D3D12_CPU_DESCRIPTOR_HANDLE cpu)
{
    d->retired_editor_slots.push_back(UINT((cpu.ptr - d->ui.Cpu(0).ptr) / d->ui.stride));
}
static Resource NewResource(Device *d, D3D12_HEAP_TYPE heap, D3D12_RESOURCE_DESC const &desc,
                            D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE *clear = nullptr)
{
    D3D12_HEAP_PROPERTIES props{};
    props.Type = heap;
    props.CreationNodeMask = props.VisibleNodeMask = 1;
    Resource r;
    Check(d->gpu->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, state, clear, IID_PPV_ARGS(&r)),
          "CreateCommittedResource");
    return r;
}
static Resource Memory(Device *d, uint64_t bytes, D3D12_HEAP_TYPE heap)
{
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = bytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return NewResource(d, heap, desc,
                       heap == D3D12_HEAP_TYPE_UPLOAD     ? D3D12_RESOURCE_STATE_GENERIC_READ
                       : heap == D3D12_HEAP_TYPE_READBACK ? D3D12_RESOURCE_STATE_COPY_DEST
                                                          : D3D12_RESOURCE_STATE_COMMON);
}
static void Transition(CommandBuffer *c, ID3D12Resource *resource, D3D12_RESOURCE_STATES after)
{
    auto [it, added] = c->states.try_emplace(resource, D3D12_RESOURCE_STATE_COMMON);
    if (added)
        c->retained.emplace_back(resource);
    if (it->second == after)
        return;
    D3D12_RESOURCE_BARRIER b{};
    b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    b.Transition = {resource, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, it->second, after};
    c->list->ResourceBarrier(1, &b);
    it->second = after;
}
static DXGI_FORMAT StorageFormat(TextureFormat f)
{
    switch (f)
    {
    case TEXTUREFORMAT_D16_UNORM:
        return DXGI_FORMAT_R16_TYPELESS;
    case TEXTUREFORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24G8_TYPELESS;
    case TEXTUREFORMAT_D32_FLOAT:
        return DXGI_FORMAT_R32_TYPELESS;
    case TEXTUREFORMAT_D32_FLOAT_S8_UINT:
        return DXGI_FORMAT_R32G8X24_TYPELESS;
    default:
        return Describe(f).format;
    }
}
static void WriteSrv(Device *d, Texture *t, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Format = t ? Describe(t->info.format).format : DXGI_FORMAT_R8G8B8A8_UNORM;
    if (t)
        switch (t->info.format)
        {
        case TEXTUREFORMAT_D16_UNORM:
            srv.Format = DXGI_FORMAT_R16_UNORM;
            break;
        case TEXTUREFORMAT_D24_UNORM_S8_UINT:
            srv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;
        case TEXTUREFORMAT_D32_FLOAT:
            srv.Format = DXGI_FORMAT_R32_FLOAT;
            break;
        case TEXTUREFORMAT_D32_FLOAT_S8_UINT:
            srv.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            break;
        default:
            break;
        }
    auto levels = t ? t->info.num_levels : 1;
    switch (t ? t->info.type : TEXTURETYPE_2D)
    {
    case TEXTURETYPE_3D:
        srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srv.Texture3D.MipLevels = levels;
        break;
    case TEXTURETYPE_2D_ARRAY:
        srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srv.Texture2DArray.MipLevels = levels;
        srv.Texture2DArray.ArraySize = t->info.layer_count_or_depth;
        break;
    case TEXTURETYPE_CUBE:
        srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srv.TextureCube.MipLevels = levels;
        break;
    case TEXTURETYPE_CUBE_ARRAY:
        srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        srv.TextureCubeArray.MipLevels = levels;
        srv.TextureCubeArray.NumCubes = t->info.layer_count_or_depth / 6;
        break;
    default:
        srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MipLevels = levels;
        break;
    }
    d->gpu->CreateShaderResourceView(t ? t->object.Get() : nullptr, &srv, handle);
}
void *EditorTexture(Texture *t)
{
    if (!t)
        return nullptr;
    if (t->editor_slot == UINT_MAX)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu;
        D3D12_GPU_DESCRIPTOR_HANDLE gpu;
        AllocateEditorView(t->owner, cpu, gpu);
        WriteSrv(t->owner, t, cpu);
        t->editor_slot = UINT((cpu.ptr - t->owner->ui.Cpu(0).ptr) / t->owner->ui.stride);
    }
    return reinterpret_cast<void *>(t->owner->ui.Gpu(t->editor_slot).ptr);
}
bool TextureSupportsFormat(Device *d, TextureFormat f, TextureType, TextureUsageFlags usage)
{
    D3D12_FEATURE_DATA_FORMAT_SUPPORT caps{Describe(f).format};
    if (FAILED(d->gpu->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &caps, sizeof caps)))
        return false;
    return (!(usage & TEXTUREUSAGE_COLOR_TARGET) || (caps.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)) &&
           (!(usage & TEXTUREUSAGE_DEPTH_STENCIL_TARGET) || (caps.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL));
}
Texture *CreateTexture(Device *d, const TextureCreateInfo *i)
{
    D3D12_RESOURCE_DESC td{};
    td.Dimension = i->type == TEXTURETYPE_3D ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    td.Width = i->width;
    td.Height = i->height;
    td.DepthOrArraySize = UINT16(i->layer_count_or_depth);
    td.MipLevels = UINT16(i->num_levels);
    td.SampleDesc.Count = 1;
    td.Format = StorageFormat(i->format);
    D3D12_CLEAR_VALUE clear{};
    clear.Format = Describe(i->format).format;
    if (i->usage & TEXTUREUSAGE_COLOR_TARGET)
    {
        td.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        std::copy_n(i->clear_color, 4, clear.Color);
    }
    if (i->usage & TEXTUREUSAGE_DEPTH_STENCIL_TARGET)
    {
        td.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        clear.DepthStencil.Depth = 1;
    }
    auto r = NewResource(d, D3D12_HEAP_TYPE_DEFAULT, td, D3D12_RESOURCE_STATE_COMMON, td.Flags ? &clear : nullptr);
    return new Texture{std::move(r), *i, d};
}
Buffer *CreateBuffer(Device *d, const BufferCreateInfo *i)
{
    return new Buffer{Memory(d, i->size, D3D12_HEAP_TYPE_DEFAULT), i->size};
}
TransferBuffer *CreateTransferBuffer(Device *, const TransferBufferCreateInfo *i)
{
    return new TransferBuffer{std::vector<unsigned char>(i->size)};
}
void ReleaseTexture(Device *, Texture *t)
{
    if (t && t->editor_slot != UINT_MAX)
    {
        FreeEditorView(t->owner, t->owner->ui.Cpu(t->editor_slot));
        t->owner->retired_editor_resources.emplace_back(t->object.Get());
    }
    delete t;
}
void ReleaseBuffer(Device *, Buffer *b)
{
    delete b;
}
void ReleaseTransferBuffer(Device *, TransferBuffer *t)
{
    delete t;
}
void ReleaseShader(Device *, Shader *s)
{
    delete s;
}
void ReleaseSampler(Device *, Sampler *s)
{
    delete s;
}
void ReleaseGraphicsPipeline(Device *, GraphicsPipeline *p)
{
    delete p;
}
Shader *CreateShader(Device *, const ShaderCreateInfo *i)
{
    return new Shader{{i->code, i->code + i->code_size}};
}
Sampler *CreateSampler(Device *, const SamplerCreateInfo *i)
{
    constexpr D3D12_TEXTURE_ADDRESS_MODE modes[] = {D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP};
    auto s = new Sampler{};
    auto &v = s->desc;
    v.AddressU = modes[i->address_mode_u];
    v.AddressV = modes[i->address_mode_v];
    v.AddressW = modes[i->address_mode_w];
    v.Filter =
        (D3D12_FILTER)((i->enable_anisotropy ? 0x55 : ((i->min_filter << 4) | (i->mag_filter << 2) | i->mipmap_mode)) |
                       (i->enable_compare ? 0x80 : 0));
    v.MipLODBias = i->mip_lod_bias;
    v.MaxAnisotropy = std::max(1u, UINT(i->max_anisotropy));
    v.ComparisonFunc = i->enable_compare ? (D3D12_COMPARISON_FUNC)i->compare_op : D3D12_COMPARISON_FUNC_ALWAYS;
    v.MinLOD = i->min_lod;
    v.MaxLOD = i->max_lod;
    return s;
}
static D3D12_BLEND Blend(BlendFactor f)
{
    constexpr D3D12_BLEND b[] = {D3D12_BLEND_ZERO,
                                 D3D12_BLEND_ZERO,
                                 D3D12_BLEND_ONE,
                                 D3D12_BLEND_SRC_COLOR,
                                 D3D12_BLEND_INV_SRC_COLOR,
                                 D3D12_BLEND_DEST_COLOR,
                                 D3D12_BLEND_INV_DEST_COLOR,
                                 D3D12_BLEND_SRC_ALPHA,
                                 D3D12_BLEND_INV_SRC_ALPHA,
                                 D3D12_BLEND_DEST_ALPHA,
                                 D3D12_BLEND_INV_DEST_ALPHA,
                                 D3D12_BLEND_BLEND_FACTOR,
                                 D3D12_BLEND_INV_BLEND_FACTOR,
                                 D3D12_BLEND_SRC_ALPHA_SAT};
    return b[f];
}
GraphicsPipeline *CreateGraphicsPipeline(Device *d, const GraphicsPipelineCreateInfo *i)
{
    auto p = std::make_unique<GraphicsPipeline>();
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
    desc.pRootSignature = d->root.Get();
    desc.VS = {i->vertex_shader->code.data(), i->vertex_shader->code.size()};
    desc.PS = {i->fragment_shader->code.data(), i->fragment_shader->code.size()};
    std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    for (UINT n = 0; n < i->vertex_input_state.num_vertex_buffers; ++n)
    {
        auto const &b = i->vertex_input_state.vertex_buffer_descriptions[n];
        p->strides.at(b.slot) = b.pitch;
    }
    for (UINT n = 0; n < i->vertex_input_state.num_vertex_attributes; ++n)
    {
        auto const &a = i->vertex_input_state.vertex_attributes[n];
        bool instance =
            i->vertex_input_state.vertex_buffer_descriptions[a.buffer_slot].input_rate == VERTEXINPUTRATE_INSTANCE;
        elements.push_back(
            {"TEXCOORD", a.location, Attribute(a.format), a.buffer_slot, a.offset,
             instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
             instance ? 1u : 0u});
    }
    desc.InputLayout = {elements.data(), UINT(elements.size())};
    desc.SampleMask = UINT_MAX;
    desc.SampleDesc.Count = 1;
    static constexpr D3D12_PRIMITIVE_TOPOLOGY topo[] = {
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, D3D_PRIMITIVE_TOPOLOGY_LINELIST,
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, D3D_PRIMITIVE_TOPOLOGY_POINTLIST};
    p->topology = topo[i->primitive_type];
    desc.PrimitiveTopologyType = i->primitive_type == PRIMITIVETYPE_POINTLIST  ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT
                                 : i->primitive_type >= PRIMITIVETYPE_LINELIST ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
                                                                               : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    desc.RasterizerState.FillMode =
        i->rasterizer_state.fill_mode == FILLMODE_LINE ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode = (D3D12_CULL_MODE)(i->rasterizer_state.cull_mode + 1);
    desc.RasterizerState.FrontCounterClockwise = i->rasterizer_state.front_face == FRONTFACE_COUNTER_CLOCKWISE;
    desc.RasterizerState.DepthClipEnable = i->rasterizer_state.enable_depth_clip;
    desc.BlendState.IndependentBlendEnable = TRUE;
    desc.NumRenderTargets = i->target_info.num_color_targets;
    for (UINT n = 0; n < desc.NumRenderTargets; ++n)
    {
        auto const &t = i->target_info.color_target_descriptions[n];
        auto const &b = t.blend_state;
        auto &dest = desc.BlendState.RenderTarget[n];
        desc.RTVFormats[n] = Describe(t.format).format;
        dest.BlendEnable = b.enable_blend;
        dest.SrcBlend = Blend(b.src_color_blendfactor);
        dest.DestBlend = Blend(b.dst_color_blendfactor);
        dest.BlendOp = (D3D12_BLEND_OP)std::max(1, int(b.color_blend_op));
        dest.SrcBlendAlpha = Blend(b.src_alpha_blendfactor);
        dest.DestBlendAlpha = Blend(b.dst_alpha_blendfactor);
        dest.BlendOpAlpha = (D3D12_BLEND_OP)std::max(1, int(b.alpha_blend_op));
        dest.RenderTargetWriteMask = b.enable_color_write_mask ? b.color_write_mask : D3D12_COLOR_WRITE_ENABLE_ALL;
    }
    desc.DSVFormat = i->target_info.has_depth_stencil_target ? Describe(i->target_info.depth_stencil_format).format
                                                             : DXGI_FORMAT_UNKNOWN;
    desc.DepthStencilState.DepthEnable =
        i->target_info.has_depth_stencil_target && i->depth_stencil_state.enable_depth_test;
    desc.DepthStencilState.DepthWriteMask =
        i->depth_stencil_state.enable_depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    desc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)std::max(1, int(i->depth_stencil_state.compare_op));
    desc.DepthStencilState.FrontFace = desc.DepthStencilState.BackFace = {
        D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS};
    if (FAILED(d->gpu->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&p->state))))
    {
        error = "Cannot create D3D12 graphics pipeline";
        return nullptr;
    }
    return p.release();
}
CommandBuffer *AcquireCommandBuffer(Device *d)
{
    Reclaim(d);
    auto c = std::make_unique<CommandBuffer>();
    c->d = d;
    c->render.command = c.get();
    c->copy.command = c.get();
    Check(d->gpu->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&c->allocator)),
          "CreateCommandAllocator");
    Check(d->gpu->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, c->allocator.Get(), nullptr,
                                    IID_PPV_ARGS(&c->list)),
          "CreateCommandList");
    return c.release();
}
TextureFormat GetSwapchainTextureFormat(Device *, void *)
{
    return TEXTUREFORMAT_B8G8R8A8_UNORM;
}
bool WaitAndAcquireSwapchainTexture(CommandBuffer *c, void *handle, Texture **t, uint32_t *w, uint32_t *h)
{
    auto d = c->d;
    RECT r{};
    GetClientRect(static_cast<HWND>(handle), &r);
    *t = nullptr;
    if (r.right <= 0 || r.bottom <= 0 || IsIconic(static_cast<HWND>(handle)))
        return true;
    if (d->width != UINT(r.right) || d->height != UINT(r.bottom))
    {
        WaitForIdle(d);
        for (auto &b : d->backbuffers)
            b.object.Reset();
        Check(d->swap->ResizeBuffers(2, r.right, r.bottom, d->format, 0), "ResizeBuffers");
        d->width = r.right;
        d->height = r.bottom;
        GetBackbuffers(d);
    }
    auto index = d->swap->GetCurrentBackBufferIndex();
    Wait(d, d->backbuffer_fences[index]);
    c->back = &d->backbuffers[index];
    *t = c->back;
    *w = d->width;
    *h = d->height;
    return true;
}
RenderPass *BeginRenderPass(CommandBuffer *c, const ColorTargetInfo *colors, uint32_t count,
                            const DepthStencilTargetInfo *depth)
{
    auto &p = c->render;
    p.targets.clear();
    auto d = c->d;
    Heap rtv, dsv;
    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 8> rt{};
    D3D12_CPU_DESCRIPTOR_HANDLE ds{};
    if (count)
        rtv.Init(d->gpu.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, count);
    for (UINT i = 0; i < count; ++i)
    {
        auto &a = colors[i];
        auto t = a.texture;
        Transition(c, t->object.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        p.targets.push_back(t->object.Get());
        D3D12_RENDER_TARGET_VIEW_DESC view{};
        view.Format = Describe(t->info.format).format;
        if (t->info.type == TEXTURETYPE_3D)
        {
            view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            view.Texture3D = {a.mip_level, a.layer_or_depth_plane, 1};
        }
        else if (t->info.layer_count_or_depth > 1)
        {
            view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray = {a.mip_level, a.layer_or_depth_plane, 1, 0};
        }
        else
        {
            view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            view.Texture2D.MipSlice = a.mip_level;
        }
        rt[i] = rtv.Cpu(i);
        d->gpu->CreateRenderTargetView(t->object.Get(), &view, rt[i]);
        if (a.load_op == LOADOP_CLEAR)
            c->list->ClearRenderTargetView(rt[i], &a.clear_color.r, 0, nullptr);
    }
    if (depth)
    {
        auto t = depth->texture;
        Transition(c, t->object.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
        p.targets.push_back(t->object.Get());
        dsv.Init(d->gpu.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
        ds = dsv.Cpu(0);
        D3D12_DEPTH_STENCIL_VIEW_DESC view{};
        view.Format = Describe(t->info.format).format;
        if (t->info.layer_count_or_depth > 1)
        {
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray = {depth->mip_level, depth->layer, 1};
        }
        else
        {
            view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            view.Texture2D.MipSlice = depth->mip_level;
        }
        d->gpu->CreateDepthStencilView(t->object.Get(), &view, ds);
        int flags = (depth->load_op == LOADOP_CLEAR ? D3D12_CLEAR_FLAG_DEPTH : 0);
        if (depth->stencil_load_op == LOADOP_CLEAR &&
            (t->info.format == TEXTUREFORMAT_D24_UNORM_S8_UINT || t->info.format == TEXTUREFORMAT_D32_FLOAT_S8_UINT))
            flags |= D3D12_CLEAR_FLAG_STENCIL;
        if (flags)
            c->list->ClearDepthStencilView(ds, (D3D12_CLEAR_FLAGS)flags, depth->clear_depth, depth->clear_stencil, 0,
                                           nullptr);
    }
    c->list->OMSetRenderTargets(count, rt.data(), FALSE, depth ? &ds : nullptr);
    return &p;
}
void EndRenderPass(RenderPass *p)
{
    auto c = p->command;
    c->list->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
    for (auto t : p->targets)
        Transition(c, t, D3D12_RESOURCE_STATE_COMMON);
    p->targets.clear();
}
void BindGraphicsPipeline(RenderPass *p, GraphicsPipeline *pipe)
{
    auto c = p->command;
    p->stages = {};
    p->pipeline = pipe;
    c->retained.emplace_back(pipe->state.Get());
    c->list->SetGraphicsRootSignature(c->d->root.Get());
    c->list->SetPipelineState(pipe->state.Get());
    c->list->IASetPrimitiveTopology(pipe->topology);
}
void SetViewport(RenderPass *p, const Viewport *v)
{
    D3D12_VIEWPORT vp{v->x, v->y, v->w, v->h, v->min_depth, v->max_depth};
    p->command->list->RSSetViewports(1, &vp);
}
void SetScissor(RenderPass *p, const Rect *r)
{
    D3D12_RECT rect{r->x, r->y, r->x + r->w, r->y + r->h};
    p->command->list->RSSetScissorRects(1, &rect);
}
void BindVertexBuffers(RenderPass *p, uint32_t first, const BufferBinding *b, uint32_t n)
{
    std::array<D3D12_VERTEX_BUFFER_VIEW, 16> views{};
    for (UINT i = 0; i < n; ++i)
    {
        auto buf = b[i].buffer;
        Transition(p->command, buf->object.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        views.at(i) = {buf->object->GetGPUVirtualAddress() + b[i].offset, buf->size - b[i].offset,
                       p->pipeline->strides.at(first + i)};
    }
    p->command->list->IASetVertexBuffers(first, n, views.data());
}
void BindIndexBuffer(RenderPass *p, const BufferBinding *b, IndexElementSize fmt)
{
    auto buf = b->buffer;
    Transition(p->command, buf->object.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
    D3D12_INDEX_BUFFER_VIEW view{buf->object->GetGPUVirtualAddress() + b->offset, buf->size - b->offset,
                                 fmt == INDEXELEMENTSIZE_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT};
    p->command->list->IASetIndexBuffer(&view);
}
static void UploadConstants(CommandBuffer *c, UINT stage, UINT slot, const void *data, UINT length)
{
    unsigned size = (length + 255) & ~255u;
    auto b = Memory(c->d, size, D3D12_HEAP_TYPE_UPLOAD);
    void *mapped = nullptr;
    D3D12_RANGE empty{};
    Check(b->Map(0, &empty, &mapped), "Map constants");
    std::memcpy(mapped, data, length);
    b->Unmap(0, nullptr);
    c->render.stages[stage].cb.at(slot) = {b->GetGPUVirtualAddress(), size};
    c->retained.emplace_back(b.Get());
}
void PushVertexUniformData(CommandBuffer *c, uint32_t slot, const void *b, uint32_t n)
{
    UploadConstants(c, 0, slot, b, n);
}
void PushFragmentUniformData(CommandBuffer *c, uint32_t slot, const void *b, uint32_t n)
{
    UploadConstants(c, 1, slot, b, n);
}
static void SetImages(RenderPass *p, UINT stage, UINT slot, const TextureSamplerBinding *b, UINT n)
{
    auto &images = p->stages[stage].images;
    if (slot + n > images.size())
        TMSG("Too many shader samplers");
    std::copy_n(b, n, images.begin() + slot);
}
void BindVertexSamplers(RenderPass *p, uint32_t first, const TextureSamplerBinding *b, uint32_t n)
{
    SetImages(p, 0, first, b, n);
}
void BindFragmentSamplers(RenderPass *p, uint32_t first, const TextureSamplerBinding *b, uint32_t n)
{
    SetImages(p, 1, first, b, n);
}
static void Reserve(CommandBuffer *c, Heap &heap, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT n, UINT capacity)
{
    if (!heap.object || heap.used + n > heap.capacity)
    {
        if (heap.object)
            c->retained.emplace_back(heap.object.Get());
        heap = Heap{};
        heap.Init(c->d->gpu.Get(), type, capacity, true);
    }
}
static void BindTables(RenderPass *p)
{
    auto c = p->command;
    auto d = c->d;
    Reserve(c, c->views, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 60, 3072);
    Reserve(c, c->samplers, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 32, 1024);
    ID3D12DescriptorHeap *heaps[] = {c->views.object.Get(), c->samplers.object.Get()};
    c->list->SetDescriptorHeaps(2, heaps);
    D3D12_SAMPLER_DESC fallback{};
    fallback.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    fallback.AddressU = fallback.AddressV = fallback.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    fallback.MaxAnisotropy = 1;
    fallback.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    fallback.MaxLOD = D3D12_FLOAT32_MAX;
    for (unsigned i = 0; i < 2; ++i)
    {
        auto &stage = p->stages[i];
        UINT cb = c->views.used, srv = cb + 14, sampler = c->samplers.used;
        for (UINT j = 0; j < 14; ++j)
            d->gpu->CreateConstantBufferView(stage.cb[j].BufferLocation ? &stage.cb[j] : nullptr, c->views.Cpu(cb + j));
        for (UINT j = 0; j < 16; ++j)
        {
            auto const &b = stage.images[j];
            if (b.texture)
                Transition(c, b.texture->object.Get(),
                           D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            WriteSrv(d, b.texture, c->views.Cpu(srv + j));
            d->gpu->CreateSampler(b.sampler ? &b.sampler->desc : &fallback, c->samplers.Cpu(sampler + j));
        }
        c->list->SetGraphicsRootDescriptorTable(i * 3, c->views.Gpu(cb));
        c->list->SetGraphicsRootDescriptorTable(i * 3 + 1, c->views.Gpu(srv));
        c->list->SetGraphicsRootDescriptorTable(i * 3 + 2, c->samplers.Gpu(sampler));
        c->views.used += 30;
        c->samplers.used += 16;
    }
}
void DrawPrimitives(RenderPass *p, uint32_t n, uint32_t instances, uint32_t first, uint32_t base)
{
    BindTables(p);
    p->command->list->DrawInstanced(n, instances, first, base);
}
void DrawIndexedPrimitives(RenderPass *p, uint32_t n, uint32_t instances, uint32_t first, int32_t offset, uint32_t base)
{
    BindTables(p);
    p->command->list->DrawIndexedInstanced(n, instances, first, offset, base);
}
CopyPass *BeginCopyPass(CommandBuffer *c)
{
    return &c->copy;
}
void EndCopyPass(CopyPass *)
{
}
void *MapTransferBuffer(Device *, TransferBuffer *t, bool)
{
    if (t->readback)
    {
        uint8_t *ptr = nullptr;
        Check(t->readback->Map(0, nullptr, reinterpret_cast<void **>(&ptr)), "Map readback");
        for (UINT z = 0; z < t->depth; ++z)
            for (UINT y = 0; y < t->rows; ++y)
                std::memcpy(t->bytes.data() + t->dest_offset + z * t->layer_pitch + y * t->row_pitch,
                            ptr + (z * t->rows + y) * t->footprint.Footprint.RowPitch, t->row_bytes);
        D3D12_RANGE empty{};
        t->readback->Unmap(0, &empty);
        t->readback.Reset();
    }
    return t->bytes.data();
}
void UnmapTransferBuffer(Device *, TransferBuffer *)
{
}
void UploadToBuffer(CopyPass *p, const TransferBufferLocation *s, const BufferRegion *dest, bool)
{
    auto c = p->command;
    auto u = Memory(c->d, dest->size, D3D12_HEAP_TYPE_UPLOAD);
    void *data = nullptr;
    D3D12_RANGE empty{};
    Check(u->Map(0, &empty, &data), "Map transfer");
    std::memcpy(data, s->transfer_buffer->bytes.data() + s->offset, dest->size);
    u->Unmap(0, nullptr);
    Transition(c, dest->buffer->object.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
    c->list->CopyBufferRegion(dest->buffer->object.Get(), dest->offset, u.Get(), 0, dest->size);
    c->retained.emplace_back(u.Get());
}
static D3D12_TEXTURE_COPY_LOCATION Subresource(Texture *t, UINT layer, UINT mip)
{
    D3D12_TEXTURE_COPY_LOCATION loc{};
    loc.pResource = t->object.Get();
    loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    loc.SubresourceIndex = layer * t->info.num_levels + mip;
    return loc;
}
struct Footprint
{
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT placed{};
    UINT rows, row_bytes, input_pitch, input_layer;
    UINT64 size;
    Footprint(const TextureRegion &region, const TextureTransferInfo &transfer)
    {
        auto f = Describe(region.texture->info.format);
        auto blocks = [&](UINT pixels) { return (pixels + f.block - 1) / f.block; };
        row_bytes = blocks(region.w) * f.bytes;
        rows = blocks(region.h);
        auto pitch = (row_bytes + 255u) & ~255u;
        input_pitch = blocks(transfer.pixels_per_row ? transfer.pixels_per_row : region.w) * f.bytes;
        input_layer = input_pitch * blocks(transfer.rows_per_layer ? transfer.rows_per_layer : region.h);
        placed.Footprint = {f.format, region.w, region.h, region.d, pitch};
        size = UINT64(pitch) * rows * region.d;
        if (!region.d || !rows ||
            UINT64(transfer.offset) + UINT64(input_layer) * (region.d - 1) + UINT64(input_pitch) * (rows - 1) +
                    row_bytes >
                transfer.transfer_buffer->bytes.size())
            TMSG("Texture transfer exceeds buffer size");
    }
};
void UploadToTexture(CopyPass *p, const TextureTransferInfo *s, const TextureRegion *dest, bool)
{
    auto c = p->command;
    Footprint fp(*dest, *s);
    auto u = Memory(c->d, fp.size, D3D12_HEAP_TYPE_UPLOAD);
    uint8_t *data = nullptr;
    D3D12_RANGE empty{};
    Check(u->Map(0, &empty, reinterpret_cast<void **>(&data)), "Map texture upload");
    for (UINT z = 0; z < dest->d; ++z)
        for (UINT y = 0; y < fp.rows; ++y)
            std::memcpy(data + (z * fp.rows + y) * fp.placed.Footprint.RowPitch,
                        s->transfer_buffer->bytes.data() + s->offset + z * fp.input_layer + y * fp.input_pitch,
                        fp.row_bytes);
    u->Unmap(0, nullptr);
    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = u.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint = fp.placed;
    auto dst = Subresource(dest->texture, dest->layer, dest->mip_level);
    Transition(c, dst.pResource, D3D12_RESOURCE_STATE_COPY_DEST);
    c->list->CopyTextureRegion(&dst, dest->x, dest->y, dest->z, &src, nullptr);
    c->retained.emplace_back(u.Get());
}
void DownloadFromTexture(CopyPass *p, const TextureRegion *s, const TextureTransferInfo *dest)
{
    auto c = p->command;
    Footprint fp(*s, *dest);
    auto t = dest->transfer_buffer;
    t->readback = Memory(c->d, fp.size, D3D12_HEAP_TYPE_READBACK);
    t->footprint = fp.placed;
    t->rows = fp.rows;
    t->depth = s->d;
    t->dest_offset = dest->offset;
    t->row_pitch = fp.input_pitch;
    t->layer_pitch = fp.input_layer;
    t->row_bytes = fp.row_bytes;
    auto src = Subresource(s->texture, s->layer, s->mip_level);
    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = t->readback.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint = fp.placed;
    D3D12_BOX box{s->x, s->y, s->z, s->x + s->w, s->y + s->h, s->z + s->d};
    Transition(c, src.pResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
    c->list->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);
    c->retained.emplace_back(t->readback.Get());
}
void CopyTextureToTexture(CopyPass *p, const TextureLocation *s, const TextureLocation *d, uint32_t w, uint32_t h,
                          uint32_t depth, bool)
{
    auto c = p->command;
    auto src = Subresource(s->texture, s->layer, s->mip_level), dst = Subresource(d->texture, d->layer, d->mip_level);
    D3D12_BOX box{s->x, s->y, s->z, s->x + w, s->y + h, s->z + depth};
    Transition(c, src.pResource, D3D12_RESOURCE_STATE_COPY_SOURCE);
    Transition(c, dst.pResource, D3D12_RESOURCE_STATE_COPY_DEST);
    c->list->CopyTextureRegion(&dst, d->x, d->y, d->z, &src, &box);
}
bool SubmitCommandBuffer(CommandBuffer *raw)
{
    std::unique_ptr<CommandBuffer> c(raw);
    for (auto &state : c->states)
        Transition(c.get(), state.first, D3D12_RESOURCE_STATE_COMMON);
    Check(c->list->Close(), "Close command list");
    ID3D12CommandList *lists[] = {c->list.Get()};
    c->d->queue->ExecuteCommandLists(1, lists);
    auto d = c->d;
    HRESULT presented = c->back ? d->swap->Present(d->sync, 0) : S_OK;
    auto fence = Signal(d);
    c->retained.emplace_back(c->allocator.Get());
    c->retained.emplace_back(c->list.Get());
    if (c->views.object)
        c->retained.emplace_back(c->views.object.Get());
    if (c->samplers.object)
        c->retained.emplace_back(c->samplers.object.Get());
    Submission submission{fence, std::move(c->retained), {}};
    if (c->back)
    {
        d->backbuffer_fences[c->back - d->backbuffers.data()] = fence;
        submission.retired_editor_slots.swap(d->retired_editor_slots);
        submission.retained.insert(submission.retained.end(), d->retired_editor_resources.begin(),
                                   d->retired_editor_resources.end());
        d->retired_editor_resources.clear();
    }
    d->in_flight.push_back(std::move(submission));
    Check(presented, "Present");
    return true;
}
Fence *SubmitCommandBufferAndAcquireFence(CommandBuffer *c)
{
    auto d = c->d;
    SubmitCommandBuffer(c);
    return new Fence{d, d->next};
}
bool WaitForFences(Device *, bool, Fence *const *fences, uint32_t n)
{
    for (UINT i = 0; i < n; ++i)
        Wait(fences[i]->d, fences[i]->value);
    return true;
}
void ReleaseFence(Device *, Fence *f)
{
    delete f;
}
bool CancelCommandBuffer(CommandBuffer *c)
{
    delete c;
    return true;
}
}
