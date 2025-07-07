#include <editor/EditorManagerD3D11.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

// #include <base/Context.h>
// #include <render/RenderEngine.h>

namespace EditorWorker
{
//using namespace RenderWorker;

EditorManagerD3D11::EditorManagerD3D11()
{
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    
    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();

    // 设置平台/渲染器后端
    // ImGui_ImplWin32_Init(Context::Instance().AppInstance().GetHWND());
	// const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
    // auto re = d3d11_re.D3DDevice();
    // auto ctx = d3d11_re.D3DDeviceImmContext();
    //ImGui_ImplDX11_Init(re, ctx);
}

EditorManagerD3D11::~EditorManagerD3D11()
{
    ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
}