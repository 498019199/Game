#import <Metal/Metal.h>
#include <editor/EditorManagerMetal.h>
#include <base/Window.h>
#include <MetalRenderEngine.h>
#include <MetalTexture.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>

namespace EditorWorker
{
using namespace RenderWorker;
static MetalRenderEngine &Renderer()
{
    return checked_cast<MetalRenderEngine &>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
}
EditorManagerMetal::EditorManagerMetal() = default;
EditorManagerMetal::~EditorManagerMetal() = default;
void EditorManagerMetal::InitializeImGui()
{
    ImGui_ImplGlfw_InitForOther(MainWnd()->DesktopWindow(), true);
    if (!ImGui_ImplMetal_Init((__bridge id<MTLDevice>)metal::NativeDevice(Renderer().Device())))
        TMSG("ImGui Metal initialization failed");
}
void EditorManagerMetal::ShutdownImGui()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}
void EditorManagerMetal::NewImGuiFrame() const
{
    auto desc = [MTLRenderPassDescriptor renderPassDescriptor];
    auto *tex = Renderer().CurrentSwapchainTexture();
    desc.colorAttachments[0].texture = (__bridge id<MTLTexture>)metal::TextureHandle(tex);
    ImGui_ImplMetal_NewFrame(desc);
    ImGui_ImplGlfw_NewFrame();
}
void EditorManagerMetal::RenderImGuiDrawData(ImDrawData *data) const
{
    auto &re = Renderer();
    re.BeginOverlayPass();
    if (auto *pass = re.CurrentRenderPass())
    {
        ImGui_ImplMetal_NewFrame((__bridge MTLRenderPassDescriptor *)metal::NativePassDescriptor(pass));
        ImGui_ImplMetal_RenderDrawData(data,
                                       (__bridge id<MTLCommandBuffer>)metal::NativeCommands(re.CurrentCommandBuffer()),
                                       (__bridge id<MTLRenderCommandEncoder>)metal::NativeEncoder(pass));
    }
}
void *EditorManagerMetal::GameViewTextureHandle() const
{
    auto tex = dynamic_cast<MetalTexture *>(GameViewColorTexture().get());
    return tex ? metal::TextureHandle(tex->GpuTexture()) : nullptr;
}
void *EditorManagerMetal::TextureHandle(ShaderResourceView const *srv) const
{
    auto tex = srv ? dynamic_cast<MetalTexture *>(srv->TextureResource().get()) : nullptr;
    return tex ? metal::TextureHandle(tex->GpuTexture()) : nullptr;
}
}
