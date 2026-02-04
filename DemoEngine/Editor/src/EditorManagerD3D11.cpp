#include <editor/EditorManagerD3D11.h>
#include <editor/EditorConsolePanel.h>
#include <editor/EditorHierarchyPanel.h>
#include <editor/EditorMainBarPanel.h>
#include <editor/EditorInspectorPanel.h>
#include <editor/EditorGameViewPanel.h>
#include <editor/EditorDialogBoxManager.h>


#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include <base/App3D.h>
#include <base/Window.h>
#include <common/ResIdentifier.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

namespace EditorWorker
{
using namespace RenderWorker;

void EditorSetting::SetWindowSize(uint32_t _srcWidth, uint32_t _srcHeight, int hWidth, int pHeight, int iWidth)
{
    gameViewWidth = _srcWidth;
    gameViewHeight = _srcHeight;

    hierarchyWidth = hWidth;
    hierarchyHeight = gameViewHeight;
    consoleWidth = (gameViewWidth + hierarchyWidth) / 3;
    consoleHeight = pHeight;
    projectWidth = gameViewWidth + hierarchyWidth - consoleWidth;
    projectHeight = pHeight;
    inspectorWidth = iWidth;
    inspectorHeight = gameViewHeight + projectHeight;
    mainBarWidth = gameViewWidth + hierarchyWidth + inspectorWidth;
    mainBarHeight = 58;
    srcWidth = mainBarWidth;
    srcHeight = inspectorHeight + mainBarHeight;
}

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
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorGameViewPanel>() );
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

void EditorManagerD3D11::SetSelectedAssert(const EditorAssetNodePtr pAssert)
{
    if(selected_asset_ptr_ == pAssert.get())
    {
        return;
    }

    selected_asset_info_.reset();
    selected_asset_info_ = nullptr;
    selected_asset_ptr_ = pAssert.get();
    switch(pAssert->type)
    {
    case AssetType::Script:
    case AssetType::Text:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetScriptInfo>();
            ptr->name = pAssert->name;
            ptr->preview = LoadTextFile( pAssert->path );
            selected_asset_info_ = ptr;
        }
        break;
    case AssetType::Shader:
    case AssetType::RayTracingShader:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetShaderInfo>();
            ptr->name = pAssert->name;
            ptr->preview = LoadTextFile( pAssert->path );
            selected_asset_info_ = ptr;
        }
        break;

    case AssetType::Texture:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetTextureInfo>();
            ptr->name = pAssert->name;
            ptr->format = pAssert->extension;
            ptr->texture = SyncLoadTexture(pAssert->path,  EAH_GPU_Read | EAH_Immutable);
            selected_asset_info_ = ptr;
        }
        break;
    
    case AssetType::Material:
    case AssetType::RayTracingMaterial:
    case AssetType::DeferredMaterial:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetMaterialInfo>();
            ptr->name = pAssert->name;
            selected_asset_info_ = ptr;
        }
        break;

    case AssetType::Model:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetMaterialInfo>();
            ptr->name = pAssert->name;
            ptr->name = pAssert->extension;
            selected_asset_info_ = ptr;

            auto model = SyncLoadModel(pAssert->path , EAH_GPU_Read | EAH_Immutable,
			    SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
        }
        break;

    case AssetType::Audio:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetAudioInfo>();
            ptr->name = pAssert->name + pAssert->extension;

            auto& context = Context::Instance();
            AudioDataSourceFactory& adsf = context.AudioDataSourceFactoryInstance();
            auto& res_loader = context.ResLoaderInstance();
            ptr->audio_buff_ = adsf.MakeAudioDataSource();
            ptr->audio_buff_->Open( res_loader.Open( pAssert->path ) );

            selected_asset_info_ = ptr;
        }
        break;
    }
}

AssetType EditorManagerD3D11::GetAssertType() const
{
    if (selected_asset_ptr_) 
    {
        return selected_asset_ptr_->type;
    }
    return AssetType::Other;
}

std::string EditorManagerD3D11::LoadTextFile(const std::string_view& path)
{
    auto& res_loader = Context::Instance().ResLoaderInstance();
    ResIdentifierPtr source = res_loader.Open( path );
    if (!source)
    {
        return "";
    }

    source->seekg(0, std::ios_base::end);
    size_t const len = static_cast<size_t>(source->tellg());
    source->seekg(0, std::ios_base::beg);
    auto text_src = MakeUniquePtr<char[]>(len + 1);
    source->read(&text_src[0], len);
    text_src[len] = 0;
    return std::string(&text_src[0]);
}

}