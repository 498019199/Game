#include <editor/EditorManagerD3D12.h>
#include <base/Window.h>
#include <D3D12RenderEngine.h>
#include <D3D12Texture.h>
#include <D3D12Interop.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_glfw.h>

namespace EditorWorker
{
using namespace RenderWorker;
static D3D12RenderEngine &Renderer()
{
    return checked_cast<D3D12RenderEngine &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
}
EditorManagerD3D12::EditorManagerD3D12() = default;
EditorManagerD3D12::~EditorManagerD3D12() = default;
void EditorManagerD3D12::InitializeImGui()
{
    auto d = Renderer().Device();
    ImGui_ImplGlfw_InitForOther(MainWnd()->DesktopWindow(), true);
    ImGui_ImplDX12_InitInfo info{};
    info.Device = static_cast<ID3D12Device *>(d3d12::NativeDevice(d));
    info.CommandQueue = d3d12::NativeQueue(d);
    info.NumFramesInFlight = 2;
    info.RTVFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    switch (Context::Instance().Config().graphics_cfg.depth_stencil_fmt)
    {
    case EF_D16:
        info.DSVFormat = DXGI_FORMAT_D16_UNORM;
        break;
    case EF_D24S8:
        info.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        break;
    case EF_D32F:
        info.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        break;
    default:
        info.DSVFormat = DXGI_FORMAT_UNKNOWN;
        break;
    }
    info.UserData = d;
    info.SrvDescriptorHeap = d3d12::EditorHeap(d);
    info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *init, D3D12_CPU_DESCRIPTOR_HANDLE *cpu,
                                   D3D12_GPU_DESCRIPTOR_HANDLE *handle) {
        d3d12::AllocateEditorView(static_cast<d3d12::Device *>(init->UserData), *cpu, *handle);
    };
    info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *init, D3D12_CPU_DESCRIPTOR_HANDLE cpu,
                                  D3D12_GPU_DESCRIPTOR_HANDLE) {
        d3d12::FreeEditorView(static_cast<d3d12::Device *>(init->UserData), cpu);
    };
    if (!ImGui_ImplDX12_Init(&info))
        TMSG("ImGui Direct3D 12 initialization failed");
}
void EditorManagerD3D12::ShutdownImGui()
{
    d3d12::WaitForIdle(Renderer().Device());
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}
void EditorManagerD3D12::NewImGuiFrame() const
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}
void EditorManagerD3D12::RenderImGuiDrawData(ImDrawData *data) const
{
    auto &re = Renderer();
    re.BeginOverlayPass();
    if (auto pass = re.CurrentRenderPass())
    {
        auto list = static_cast<ID3D12GraphicsCommandList *>(d3d12::NativeEncoder(pass));
        auto heap = d3d12::EditorHeap(re.Device());
        list->SetDescriptorHeaps(1, &heap);
        ImGui_ImplDX12_RenderDrawData(data, list);
    }
}
void *EditorManagerD3D12::GameViewTextureHandle() const
{
    auto t = dynamic_cast<D3D12Texture *>(GameViewColorTexture().get());
    return t ? d3d12::EditorTexture(t->GpuTexture()) : nullptr;
}
void *EditorManagerD3D12::TextureHandle(ShaderResourceView const *srv) const
{
    auto t = srv ? dynamic_cast<D3D12Texture *>(srv->TextureResource().get()) : nullptr;
    return t ? d3d12::EditorTexture(t->GpuTexture()) : nullptr;
}
}
