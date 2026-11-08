#include <editor/EditorManagerD3D11.h>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <d3d11.h>
#include <base/Window.h>
#include <render/RenderFactory.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace EditorWorker
{
using namespace RenderWorker;

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
void EditorManagerD3D11::ProcessWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}
#endif

EditorManagerD3D11::EditorManagerD3D11() = default;
EditorManagerD3D11::~EditorManagerD3D11() = default;

void EditorManagerD3D11::InitializeImGui()
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    ImGui_ImplWin32_Init(Context::Instance().AppInstance().MainWnd()->GetHWND());
    ImGui_ImplDX11_Init(static_cast<ID3D11Device*>(re.GetD3DDevice()),
        static_cast<ID3D11DeviceContext*>(re.GetD3DDeviceImmContext()));
}

void EditorManagerD3D11::ShutdownImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
}

void EditorManagerD3D11::NewImGuiFrame() const
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
}

void EditorManagerD3D11::RenderImGuiDrawData(ImDrawData* draw_data) const
{
    ImGui_ImplDX11_RenderDrawData(draw_data);
}

void* EditorManagerD3D11::GameViewTextureHandle() const
{
    auto const& srv = GameViewShaderResource();
    return srv ? srv->GetShaderResourceView() : nullptr;
}

void* EditorManagerD3D11::TextureHandle(ShaderResourceView const* srv) const
{
    return srv ? const_cast<ShaderResourceView*>(srv)->GetShaderResourceView() : nullptr;
}
}
