#include <editor/EditorManagerD3D11.h>
#include <editor/EditorConsolePanel.h>
#include <editor/EditorHierarchyPanel.h>
#include <editor/EditorMainBarPanel.h>
#include <editor/EditorInspectorPanel.h>


#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include <base/Context.h>
#include <base/WinApp.h>
#include <render/RenderEngine.h>

namespace EditorWorker
{
using namespace RenderWorker;

EditorManagerD3D11::EditorManagerD3D11()
{
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    
    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();

    //设置平台/渲染器后端
    ImGui_ImplWin32_Init(Context::Instance().AppInstance().GetHWND());
	auto d3d11_re = Context::Instance().RenderEngineInstance();
    auto re = static_cast<ID3D11Device*>(d3d11_re.GetD3DDevice());
    auto ctx = static_cast<ID3D11DeviceContext*>(d3d11_re.GetD3DDeviceImmContext());
    ImGui_ImplDX11_Init(re, ctx);
}

EditorManagerD3D11::~EditorManagerD3D11()
{
    ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void EditorManagerD3D11::Init()
{
    PanelList_.push_back( CommonWorker::MakeSharedPtr<EditorProjectPanel>() );
    PanelList_.push_back( CommonWorker::MakeSharedPtr<EditorMainBarPanel>() );
    PanelList_.push_back( CommonWorker::MakeSharedPtr<EditorHierarchyPanel>() );
    PanelList_.push_back( CommonWorker::MakeSharedPtr<EditorInspectorPanel>() );
    PanelList_.push_back( CommonWorker::MakeSharedPtr<EditorConsolePanel>() );
}

void EditorManagerD3D11::Render()
{
    for(auto panel : PanelList_)
    {
        if(panel)
        {
            panel->OnRender();
        }
    }
    ImGui::Render();
}

void EditorManagerD3D11::BeginRender()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void EditorManagerD3D11::EndRender()
{
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

}