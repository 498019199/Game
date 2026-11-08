// Checks actual Metal pixels for the shared RmlUi shader, font texture and scissor rectangle.
#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/UIManager.h>
#include <base/Window.h>
#include <render/RenderFactory.h>
#include <render/RenderEngine.h>
#include <render/FrameBuffer.h>
#include <SDL3RenderEngine.h>
#include <SDL3Texture.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <iostream>

using namespace RenderWorker;
namespace
{
constexpr uint32_t size = 128;
void Require(bool value, char const* message) { if (!value) throw std::runtime_error(message); }
class App final : public App3D
{
public: App() : App3D("RmlUi GPU Tests") {}
private: uint32_t DoUpdate(uint32_t) override { return URV_Finished; }
};
std::vector<uint8_t> Readback(SDL3RenderEngine& re, TexturePtr const& texture)
{
    std::vector<uint8_t> pixels(size * size * 4);
    SDL_GPUTransferBufferCreateInfo info{};
    info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD; info.size = static_cast<Uint32>(pixels.size());
    auto* buffer = SDL_CreateGPUTransferBuffer(re.Device(), &info);
    Require(buffer != nullptr, "Readback buffer creation failed");
    auto release_buffer = CommonWorker::Defer([&] { SDL_ReleaseGPUTransferBuffer(re.Device(), buffer); });
    auto* commands = SDL_AcquireGPUCommandBuffer(re.Device());
    Require(commands != nullptr, "Readback command allocation failed");
    auto* copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureRegion source{};
    source.texture = dynamic_cast<SDL3Texture&>(*texture).GpuTexture(); source.w = size; source.h = size; source.d = 1;
    SDL_GPUTextureTransferInfo dest{}; dest.transfer_buffer = buffer; dest.pixels_per_row = size; dest.rows_per_layer = size;
    SDL_DownloadFromGPUTexture(copy, &source, &dest);
    SDL_EndGPUCopyPass(copy);
    auto* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    Require(fence != nullptr, "Readback submit failed");
    auto release_fence = CommonWorker::Defer([&] { SDL_ReleaseGPUFence(re.Device(), fence); });
    Require(SDL_WaitForGPUFences(re.Device(), true, &fence, 1), "Readback wait failed");
    auto* bytes = SDL_MapGPUTransferBuffer(re.Device(), buffer, false);
    Require(bytes != nullptr, "Readback map failed");
    std::memcpy(pixels.data(), bytes, pixels.size());
    SDL_UnmapGPUTransferBuffer(re.Device(), buffer);
    return pixels;
}
void Test()
{
    auto& ctx = Context::Instance(); auto& rf = ctx.RenderFactoryInstance();
    auto& re = dynamic_cast<SDL3RenderEngine&>(rf.RenderEngineInstance());
    auto& ui = ctx.UIManagerInstance();
    ui.Init(); Require(ui.Valid(), "RmlUi pipeline/context initialization failed");
    ui.SetDimensions(size, size);
    auto* doc = ui.LoadDocument(RMLUI_GPU_FIXTURE);
    Require(doc != nullptr, "RmlUi probe document failed to load");
    ui.ShowDocument(doc);
    auto target = rf.MakeTexture2D(size, size, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
    auto fb = rf.MakeFrameBuffer(); fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(target, 0, 1, 0));
    fb->Viewport()->Camera(re.CurFrameBuffer()->Viewport()->Camera());
    fb->Viewport()->Width(size); fb->Viewport()->Height(size);
    auto draw = [&] {
        re.BeginFrame(); re.BindFrameBuffer(fb);
        fb->Clear(FrameBuffer::CBM_Color, Color(0,0,0,1), 1, 0);
        ui.RenderIntoGameView();
        re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
        return Readback(re, target);
    };
    auto const pixels = draw();
    auto pixel = [&](uint32_t x, uint32_t y, uint32_t c) { return pixels[(y * size + x) * 4 + c]; };
    Require(pixel(12,12,0) > 220 && pixel(12,12,1) < 10, "RmlUi solid geometry/transform/color was not rendered");
    Require(pixel(50,12,0) < 10, "RmlUi solid geometry escaped its bounds");
    Require(pixel(12,44,1) > 220, "RmlUi clipped geometry was not rendered");
    Require(pixel(30,44,1) < 10, "RmlUi scissor did not clip overflow");
    size_t glyph_pixels = 0;
    for (uint32_t y = 72; y < 110; ++y)
        for (uint32_t x = 0; x < size; ++x)
            if (pixel(x,y,0) > 40 && pixel(x,y,1) > 40 && pixel(x,y,2) > 40) ++glyph_pixels;
    Require(glyph_pixels > 30, "RmlUi font atlas produced no visible text");
    ui.HideDocument(doc);
    auto const hidden = draw();
    for (size_t i = 0; i < hidden.size(); i += 4)
        Require(hidden[i] < 10 && hidden[i+1] < 10 && hidden[i+2] < 10, "Hidden RmlUi document still draws");
    ui.ShowDocument(doc);
    Require(draw() == pixels, "RmlUi show/hide cycle changed pixels");
    ui.CloseDocument(doc);
    std::cout << "PASS: RmlUi Metal pixels, transform, solid color, clipping, font atlas (" << glyph_pixels
        << " pixels), hide/show" << std::endl;
}
}
int main()
{
    int result = 0;
    try {
        auto& ctx = Context::Instance(); ctx.LoadConfig("KlayGE.cfg"); auto cfg = ctx.Config();
        cfg.render_factory_name = "SDL3";
        cfg.graphics_cfg.width = size; cfg.graphics_cfg.height = size;
        cfg.graphics_cfg.full_screen = false; cfg.graphics_cfg.gamma = false; cfg.graphics_cfg.hide_win = true;
        ctx.Config(cfg);
        { App app; app.Create(); Test(); }
    } catch (std::exception const& e) { std::cerr << "FAIL: " << e.what() << std::endl; result = 1; }
    Context::Destroy();
    return result;
}
