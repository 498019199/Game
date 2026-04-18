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
#include <world/World.h>
#include <common/ResIdentifier.h>
#include <render/RenderEngine.h>
#include <base/InputFactory.h>
#include <render/RenderFactory.h>
#include "Model.h"

namespace
{
	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}

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
    tbController_.AttachCamera(this->ActiveCamera());
	tbController_.Scalers(0.01f, 0.01f);
    this->LookAt(float3(-25.72f, 29.65f, 24.57f), float3(-24.93f, 29.09f, 24.32f));
	this->Proj(0.05f, 300.0f);

    auto& context = Context::Instance();
	RenderFactory& rf = context.RenderFactoryInstance();
	auto& d3d11_re = rf.RenderEngineInstance();
#ifndef EDITOR_DEBUG_MODE
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();

    //设置平台/渲染器后端
    ImGui_ImplWin32_Init(Context::Instance().AppInstance().MainWnd()->GetHWND()); 
    auto re = static_cast<ID3D11Device*>(d3d11_re.GetD3DDevice());
    auto ctx = static_cast<ID3D11DeviceContext*>(d3d11_re.GetD3DDeviceImmContext());
    ImGui_ImplDX11_Init(re, ctx);
    
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorProjectPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorMainBarPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorHierarchyPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorInspectorPanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorConsolePanel>() );
    panel_list_.push_back( CommonWorker::MakeSharedPtr<EditorGameViewPanel>() );
#endif // EDITOR_DEBUG_MODE

	InputEngine& inputEngine(context.InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));
    action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	const RenderDeviceCaps& caps = d3d11_re.DeviceCaps();
    depth_texture_support_ = caps.depth_texture_support;

    back_face_depth_fb_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = rf.RenderEngineInstance().CurFrameBuffer();
	back_face_depth_fb_->Viewport()->Camera(screen_buffer->Viewport()->Camera());

    light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1.5f, 1.5f, 1.5f));
	light_->Falloff(float3(1, 0.5f, 0.0f));

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(MathWorker::scaling(0.05f, 0.05f, 0.05f) * light_proxy->RootNode()->TransformToParent());

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	light_node->TransformToParent(MathWorker::translation(0.0f, 2.0f, -3.0f));
	light_node->AddComponent(light_);
	light_node->AddChild(light_proxy->RootNode());
	context.WorldInstance().SceneRootNode().AddChild(light_node);
}

void EditorManagerD3D11::OnResize(uint32_t width, uint32_t height)
{
    App3D::OnResize(width, height);

    auto& context = Context::Instance();
	RenderFactory& rf = context.RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();

	RenderWorker::TexturePtr back_face_depth_tex;
	RenderWorker::TexturePtr back_face_ds_tex;
	RenderWorker::DepthStencilViewPtr back_face_ds_view;
	ElementFormat fmt;
	if (depth_texture_support_)
	{
		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
		COMMON_ASSERT(fmt != EF_Unknown);

		// Just dummy
		back_face_depth_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

		fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_D24S8, EF_D16}), 1, 0);
		COMMON_ASSERT(fmt != EF_Unknown);

		float4 constexpr back_face_ds_clear_value(0, 0, 0, 0);
		back_face_ds_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, {}, &back_face_ds_clear_value);
		back_face_ds_view = rf.Make2DDsv(back_face_ds_tex, 0, 1, 0);

        if( model_ )
        {		
            model_->ForEachMesh([back_face_ds_tex](Renderable& mesh)
			    {
				    checked_cast<DetailedMesh&>(mesh).BackFaceDepthTex(back_face_ds_tex);
			    });
        }
	}
	else
	{
		if (caps.pack_to_rgba_required)
		{
			fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({EF_ABGR8, EF_ARGB8}), 1, 0);
			COMMON_ASSERT(fmt != EF_Unknown);
		}
		else
		{
			fmt = EF_R16F;
		}
		back_face_depth_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
		back_face_ds_view = rf.Make2DDsv(width, height, EF_D16, 1, 0);

        if( model_ )
        {
            model_->ForEachMesh([back_face_depth_tex](Renderable& mesh)
                {
                    checked_cast<DetailedMesh&>(mesh).BackFaceDepthTex(back_face_depth_tex);
                });
        }
	}

	back_face_depth_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(back_face_depth_tex, 0, 1, 0));
	back_face_depth_fb_->Attach(back_face_ds_view);
}

void EditorManagerD3D11::DoUpdateOverlay()
{
}


void EditorManagerD3D11::OnDestroy()
{
#ifndef EDITOR_DEBUG_MODE
    ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif //EDITOR_DEBUG_MODE
}

uint32_t EditorManagerD3D11::DoUpdate(uint32_t pass)
{
    if( nullptr == model_ )
    {
        RenderEditorPanels();
        return App3D::URV_NeedFlush | App3D::URV_Finished;
    }

    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
    switch (pass)
    {
        case 0:
        {
            re.BindFrameBuffer(back_face_depth_fb_);
            re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 0.0f, 0);

            if( model_ )
            {
                model_->ForEachMesh([](Renderable& mesh)
                    {
                        checked_cast<DetailedMesh&>(mesh).BackFaceDepthPass(true);
                    });
            }
        }
            return App3D::URV_NeedFlush;
        default:
        {
            re.BindFrameBuffer(FrameBufferPtr());
            Color clear_clr(0.2f, 0.4f, 0.6f, 1);
            if (Context::Instance().Config().graphics_cfg.gamma)
            {
                clear_clr.r() = 0.029f;
                clear_clr.g() = 0.133f;
                clear_clr.b() = 0.325f;
            }
            re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

            RenderEditorPanels();

            model_->ForEachMesh([this](Renderable& mesh)
                {
                    auto& detailed_mesh = checked_cast<DetailedMesh&>(mesh);

                    detailed_mesh.LightPos(light_->Position());
                    detailed_mesh.LightColor(light_->Color());
                    detailed_mesh.LightFalloff(light_->Falloff());
                    detailed_mesh.EyePos(this->ActiveCamera().EyePos());
                    detailed_mesh.BackFaceDepthPass(false);
                });

        }
            return App3D::URV_NeedFlush | App3D::URV_Finished;
    }
}

void EditorManagerD3D11::RenderEditorPanels() const
{
#ifndef EDITOR_DEBUG_MODE
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
#endif // EDITOR_DEBUG_MODE
}

void EditorManagerD3D11::InputHandler(RenderWorker::InputEngine const & sender, RenderWorker::InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
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

            model_ = SyncLoadModel(pAssert->path , EAH_GPU_Read | EAH_Immutable,
			    SceneNode::SOA_Cullable, 
                [](RenderModel& model)
                {
                    model.RootNode()->TransformToParent(MathWorker::translation(0.0f, 5.0f, 0.0f));
                    AddToSceneRootHelper(model);
                }, CreateModelFactory<RenderModel>, CreateMeshFactory<DetailedMesh>);
            
            this->LookAt(float3(-0.4f, 1, 3.9f), float3(0, 1, 0), float3(0.0f, 1.0f, 0.0f));
	        this->Proj(0.1f, 200.0f);;
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