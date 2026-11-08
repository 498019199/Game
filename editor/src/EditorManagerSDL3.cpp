#include <editor/EditorManagerSDL3.h>
#include <base/Window.h>

#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <SDL3/SDL.h>

#include <SDL3RenderEngine.h>
#include <SDL3Texture.h>
#include <render/RenderFactory.h>
#include <common/Log.h>

namespace EditorWorker
{
using namespace RenderWorker;

EditorManagerSDL3::EditorManagerSDL3() = default;
EditorManagerSDL3::~EditorManagerSDL3() = default;

void EditorManagerSDL3::InitializeImGui()
{
    auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
    ImGui_ImplSDL3_InitForSDLGPU(re.Window());
    sdl_events_ = MainWnd()->OnSDLEvent().Connect([](SDL_Event const& event) {
        ImGui_ImplSDL3_ProcessEvent(&event);
    });

    ImGui_ImplSDLGPU3_InitInfo init_info{};
    init_info.Device = re.Device();
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(re.Device(), re.Window());
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    if (!ImGui_ImplSDLGPU3_Init(&init_info))
    {
        LogError() << "Editor: ImGui SDLGPU3 backend failed to initialize." << std::endl;
    }
}

void EditorManagerSDL3::ShutdownImGui()
{
    sdl_events_.Disconnect();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
}

void EditorManagerSDL3::NewImGuiFrame() const
{
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
}

void EditorManagerSDL3::RenderImGuiDrawData(ImDrawData* draw_data) const
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    auto& sdl_re = checked_cast<SDL3RenderEngine&>(re);
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, sdl_re.CurrentCommandBuffer());
    re.BeginOverlayPass();
    ImGui_ImplSDLGPU3_RenderDrawData(draw_data, sdl_re.CurrentCommandBuffer(), sdl_re.CurrentRenderPass());
}

void* EditorManagerSDL3::GameViewTextureHandle() const
{
    auto const* tex = dynamic_cast<SDL3Texture*>(GameViewColorTexture().get());
    return tex ? static_cast<void*>(tex->GpuTexture()) : nullptr;
}

void* EditorManagerSDL3::TextureHandle(ShaderResourceView const* srv) const
{
    if (!srv || !srv->TextureResource())
    {
        return nullptr;
    }
    auto const* tex = dynamic_cast<SDL3Texture*>(srv->TextureResource().get());
    return tex ? static_cast<void*>(tex->GpuTexture()) : nullptr;
}
}
