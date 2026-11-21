#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#include <editor/EditorManagerMetal.h>
#include <base/Window.h>
#include <MetalRenderEngine.h>
#include <MetalTexture.h>
#include <imgui_impl_osx.h>
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
    auto* window = (__bridge NSWindow*)MainWnd()->NativeHandle();
    if (!ImGui_ImplOSX_Init(window.contentView))
        TMSG("ImGui Cocoa initialization failed");
    imgui_text_responder_ = (__bridge void*)window.firstResponder;
    [window makeFirstResponder:window.contentView];
    if (!ImGui_ImplMetal_Init((__bridge id<MTLDevice>)metal::NativeDevice(Renderer().Device())))
        TMSG("ImGui Metal initialization failed");
}
void EditorManagerMetal::ShutdownImGui()
{
    ImGui_ImplMetal_Shutdown();
    auto* window = (__bridge NSWindow*)MainWnd()->NativeHandle();
    [window makeFirstResponder:window.contentView];
    imgui_text_responder_ = nullptr;
    ImGui_ImplOSX_Shutdown();
}
void EditorManagerMetal::NewImGuiFrame() const
{
    auto desc = [MTLRenderPassDescriptor renderPassDescriptor];
    auto *tex = Renderer().CurrentSwapchainTexture();
    desc.colorAttachments[0].texture = (__bridge id<MTLTexture>)metal::TextureHandle(tex);
    ImGui_ImplMetal_NewFrame(desc);
    auto* window = (__bridge NSWindow*)MainWnd()->NativeHandle();
    // Only the UI that owns text input should be the Cocoa IME responder.
    NSResponder* responder = ImGui::GetIO().WantTextInput && imgui_text_responder_
        ? (__bridge NSResponder*)imgui_text_responder_ : window.contentView;
    if (window.firstResponder != responder)
        [window makeFirstResponder:responder];
    ImGui_ImplOSX_NewFrame(window.contentView);
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
