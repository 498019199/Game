#include <editor/EditorManagerD3D11.h>
#include <editor/EditorConsolePanel.h>
#include <editor/EditorHierarchyPanel.h>
#include <editor/EditorMainBarPanel.h>
#include <editor/EditorInspectorPanel.h>
#include <editor/EditorDialogBoxManager.h>


#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include <base/App3D.h>
#include <base/Window.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

namespace EditorWorker
{
using namespace RenderWorker;

EditorManagerD3D11::EditorManagerD3D11()
    :App3D("Editor App <DirectX 11>")
{
}

EditorManagerD3D11::~EditorManagerD3D11()
{

}

void EditorManagerD3D11::OnCreate()
{
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();

    //设置平台/渲染器后端
    ImGui_ImplWin32_Init(Context::Instance().AppInstance().MainWnd()->GetHWND()); 
    auto& rf = Context::Instance().RenderFactoryInstance();
	auto& d3d11_re = rf.RenderEngineInstance();
    auto re = static_cast<ID3D11Device*>(d3d11_re.GetD3DDevice());
    auto ctx = static_cast<ID3D11DeviceContext*>(d3d11_re.GetD3DDeviceImmContext());
    ImGui_ImplDX11_Init(re, ctx);
    
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorProjectPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorMainBarPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorHierarchyPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorInspectorPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorConsolePanel>() );
}

void EditorManagerD3D11::OnDestroy()
{
    ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

uint32_t EditorManagerD3D11::DoUpdate(uint32_t pass)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    for(auto panel : panel_list_)
    {
        if(panel)
        {
            panel->OnRender(setting_);
        }
    }

    //EditorDialogBoxManager::Instance().OnRender();
    ImGui::Render();

    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return App3D::URV_NeedFlush | App3D::URV_Finished;
}

void EditorManagerD3D11::SetWindowSize(int hWidth, int pHeight, int iWidth)
{
    auto cfg = Context::Instance().Config();
    int Width = cfg.graphics_cfg.width;
    int Height = cfg.graphics_cfg.height;

    setting_.hierarchyWidth = hWidth;
    setting_.hierarchyHeight = Height;
    setting_.consoleWidth = (Width + setting_.hierarchyWidth) / 3;
    setting_.consoleHeight = pHeight;
    setting_.projectWidth = Width + setting_.hierarchyWidth - setting_.consoleWidth;
    setting_.projectHeight = pHeight;
    setting_.inspectorWidth = iWidth;
    setting_.inspectorHeight = Height + setting_.projectHeight;
    setting_.mainBarWidth = Width + setting_.hierarchyWidth + setting_.inspectorWidth;
    setting_.mainBarHeight = 58;
}

}