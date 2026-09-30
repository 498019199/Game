// Bounded native-GPU integration probe. Does not load or change the user's scene.
#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <base/ResLoader.h>
#include <game/Model.h>
#include <render/RenderFactory.h>
#include <render/RenderEngine.h>
#include <render/RenderLayout.h>
#include <render/FrameBuffer.h>
#include <SDL3RenderEngine.h>
#include <SDL3Texture.h>
#include <SDL3/SDL.h>
#include <iostream>
#include <array>
#include <cmath>
#include <dbghelp.h>

using namespace RenderWorker;
namespace
{
constexpr uint32_t size = 32;
void Require(bool value, char const* message) { if (!value) throw std::runtime_error(message); }
class App final : public App3D
{
public: App() : App3D("Material GPU Tests") {}
private: uint32_t DoUpdate(uint32_t) override { return URV_Finished; }
};
class ProbeMesh final : public DetailedMesh
{
public:
    ProbeMesh() : DetailedMesh(L"MaterialProbe") {}
    void Draw(RenderLayout const& layout) {
        OnRenderBegin();
        *effect_->ParameterByName("worldviewproj") = float4x4::Identity();
        *effect_->ParameterByName("pos_center") = float3(0, 0, 0);
        *effect_->ParameterByName("pos_extent") = float3(1, 1, 1);
        *effect_->ParameterByName("tc_center") = float2(0, 0);
        *effect_->ParameterByName("tc_extent") = float2(1, 1);
        if (auto p = effect_->ParameterByName("eye_pos_os")) *p = float3(0, 0, 3);
        Context::Instance().RenderFactoryInstance().RenderEngineInstance().Render(*effect_, *technique_, layout);
    }
    RenderEffect* Effect() const { return effect_.get(); }
};
std::vector<uint8_t> Readback(RenderFactory& rf, TexturePtr const& texture)
{
    std::vector<uint8_t> pixels(size * size * 4);
    auto& re = rf.RenderEngineInstance();
    if (auto* sdl = dynamic_cast<SDL3RenderEngine*>(&re)) {
        SDL_GPUTransferBufferCreateInfo info{}; info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD; info.size = static_cast<Uint32>(pixels.size());
        auto buffer = SDL_CreateGPUTransferBuffer(sdl->Device(), &info);
        Require(buffer != nullptr, "SDL readback buffer creation failed");
        auto commands = SDL_AcquireGPUCommandBuffer(sdl->Device());
        Require(commands != nullptr, "SDL readback commands failed");
        auto copy = SDL_BeginGPUCopyPass(commands);
        SDL_GPUTextureRegion source{}; source.texture = dynamic_cast<SDL3Texture&>(*texture).GpuTexture(); source.w = size; source.h = size; source.d = 1;
        SDL_GPUTextureTransferInfo dest{}; dest.transfer_buffer = buffer; dest.pixels_per_row = size; dest.rows_per_layer = size;
        SDL_DownloadFromGPUTexture(copy, &source, &dest);
        SDL_EndGPUCopyPass(copy);
        auto fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
        Require(fence != nullptr, "SDL readback submit failed");
        Require(SDL_WaitForGPUFences(sdl->Device(), true, &fence, 1), "SDL readback wait failed");
        auto bytes = SDL_MapGPUTransferBuffer(sdl->Device(), buffer, false);
        Require(bytes != nullptr, "SDL readback map failed");
        std::memcpy(pixels.data(), bytes, pixels.size());
        SDL_UnmapGPUTransferBuffer(sdl->Device(), buffer);
        SDL_ReleaseGPUFence(sdl->Device(), fence); SDL_ReleaseGPUTransferBuffer(sdl->Device(), buffer);
    } else {
        auto staging = rf.MakeTexture2D(size, size, 1, 1, EF_ABGR8, 1, 0, EAH_CPU_Read);
        texture->CopyToTexture(*staging, TextureFilter::Point);
        void* data = nullptr; uint32_t pitch = 0;
        staging->Map2D(0, 0, TMA_Read_Only, 0, 0, size, size, data, pitch);
        for (uint32_t y = 0; y < size; ++y) std::memcpy(pixels.data() + y * size * 4, static_cast<uint8_t*>(data) + y * pitch, size * 4);
        staging->Unmap2D(0, 0);
    }
    return pixels;
}
double Difference(std::vector<uint8_t> const& a, std::vector<uint8_t> const& b)
{
    double difference = 0;
    for (size_t i = 0; i < a.size(); ++i) if (i % 4 != 3) difference += std::abs(int(a[i]) - int(b[i]));
    return difference / (size * size * 3);
}
void Test()
{
    auto& rf = Context::Instance().RenderFactoryInstance(); auto& re = rf.RenderEngineInstance();
    auto target = rf.MakeTexture2D(size, size, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
    auto fb = rf.MakeFrameBuffer(); fb->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(target, 0, 1, 0));
    fb->Viewport()->Camera(re.CurFrameBuffer()->Viewport()->Camera());
    fb->Attach(rf.Make2DDsv(size, size, EF_D16, 1, 0));
    struct Vertex { float pos[4]; float uv[2]; float tangent[4]; };
    Vertex vertices[] = {
        {{-1,-1,0.5f,1},{0,1},{0.5f,0.5f,0.5f,1}}, {{-1,1,0.5f,1},{0,0},{0.5f,0.5f,0.5f,1}},
        {{1,-1,0.5f,1},{1,1},{0.5f,0.5f,0.5f,1}}, {{1,1,0.5f,1},{1,0},{0.5f,0.5f,0.5f,1}}};
    auto layout = rf.MakeRenderLayout(); layout->TopologyType(RenderLayout::TT_TriangleStrip); layout->NumVertices(4);
    VertexElement format[] = {{VEU_Position,0,EF_ABGR32F},{VEU_TextureCoord,0,EF_GR32F},{VEU_Tangent,0,EF_ABGR32F}};
    layout->BindVertexStream(rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(vertices), vertices), format);
    auto material = std::make_shared<RenderMaterial>();
    material->Glossiness(0); Require(material->Glossiness() == 0, "GPU glossiness zero changed");
    material->Glossiness(1); Require(material->Glossiness() == 1, "GPU glossiness one changed");
    material->Template("EditorLitV1"); material->Albedo(float4(0.2f,0.3f,0.4f,1)); material->Metalness(0.3f);
    material->Glossiness(0.3f); material->NormalScale(1); material->OcclusionStrength(1); material->Emissive(float3(0,0,0));
    ProbeMesh mesh; mesh.Material(material);
    auto draw = [&] {
        re.BeginFrame(); re.BindFrameBuffer(fb);
        fb->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0,0,0,1), 1, 0);
        mesh.Draw(*layout); re.BindFrameBuffer(FrameBufferPtr()); re.EndFrame();
        return Readback(rf, target);
    };
    auto base = draw(); auto effect = mesh.Effect();
    std::vector<uint8_t> black(base.size(), 0);
    Require(Difference(base, black) > 2, "Probe geometry rendered no visible pixels");
    auto differs = [&](char const* name) { auto pixels = draw(); auto delta = Difference(base, pixels);
        std::cout << name << " mean pixel difference=" << delta << std::endl;
        Require(delta > 0.1, name); Require(mesh.Effect() == effect, "Parameter edit recreated effect"); };
    material->Albedo(float4(0.8f,0.1f,0.1f,1)); differs("Albedo"); material->Albedo(float4(0.2f,0.3f,0.4f,1));
    material->Metalness(1); differs("Metalness"); material->Metalness(0.3f);
    material->Glossiness(0.85f); differs("Glossiness"); material->Glossiness(0.3f);
    material->Emissive(float3(0.2f,0.1f,0.1f)); differs("Emissive"); material->Emissive(float3(0,0,0));
    auto texture = [&](uint32_t rgba) {
        ElementInitData init{}; init.data = &rgba; init.row_pitch = 4; init.slice_pitch = 4;
        return rf.MakeTextureSrv(rf.MakeTexture2D(1,1,1,1,EF_ABGR8,1,0,EAH_GPU_Read | EAH_Immutable, std::span(&init,1)));
    };
    material->Texture(RenderMaterial::TS_Albedo, texture(0xff204080)); differs("Albedo texture"); material->Texture(RenderMaterial::TS_Albedo, {});
    material->Texture(RenderMaterial::TS_MetalnessGlossiness, texture(0xff002040)); differs("MetalnessGlossiness texture"); material->Texture(RenderMaterial::TS_MetalnessGlossiness, {});
    material->Texture(RenderMaterial::TS_Normal, texture(0xffff80c0)); differs("Normal texture");
    auto tilted = draw(); material->NormalScale(0); Require(Difference(tilted, draw()) > 0.1, "NormalScale has no effect");
    material->Texture(RenderMaterial::TS_Normal, {}); material->NormalScale(1);
    material->Texture(RenderMaterial::TS_Occlusion, texture(0xff202020)); differs("Occlusion texture");
    auto occluded = draw(); material->OcclusionStrength(0); Require(Difference(occluded, draw()) > 0.1, "OcclusionStrength has no effect");
    material->Texture(RenderMaterial::TS_Occlusion, {}); material->OcclusionStrength(1);
    material->Emissive(float3(0.2f,0.2f,0.2f)); auto emitting = draw();
    material->Texture(RenderMaterial::TS_Emissive, texture(0xff204080)); Require(Difference(emitting, draw()) > 0.1, "Emissive texture has no effect");
    material->Texture(RenderMaterial::TS_Emissive, {}); material->Emissive(float3(0,0,0));
    material->Template(""); auto legacy = draw(); Require(mesh.Effect() != effect, "Legacy did not rebind");
    material->Template("EditorLitV1"); auto rebound = draw();
    Require(Difference(base, rebound) < 0.1, "Rebinding template changed material output");
    std::cout << "PASS: native shader compile, all exposed parameters/textures, DetailedMesh legacy/editor rebind" << std::endl;
}
}
int main(int argc, char** argv)
{
#if defined(_M_X64)
    ::SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* exception) -> LONG {
        auto process = GetCurrentProcess(); SymInitialize(process, nullptr, TRUE);
        CONTEXT context = *exception->ContextRecord;
        STACKFRAME64 frame{};
        frame.AddrPC = {context.Rip, 0, AddrModeFlat}; frame.AddrStack = {context.Rsp, 0, AddrModeFlat}; frame.AddrFrame = {context.Rbp, 0, AddrModeFlat};
        for (int i = 0; i < 24; ++i) {
            char storage[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
            auto symbol = reinterpret_cast<SYMBOL_INFO*>(storage); symbol->SizeOfStruct = sizeof(SYMBOL_INFO); symbol->MaxNameLen = MAX_SYM_NAME;
            DWORD64 displacement = 0; IMAGEHLP_LINE64 line{}; line.SizeOfStruct = sizeof(line); DWORD offset = 0;
            if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) std::cerr << symbol->Name;
            if (SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset, &line)) std::cerr << " " << line.FileName << ":" << line.LineNumber;
            std::cerr << std::endl;
            if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, GetCurrentThread(), &frame, &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) break;
        }
        return EXCEPTION_EXECUTE_HANDLER;
    });
#endif
    int result = 0;
    try {
        auto& ctx = Context::Instance(); ctx.LoadConfig("KlayGE.cfg"); auto cfg = ctx.Config();
        cfg.render_factory_name = argc > 1 ? argv[1] : "SDL3";
        cfg.graphics_cfg.width = 64; cfg.graphics_cfg.height = 64; cfg.graphics_cfg.full_screen = false; cfg.graphics_cfg.gamma = false;
        ctx.Config(cfg);
        ctx.ResLoaderInstance().AddPath("../../Assets/Shaders"); ctx.ResLoaderInstance().AddPath("../../Assets");
        { App app; app.Create(); ::ShowWindow(app.MainWnd()->GetHWND(), SW_HIDE); Test(); }
    } catch (std::exception const& e) { std::cerr << "FAIL: " << e.what() << std::endl; result = 1; }
    Context::Destroy();
    return result;
}
