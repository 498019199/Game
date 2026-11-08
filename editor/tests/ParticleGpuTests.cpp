// Bounded native GPU probe. Owns its scene and never loads user scene assets.
#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/ResLoader.h>
#include <base/Window.h>
#include <render/FrameBuffer.h>
#include <render/ParticleRenderable.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <world/ParticleSystem.h>
#include <world/SceneNode.h>
#include <SDL3RenderEngine.h>
#include <SDL3Texture.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace RenderWorker;
namespace
{
constexpr uint32_t size = 128;
void Require(bool value, char const* message) { if (!value) throw std::runtime_error(message); }
class App final : public App3D
{
public: App() : App3D("Particle GPU Tests") {}
private: uint32_t DoUpdate(uint32_t) override { return URV_Finished; }
};
std::vector<uint8_t> Readback(RenderFactory& rf, TexturePtr const& texture)
{
    auto const width = texture->Width(0), height = texture->Height(0);
    std::vector<uint8_t> pixels(width * height * 4);
    auto& re = rf.RenderEngineInstance();
    if (auto* sdl = dynamic_cast<SDL3RenderEngine*>(&re))
    {
        SDL_GPUTransferBufferCreateInfo info{};
        info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD; info.size = static_cast<Uint32>(pixels.size());
        auto* buffer = SDL_CreateGPUTransferBuffer(sdl->Device(), &info);
        Require(buffer != nullptr, "Readback buffer creation failed");
        auto release_buffer = CommonWorker::Defer([&] { SDL_ReleaseGPUTransferBuffer(sdl->Device(), buffer); });
        auto* commands = SDL_AcquireGPUCommandBuffer(sdl->Device());
        Require(commands != nullptr, "Readback command allocation failed");
        auto* copy = SDL_BeginGPUCopyPass(commands);
        SDL_GPUTextureRegion source{};
        source.texture = dynamic_cast<SDL3Texture&>(*texture).GpuTexture(); source.w = width; source.h = height; source.d = 1;
        SDL_GPUTextureTransferInfo dest{};
        dest.transfer_buffer = buffer; dest.pixels_per_row = width; dest.rows_per_layer = height;
        SDL_DownloadFromGPUTexture(copy, &source, &dest);
        SDL_EndGPUCopyPass(copy);
        auto* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
        Require(fence != nullptr, "Readback submit failed");
        auto release_fence = CommonWorker::Defer([&] { SDL_ReleaseGPUFence(sdl->Device(), fence); });
        Require(SDL_WaitForGPUFences(sdl->Device(), true, &fence, 1), "Readback wait failed");
        auto* bytes = SDL_MapGPUTransferBuffer(sdl->Device(), buffer, false);
        Require(bytes != nullptr, "Readback map failed");
        std::memcpy(pixels.data(), bytes, pixels.size());
        SDL_UnmapGPUTransferBuffer(sdl->Device(), buffer);
    }
    else
    {
        auto staging = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read);
        texture->CopyToTexture(*staging, TextureFilter::Point);
        void* data = nullptr; uint32_t pitch = 0;
        staging->Map2D(0, 0, TMA_Read_Only, 0, 0, width, height, data, pitch);
        for (uint32_t y = 0; y < height; ++y)
            std::memcpy(pixels.data() + y * width * 4, static_cast<uint8_t*>(data) + y * pitch, width * 4);
        staging->Unmap2D(0, 0);
    }
    return pixels;
}
size_t VisiblePixels(std::vector<uint8_t> const& pixels)
{
    size_t count = 0;
    for (size_t i = 0; i < pixels.size(); i += 4)
        if (pixels[i] > 2 || pixels[i+1] > 2 || pixels[i+2] > 2) ++count;
    return count;
}
double HorizontalCenter(std::vector<uint8_t> const& pixels)
{
    double sum = 0, weight = 0;
    for (uint32_t y = 0; y < size; ++y)
        for (uint32_t x = 0; x < size; ++x)
        {
            auto const i = (y * size + x) * 4;
            auto const value = pixels[i] + pixels[i+1] + pixels[i+2];
            weight += value; sum += x * value;
        }
    return sum / std::max(1.0, weight);
}
double Energy(std::vector<uint8_t> const& pixels)
{
    double total = 0;
    for (size_t i = 0; i < pixels.size(); i += 4) total += pixels[i] + pixels[i+1] + pixels[i+2];
    return total;
}
void WritePreview(char const* name, std::vector<uint8_t> const& pixels)
{
    auto path = std::filesystem::temp_directory_path() / (std::string("game-particles-") + name + ".ppm");
    std::ofstream out(path, std::ios::binary);
    out << "P6\n" << size << ' ' << size << "\n255\n";
    for (size_t i = 0; i < pixels.size(); i += 4) out.write(reinterpret_cast<char const*>(pixels.data() + i), 3);
    Require(out.good(), "Could not save particle preview");
    std::cout << "Preview: " << path.string() << std::endl;
}
void Test(bool write_previews)
{
    auto& rf = Context::Instance().RenderFactoryInstance(); auto& re = rf.RenderEngineInstance();
    auto camera = MakeSharedPtr<Camera>();
    auto camera_node = MakeSharedPtr<SceneNode>(camera, L"ParticleProbeCamera", SceneNode::SOA_Moveable);
    camera_node->TransformToWorld(MathWorker::translation(0.f, 1.f, -5.f));
    camera->ProjOrthoParams(6.f, 6.f, 0.1f, 20.f);
    auto target = rf.MakeTexture2D(size, size, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
    auto fb = rf.MakeFrameBuffer();
    fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(target, 0, 1, 0));
    fb->Attach(rf.Make2DDsv(size, size, EF_D16, 1, 0));
    fb->Viewport()->Camera(camera);
    fb->Viewport()->Width(size); fb->Viewport()->Height(size);
    auto system = MakeSharedPtr<ParticleSystem>();
    auto node = MakeSharedPtr<SceneNode>(system, L"ParticleProbe", SceneNode::SOA_Moveable);
    ParticleRenderable renderer(system);
    renderer.BindSceneNode(node.get());
    auto draw = [&](float depth = 1.f, std::vector<uint8_t> const* background = nullptr)
    {
        Window::PumpEvents();
        if (background) target->UpdateSubresource2D(0, 0, 0, 0, size, size, background->data(), size * 4);
        re.BeginFrame(); re.BindFrameBuffer(fb);
        fb->Clear(FrameBuffer::CBM_Depth | (background ? 0u : FrameBuffer::CBM_Color), Color(0, 0, 0, 1), depth, 0);
        renderer.BindSceneNode(node.get());
        renderer.Render();
        re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
        return Readback(rf, target);
    };
    std::vector<std::vector<uint8_t>> presets;
    for (auto preset : {ParticlePreset::Fire, ParticlePreset::Smoke, ParticlePreset::Sparks, ParticlePreset::Explosion})
    {
        system->Settings(ParticleSettings::ForPreset(preset));
        system->Restart();
        for (int i = 0; i < 40; ++i) system->Update(1.f / 60.f);
        system->Pause();
        Require(!system->Particles().empty(), "Preset emitted no particles");
        auto pixels = draw();
        auto const visible = VisiblePixels(pixels);
        std::cout << "Preset " << static_cast<int>(preset) << ": " << visible << " visible pixels" << std::endl;
        Require(visible > 4, "Preset produced no visible GPU pixels");
        if (write_previews)
        {
            char const* names[] = {"fire", "smoke", "sparks", "explosion"};
            WritePreview(names[static_cast<int>(preset)], pixels);
        }
        presets.push_back(std::move(pixels));
        Require(VisiblePixels(draw(0.f)) == 0, "Particles ignored scene depth");
        system->Stop();
        Require(VisiblePixels(draw()) == 0, "Stopped emitter left stale particle geometry");
    }
    Require(presets[0] != presets[1] && presets[1] != presets[2] && presets[0] != presets[2],
        "Fire/smoke/sparks rendered identical images");

    auto dense = ParticleSettings::ForPreset(ParticlePreset::Fire);
    dense.max_particles = 256; dense.emission_rate = 1000;
    dense.lifetime_min = dense.lifetime_max = 2;
    system->Settings(dense); system->Restart(); system->Update(0.5f); system->Pause();
    Require(system->Particles().size() > 64, "Buffer growth fixture did not exceed initial capacity");
    Require(VisiblePixels(draw()) > 4, "Growing particle buffers lost geometry");

    auto stationary = ParticleSettings::ForPreset(ParticlePreset::Fire);
    stationary.max_particles = 1; stationary.emission_rate = 100;
    stationary.radius = 0; stationary.speed_min = stationary.speed_max = 0;
    stationary.acceleration = float3::Zero();
    stationary.lifetime_min = stationary.lifetime_max = 10;
    stationary.start_size = stationary.end_size = 1;
    stationary.start_color = stationary.end_color = float4(1, 0, 0, 0.6f);
    system->Settings(stationary); system->Restart(); system->Update(0.1f); system->Pause();
    auto const centered = draw();
    node->TransformToWorld(MathWorker::translation(1.f, 0.f, 0.f));
    auto const moved = draw();
    Require(HorizontalCenter(moved) > HorizontalCenter(centered) + 10, "Emitter transform did not move particles");
    node->Visible(false);
    Require(VisiblePixels(draw()) == 0, "Hidden emitter still draws");
    node->Visible(true); node->TransformToWorld(float4x4::Identity());

    stationary.start_color = stationary.end_color = float4(0, 0, 1, 0.6f);
    auto back_system = MakeSharedPtr<ParticleSystem>(stationary);
    back_system->Update(0.1f); back_system->Pause();
    auto back_node = MakeSharedPtr<SceneNode>(back_system, L"DepthWriteProbe", SceneNode::SOA_Moveable);
    back_node->TransformToWorld(MathWorker::translation(0.f, 0.f, 1.f));
    ParticleRenderable back_renderer(back_system);
    re.BeginFrame(); re.BindFrameBuffer(fb);
    fb->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 1), 1, 0);
    renderer.BindSceneNode(node.get()); renderer.Render();
    back_renderer.BindSceneNode(back_node.get()); back_renderer.Render();
    re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
    auto const combined = Readback(rf, target);
    size_t blue_pixels = 0;
    for (size_t i = 0; i < combined.size(); i += 4) if (combined[i+2] > 10) ++blue_pixels;
    Require(blue_pixels > 4, "Front particles wrote depth and hid particles behind them");

    // World particles must retain position AND size after moving/scaling the emitter.
    stationary.simulation_space = ParticleSimulationSpace::World;
    system->Settings(stationary); system->Restart(); system->Update(.1f); system->Pause();
    auto world_before = draw();
    node->TransformToWorld(MathWorker::scaling(3.f, 3.f, 3.f) * MathWorker::translation(2.f, 0.f, 0.f));
    Require(draw() == world_before, "World-space particles followed the emitter or changed size after birth");
    node->TransformToWorld(float4x4::Identity());

    auto configure = [&](ParticleSettings const& settings, float dt = .1f) {
        system->Settings(settings); system->Restart(); system->Update(dt); system->Pause();
    };
    stationary.simulation_space = ParticleSimulationSpace::Local;
    stationary.start_color = stationary.end_color = float4(.1f, .08f, .04f, .6f);
    configure(stationary);
    double const baseline = Energy(draw());
    stationary.emissive_intensity = 3.f;
    configure(stationary);
    Require(Energy(draw()) > baseline * 2.5, "Emissive intensity did not increase particle brightness");
    stationary.emissive_intensity = 1;
    stationary.glow_strength = 1;
    configure(stationary);
    Require(Energy(draw()) > baseline * 1.15, "Soft glow did not increase visible light");
    stationary.glow_strength = 0;
    stationary.emission_rate = 0; stationary.burst_count = 1;
    stationary.flicker_frequency = 2; stationary.flicker_amount = 1;
    configure(stationary, 1.f / 120.f);
    double const bright = Energy(draw());
    system->Play(); system->Update(.25f); system->Pause();
    Require(Energy(draw()) < bright * .15, "Flicker frequency and amount did not animate intensity");
    auto const frozen = draw();
    Require(draw() == frozen, "Paused flicker continued animating");

    stationary.flicker_frequency = stationary.flicker_amount = 0;
    stationary.start_size = stationary.end_size = .25f;
    stationary.speed_min = stationary.speed_max = 2;
    stationary.direction = float3(1, 0, 0); stationary.spread = 0;
    stationary.start_color = stationary.end_color = float4(1, 1, 1, 1);
    configure(stationary);
    auto const round_pixels = VisiblePixels(draw());
    stationary.velocity_stretch = .8f;
    configure(stationary);
    auto const stretched = draw();
    Require(VisiblePixels(stretched) > round_pixels * 2, "Velocity stretch did not produce a visible trail");
    if (write_previews) WritePreview("trail", stretched);

    // A patterned background proves refraction rather than drawing a tinted blob.
    std::vector<uint8_t> pattern(size * size * 4);
    for (uint32_t y = 0; y < size; ++y) for (uint32_t x = 0; x < size; ++x) {
        size_t const i = (y * size + x) * 4;
        bool const tile = ((x / 8 + y / 8) % 2) != 0;
        pattern[i] = tile ? 210 : 20;
        pattern[i+1] = tile ? 40 : 180;
        pattern[i+2] = static_cast<uint8_t>(y * 2);
        pattern[i+3] = 255;
    }
    stationary.speed_min = stationary.speed_max = stationary.velocity_stretch = 0;
    stationary.start_size = stationary.end_size = 3;
    stationary.blend = ParticleBlendMode::Distortion;
    stationary.distortion_strength = 0;
    configure(stationary);
    auto const undistorted = draw(1, &pattern);
    size_t identity_errors = 0;
    for (size_t i = 0; i < pattern.size(); ++i)
        if (std::abs(int(pattern[i]) - int(undistorted[i])) > 2) ++identity_errors;
    Require(identity_errors == 0, "Zero-strength refraction changed or flipped the background");
    stationary.distortion_strength = 12;
    configure(stationary);
    auto const warped = draw(1, &pattern);
    size_t displaced = 0;
    for (size_t i = 0; i < pattern.size(); i += 4)
        if (std::abs(int(pattern[i]) - int(warped[i])) > 8 || std::abs(int(pattern[i+1]) - int(warped[i+1])) > 8) ++displaced;
    Require(displaced > 30 && displaced < size * size / 2, "Refraction must displace background only within the particle mask");
    Require(draw(0, &pattern) == pattern, "Occluded refraction changed the scene");
    if (write_previews) { WritePreview("refraction-before", pattern); WritePreview("refraction-after", warped); }
    system->Stop();
    Require(draw(1, &pattern) == pattern, "Stopped distortion retained a stale background or particle");

    // Snapshot reuse must follow framebuffer size and content after resize/target switches.
    TexturePtr snapshot;
    re.BeginFrame(); re.BindFrameBuffer(fb);
    Require(re.CaptureCurrentColor(snapshot), "Scene color capture failed");
    re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
    Require(snapshot != target && Readback(rf, snapshot) == pattern, "Snapshot aliased or changed scene pixels");
    auto const original_snapshot = snapshot;
    auto smaller_target = rf.MakeTexture2D(size / 2, size / 4, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
    auto smaller_fb = rf.MakeFrameBuffer();
    smaller_fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(smaller_target, 0, 1, 0));
    smaller_fb->Viewport()->Width(size / 2); smaller_fb->Viewport()->Height(size / 4);
    re.BeginFrame(); re.BindFrameBuffer(smaller_fb);
    smaller_fb->Clear(FrameBuffer::CBM_Color, Color(0, 1, 0, 1), 1, 0);
    Require(re.CaptureCurrentColor(snapshot), "Capture failed after framebuffer resize");
    re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
    Require(snapshot != original_snapshot && snapshot->Width(0) == size / 2 && snapshot->Height(0) == size / 4,
        "Snapshot did not rebuild for the new framebuffer dimensions");
    auto const resized_pixels = Readback(rf, snapshot);
    for (size_t i = 0; i < resized_pixels.size(); i += 4)
        Require(resized_pixels[i] == 0 && resized_pixels[i+1] == 255 && resized_pixels[i+2] == 0,
            "Resized snapshot retained pixels from the previous target");
    auto const resized_snapshot = snapshot;
    re.BeginFrame(); re.BindFrameBuffer(smaller_fb);
    smaller_fb->Clear(FrameBuffer::CBM_Color, Color(0, 0, 1, 1), 1, 0);
    Require(re.CaptureCurrentColor(snapshot) && snapshot == resized_snapshot, "Compatible snapshot was not reused");
    smaller_fb->Viewport()->Width(0);
    Require(!re.CaptureCurrentColor(snapshot), "Zero-sized viewport must not capture stale content");
    re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
    auto const reused_pixels = Readback(rf, snapshot);
    for (size_t i = 0; i < reused_pixels.size(); i += 4)
        Require(reused_pixels[i] == 0 && reused_pixels[i+1] == 0 && reused_pixels[i+2] == 255,
            "Reused snapshot did not refresh to the current frame");
    std::cout << "PASS: presets, depth, buffers, world space, trails, emission, glow, flicker, refraction and capture resize" << std::endl;
}
}
int main(int argc, char** argv)
{
    int result = 0;
    try
    {
        auto& ctx = Context::Instance(); ctx.LoadConfig("KlayGE.cfg"); auto cfg = ctx.Config();
        cfg.render_factory_name = argc > 1 ? argv[1] : "SDL3";
        cfg.graphics_cfg.width = size; cfg.graphics_cfg.height = size;
        cfg.graphics_cfg.full_screen = false; cfg.graphics_cfg.gamma = false;
        ctx.Config(cfg);
        ctx.ResLoaderInstance().AddPath("../../assets/shaders");
        ctx.ResLoaderInstance().AddPath("../../assets");
        {
            App app; app.Create();
            if (app.MainWnd()->GetSDLWindow())
            {
                Require(SDL_ShowWindow(app.MainWnd()->GetSDLWindow()), "Cannot show GPU probe window");
                Require(SDL_SyncWindow(app.MainWnd()->GetSDLWindow()), "Cannot sync GPU probe window");
            }
            Test(argc > 2 && std::string_view(argv[2]) == "--write-previews");
        }
    }
    catch (std::exception const& e) { std::cerr << "FAIL: " << e.what() << std::endl; result = 1; }
    Context::Destroy();
    return result;
}
