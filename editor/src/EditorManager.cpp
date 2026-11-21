#include <editor/EditorManager.h>
#include <ImGuizmo.h>
#include <editor/EditorGizmoMath.h>
#include <editor/EditorMaterialPreview.h>
#include <cstdio>
#include <optional>
#include <editor/EditorConsolePanel.h>
#include <editor/EditorHierarchyPanel.h>
#include <editor/EditorMainBarPanel.h>
#include <editor/EditorInspectorPanel.h>
#include <editor/EditorGameViewPanel.h>
#include <editor/EditorDialogBoxManager.h>


#include <imgui.h>

#include <base/App3D.h>
#include <base/Window.h>
#include <world/World.h>
#include <common/ResIdentifier.h>
#include <common/Profiler.h>
#include <common/Util.h>
#include <common/Log.h>
#include <render/RenderEngine.h>
#include <base/InputFactory.h>
#include <base/UIManager.h>
#include <render/RenderFactory.h>
#include <game/GameContext.h>
#include <game/Model.h>
#include <common/ErrorHandling.h>

namespace
{
	enum
	{
		Exit,
		ToggleGm,
		GmSubmit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};
}

namespace EditorWorker
{
using namespace RenderWorker;

EditorManager::EditorManager()
    : App3D("Editor App")
    , scene_path_("scenes/DefaultScene.scene")
{
}

EditorManager::~EditorManager()
{

}

void EditorManager::OnCreate()
{
    GameContext::Instance().Startup();
    materials_ = std::make_unique<EditorMaterialService>(
        std::filesystem::path(Context::Instance().ResLoaderInstance().LocalFolder()) / "../../assets",
        [](std::filesystem::path const& path) {
            auto tex = SyncLoadTexture(MaterialUtf8(path), EAH_GPU_Read | EAH_Immutable);
            if (!tex) TMSG("Texture load failed: " + MaterialUtf8(path));
            return Context::Instance().RenderFactoryInstance().MakeTextureSrv(tex);
        });

    auto& context = Context::Instance();
	RenderFactory& rf = context.RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
#ifndef EDITOR_DEBUG_MODE
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();

    // 高 DPI / 4K：默认字体与样式按未缩放像素设计，需与 HWND 的 DPI 一致放大
#ifdef ZENGINE_PLATFORM_DARWIN
    // Cocoa and ImGui use logical points on macOS; the backend already applies the
    // Retina framebuffer scale. Scaling the layout again clips whole panels.
    float const imgui_dpi_scale = 1.0f;
#else
    float const imgui_dpi_scale = Context::Instance().AppInstance().MainWnd()->DPIScale();
#endif
    ImGui::GetStyle().ScaleAllSizes(imgui_dpi_scale);
    io.FontGlobalScale = imgui_dpi_scale;

    InitializeImGui();
    setting_.ApplyDpiScale(imgui_dpi_scale);

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
    action_handler_t input_handler = CommonWorker::MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	const RenderDeviceCaps& caps = re.DeviceCaps();
    depth_texture_support_ = caps.depth_texture_support;

    back_face_depth_fb_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = rf.RenderEngineInstance().CurFrameBuffer();
	back_face_depth_fb_->Viewport()->Camera(screen_buffer->Viewport()->Camera());

    scene_.LoadScene(scene_path_);
    ApplySceneCamera();

    auto& ui = Context::Instance().UIManagerInstance();
    ui.SetDimensions(static_cast<int>(setting_.gameViewWidth), static_cast<int>(setting_.gameViewHeight));
    if (!GameContext::Instance().GmDebugWindowInstance().Initialize())
    {
        LogError() << "Editor: GM debug window failed to initialize." << std::endl;
    }
}

void EditorManager::OnResize(uint32_t width, uint32_t height)
{
    if (!width || !height) return;
    // BeginFrame runs outside World::Update's traversal lock. Protect the mesh
    // depth bindings and shared camera while the scene update thread is active.
    std::lock_guard<std::mutex> lock(Context::Instance().WorldInstance().MutexForUpdate());
#ifdef ZENGINE_PLATFORM_DARWIN
    if (MainWnd()->Width() && MainWnd()->Height())
    {
        width = MainWnd()->Width();
        height = MainWnd()->Height();
    }
#endif
    setting_.ResizeClient(width, height);

    auto& rf = Context::Instance().RenderFactoryInstance();
    auto& re = rf.RenderEngineInstance();
    RenderDeviceCaps const& caps = re.DeviceCaps();
    RebuildBackFaceDepthTarget(rf, caps, setting_.gameViewWidth, setting_.gameViewHeight);
    RebuildGameViewRenderTarget(rf, caps);
    auto& camera = *re.ScreenFrameBuffer()->Viewport()->Camera();
    camera.ProjParams(re.DefaultFOV(), static_cast<float>(setting_.gameViewWidth) / setting_.gameViewHeight,
        camera.NearPlane(), camera.FarPlane());
    for (auto const& panel : panel_list_) panel->OnResize();
}

void EditorManager::ApplySceneCamera()
{
    if (auto const& saved = scene_.SceneCamera(); saved && !scene_.ParticleEffects().empty()) {
        // Copy the stored shot into the editor camera, leaving scene state and
        // its dirty baseline untouched. Use the actual preview aspect ratio.
        LookAt(saved->EyePos(), saved->EyePos() + saved->ForwardVec(), saved->UpVec());
        float const aspect = setting_.gameViewWidth > 0 && setting_.gameViewHeight > 0
            ? static_cast<float>(setting_.gameViewWidth) / setting_.gameViewHeight : saved->Aspect();
        ActiveCamera().ProjParams(saved->FOV(), aspect, saved->NearPlane(), saved->FarPlane());
    } else {
        LookAt(float3(-0.4f, 1.0f, 3.9f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
        Proj(0.1f, 200.0f);
    }
    
    scene_.SetupCameraController(ActiveCamera());
    scene_.SetCameraControllerInputEnabled(game_view_input_active_);
}

void EditorManager::RebuildBackFaceDepthTarget(RenderFactory& rf, RenderDeviceCaps const& caps, uint32_t width, uint32_t height)
{
    TexturePtr back_face_depth_tex;
    DepthStencilViewPtr back_face_ds_view;
    ElementFormat fmt;

    if (depth_texture_support_)
    {
        fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({ EF_ABGR8, EF_ARGB8 }), 1, 0);
        COMMON_ASSERT(fmt != EF_Unknown);
        back_face_depth_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);

        fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({ EF_D24S8, EF_D16 }), 1, 0);
        COMMON_ASSERT(fmt != EF_Unknown);

        float4 constexpr back_face_ds_clear_value(0, 0, 0, 0);
        TexturePtr back_face_ds_tex = rf.MakeTexture2D(
            width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, {}, &back_face_ds_clear_value);
        back_face_ds_view = rf.Make2DDsv(back_face_ds_tex, 0, 1, 0);

        Context::Instance().WorldInstance().SceneRootNode().Traverse([&back_face_ds_tex](SceneNode& node) {
            node.ForEachComponentOfType<RenderableComponent>([back_face_ds_tex](RenderableComponent& comp) {
                if (auto* detailed_mesh = dynamic_cast<DetailedMesh*>(&comp.BoundRenderable()))
                {
                    detailed_mesh->BackFaceDepthTex(back_face_ds_tex);
                }
            });
            return true;
        });
    }
    else
    {
        if (caps.pack_to_rgba_required)
        {
            fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({ EF_ABGR8, EF_ARGB8 }), 1, 0);
            COMMON_ASSERT(fmt != EF_Unknown);
        }
        else
        {
            fmt = EF_R16F;
        }
        back_face_depth_tex = rf.MakeTexture2D(width, height, 1, 1, fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
        back_face_ds_view = rf.Make2DDsv(width, height, EF_D16, 1, 0);

        Context::Instance().WorldInstance().SceneRootNode().Traverse([&back_face_depth_tex](SceneNode& node) {
            node.ForEachComponentOfType<RenderableComponent>([back_face_depth_tex](RenderableComponent& comp) {
                if (auto* detailed_mesh = dynamic_cast<DetailedMesh*>(&comp.BoundRenderable()))
                {
                    detailed_mesh->BackFaceDepthTex(back_face_depth_tex);
                }
            });
            return true;
        });
    }

    back_face_depth_fb_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(back_face_depth_tex, 0, 1, 0));
    back_face_depth_fb_->Attach(back_face_ds_view);
}

void EditorManager::RebuildGameViewRenderTarget(RenderFactory& rf, RenderDeviceCaps const& caps)
{
	uint32_t const gw = (std::max)(1, setting_.gameViewWidth);
	uint32_t const gh = (std::max)(1, setting_.gameViewHeight);

	ElementFormat color_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({ EF_ABGR8, EF_ARGB8 }), 1, 0);
	COMMON_ASSERT(color_fmt != EF_Unknown);

	game_view_color_tex_ = rf.MakeTexture2D(gw, gh, 1, 1, color_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write);
	RenderWorker::RenderTargetViewPtr color_rtv = rf.Make2DRtv(game_view_color_tex_, 0, 1, 0);

	RenderWorker::DepthStencilViewPtr game_dsv;
	if (depth_texture_support_)
	{
		ElementFormat ds_fmt = caps.BestMatchTextureRenderTargetFormat(MakeSpan({ EF_D24S8, EF_D16 }), 1, 0);
		COMMON_ASSERT(ds_fmt != EF_Unknown);
		float4 constexpr ds_clear(0, 0, 0, 0);
		TexturePtr ds_tex = rf.MakeTexture2D(gw, gh, 1, 1, ds_fmt, 1, 0, EAH_GPU_Read | EAH_GPU_Write, {}, &ds_clear);
		game_dsv = rf.Make2DDsv(ds_tex, 0, 1, 0);
	}
	else
	{
		game_dsv = rf.Make2DDsv(gw, gh, EF_D16, 1, 0);
	}

	game_view_fb_ = rf.MakeFrameBuffer();
	game_view_fb_->Attach(FrameBuffer::Attachment::Color0, color_rtv);
	game_view_fb_->Attach(game_dsv);

	FrameBufferPtr screen_buffer = rf.RenderEngineInstance().ScreenFrameBuffer();
	game_view_fb_->Viewport()->Camera(screen_buffer->Viewport()->Camera());

	game_view_srv_ = rf.MakeTextureSrv(game_view_color_tex_);

	Context::Instance().UIManagerInstance().SetDimensions(static_cast<int>(gw), static_cast<int>(gh));
}

void* EditorManager::GameViewShaderResourceView() const
{
#ifndef EDITOR_DEBUG_MODE
	if (game_view_srv_)
	{
		return GameViewTextureHandle();
	}
#endif
	return nullptr;
}

void* EditorManager::ImGuiTextureHandle(ShaderResourceViewPtr const& srv) const
{
    return srv ? TextureHandle(srv.get()) : nullptr;
}

void EditorManager::GameViewInputActive(bool active)
{
	if (game_view_input_active_ == active)
	{
		return;
	}

	game_view_input_active_ = active;
	scene_.SetCameraControllerInputEnabled(active);
}

void EditorManager::SetCameraMoveBoost(bool boost)
{
	scene_.SetCameraControllerMoveBoost(boost);
}

void EditorManager::DoUpdateOverlay()
{
}


void EditorManager::OnDestroy()
{
    selected_asset_info_.reset();
    material_preview_.reset(); preview_material_.reset(); preview_document_.reset(); synced_materials_.clear();
    materials_.reset();
#ifndef EDITOR_DEBUG_MODE
	ShutdownImGui();
	ImGui::DestroyContext();
#endif //EDITOR_DEBUG_MODE

    GameContext::Instance().Shutdown();
}

uint32_t EditorManager::DoUpdate(uint32_t pass)
{
    auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	auto const editor_screen_pass = [&re, this]() -> uint32_t {
		ZENGINE_ZONE("Editor::Pass2_UI");
        if (preview_document_ && preview_material_) {
            try {
                if (!material_preview_) material_preview_ = std::make_unique<EditorMaterialPreview>();
                material_preview_->Render(preview_material_, preview_shape_, preview_yaw_, preview_distance_);
                preview_error_.clear();
            } catch (std::exception const& error) { preview_error_ = error.what(); }
        }
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
		return App3D::URV_Finished;
	};

    switch (pass)
    {
        case 0:
        {
            UpdateMaterialViews();
			ZENGINE_ZONE("Editor::Pass0_BackFaceDepth");
            re.BindFrameBuffer(back_face_depth_fb_);
            re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 0.0f, 0);
            scene_.UpdateDetailedMeshes(ActiveCamera().EyePos(), true);
        }
            return App3D::URV_NeedFlush;
        case 1:
        {
			ZENGINE_ZONE("Editor::Pass1_GameView");
            re.BindFrameBuffer(game_view_fb_);
            Color clear_clr(0.2f, 0.4f, 0.6f, 1);
            if (Context::Instance().Config().graphics_cfg.gamma)
            {
                clear_clr.r() = 0.029f;
                clear_clr.g() = 0.133f;
                clear_clr.b() = 0.325f;
            }
            re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);
            scene_.UpdateDetailedMeshes(ActiveCamera().EyePos(), false);
            // Flush scene into game_view_fb first; Rml UI is drawn in pass 2 on top.
            return App3D::URV_NeedFlush;
        }
        case 2:
        {
			ZENGINE_ZONE("Editor::Pass2_RmlUI");
			// Still bound to game_view_fb_ after pass 1 flush.
			Context::Instance().UIManagerInstance().RenderIntoGameView();
			return editor_screen_pass();
        }
        default:
            COMMON_ASSERT(false);
            return 0;
    }
}

void EditorManager::RenderEditorPanels()
{
#ifndef EDITOR_DEBUG_MODE
	NewImGuiFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
	Context::Instance().UIManagerInstance().SetInputViewport(false, false, 0, 0, 1, 1);

    for(auto panel : panel_list_)
    {
        if(panel)
        {
            panel->OnRender(setting_);
        }
    }

    DrawSceneDialog();
    if (exit_pending_) ImGui::OpenPopup("Unsaved changes - Exit");
    if (ImGui::BeginPopupModal("Unsaved changes - Exit", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Save modified materials and scene before exiting?");
        if (scene_.Dirty()) ImGui::BulletText("Scene: %s", scene_path_.c_str());
        for (auto const& [path, doc] : materials_->Documents())
            if (doc->Dirty()) ImGui::BulletText("%s", path.c_str());
        if (!exit_error_.empty()) ImGui::TextWrapped("%s", exit_error_.c_str());
        auto finish = [&](MaterialCloseChoice choice) {
            try {
                if (materials_->ResolveExit(choice)) {
                    if (choice == MaterialCloseChoice::Save && scene_.Dirty()) SaveSceneFile(scene_path_);
                    exit_approved_ = true; Quit();
                }
                exit_pending_ = false; exit_error_.clear(); ImGui::CloseCurrentPopup();
            } catch (std::exception const& e) { exit_error_ = e.what(); }
        };
        if (ImGui::Button("Save All")) finish(MaterialCloseChoice::Save);
        ImGui::SameLine(); if (ImGui::Button("Discard All")) finish(MaterialCloseChoice::Discard);
        ImGui::SameLine(); if (ImGui::Button("Cancel")) finish(MaterialCloseChoice::Cancel);
        ImGui::EndPopup();
    }

    EditorDialogBoxManager::Instance().OnRender();
	Context::Instance().UIManagerInstance().RouteInput(true,
		ImGui::GetIO().WantTextInput || ImGui::IsAnyItemActive() ||
		ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel));
    ImGui::Render();

	RenderImGuiDrawData(ImGui::GetDrawData());
#endif // EDITOR_DEBUG_MODE
}

void EditorManager::InputHandler(RenderWorker::InputEngine const & sender, RenderWorker::InputAction const & action)
{
	KFL_UNUSED(sender);
	auto const* kb = dynamic_cast<InputKeyboardActionParam const*>(action.second.get());
	if (!kb)
	{
		return;
	}

	switch (action.first)
	{
	case Exit:
		if (kb->buttons_down.test(KS_Escape) && !scene_view_active_)
		{
			this->Quit();
		}
		break;
	case ToggleGm:
		if (kb->buttons_down.test(KS_Grave))
		{
			GameContext::Instance().GmDebugWindowInstance().ToggleVisible();
		}
		break;
	case GmSubmit:
		if (kb->buttons_down.test(KS_Enter) && GameContext::Instance().GmDebugWindowInstance().Visible())
		{
			GameContext::Instance().GmDebugWindowInstance().Submit();
		}
		break;
	}
}

void EditorManager::SetSelectedAssert(const EditorAssetNodePtr pAssert)
{
    if(selected_asset_ptr_ == pAssert)
    {
        return;
    }

    selected_asset_info_.reset();
    selected_asset_info_ = nullptr;
    selected_scene_node_ = nullptr;
    selected_mesh_name_.clear();
    selected_asset_ptr_ = pAssert;
    selected_asset_type_ = pAssert->type;
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
            try {
                ptr->path = MaterialUtf8(std::filesystem::relative(MaterialPath(pAssert->path),
                    std::filesystem::path(Context::Instance().ResLoaderInstance().LocalFolder()) / "../../assets"));
                ptr->document = Materials().Open(ptr->path);
                PreviewMaterial(ptr->document);
            }
            catch (std::exception const& e) { ptr->error = e.what(); }
            selected_asset_info_ = ptr;
        }
        break;

    case AssetType::Model:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetModelInfo>();
            ptr->name = pAssert->name;
            ptr->format = pAssert->extension;
            selected_asset_info_ = ptr;

            ptr->model = SyncLoadModel(pAssert->path , EAH_GPU_Read | EAH_Immutable,
			    SceneNode::SOA_Cullable, 
                [](RenderModel& model)
                {
                    model.RootNode()->TransformToParent(MathWorker::translation(0.0f, 0.0f, 0.0f));
                    AddToSceneRootHelper(model);
                }, CreateModelFactory<RenderModel>, CreateDetailedMesh);
        }
        break;

    case AssetType::Audio:
        {
            auto ptr = CommonWorker::MakeSharedPtr<AssetAudioInfo>();
            ptr->name = pAssert->name + pAssert->extension;

            try
            {
                auto& context = Context::Instance();
                auto file = context.ResLoaderInstance().Open(pAssert->path);
                if (!file)
                    TMSG("Audio resource not found: " + pAssert->path);
                auto source = context.AudioDataSourceFactoryInstance().MakeAudioDataSource();
                if (!source)
                    TMSG("Audio decoder creation failed");
                source->Open(file);
                ptr->decoded_size = source->Size();
                ptr->audio_buff_ = std::move(source);
            }
            catch (std::exception const& e)
            {
                ptr->error = e.what();
            }

            selected_asset_info_ = ptr;
        }
        break;

    default:
        break;
    }
}

void EditorManager::ClearAssetSelection()
{
    selected_asset_ptr_.reset(); selected_asset_info_.reset(); selected_asset_type_ = AssetType::Other;
    selected_scene_node_ = nullptr; selected_mesh_name_.clear();
}

void EditorManager::SelectMaterialDocument(MaterialDocumentPtr doc)
{
    ClearAssetSelection();
    auto info = std::make_shared<AssetMaterialInfo>();
    PreviewMaterial(doc);
    info->document = std::move(doc); info->path = info->document->Path(); info->name = info->document->Material().Name();
    selected_asset_type_ = AssetType::Material; selected_asset_info_ = std::move(info);
}

bool EditorManager::CloseMaterialDocument(MaterialDocumentPtr const& doc, MaterialCloseChoice choice)
{
    if (!materials_->Close(doc, choice)) return false;
    // Discard changes the document version after it has left the open-document
    // list. Publish its restored value before dropping the last editor reference.
    scene_.RefreshMaterial(doc->Path(), doc->Material().Clone());
    synced_materials_.erase(doc->Path());
    if (preview_document_ == doc) {
        preview_document_.reset();
        preview_focus_ = false;
        preview_error_.clear();
    }
    ClearAssetSelection();
    return true;
}

bool EditorManager::OnCloseRequested()
{
    if (exit_approved_ || !materials_ || (!materials_->HasDirty() && !scene_.Dirty())) return true;
    exit_pending_ = true;
    return false;
}

void EditorManager::SetSelectedSceneNode(SceneNode const* node, std::string_view mesh_name)
{
    if (selected_scene_node_ == node && selected_mesh_name_ == mesh_name)
    {
        return;
    }

    selected_asset_ptr_ = nullptr;
    selected_scene_node_ = node;
    scene_focus_ = node != nullptr;
    selected_mesh_name_ = mesh_name;
    selected_asset_info_.reset();

    if (!node)
    {
        selected_asset_type_ = AssetType::Other;
        return;
    }

    RenderModelPtr model = scene_.FindModelForNode(*node);
    if (!model)
    {
        selected_asset_type_ = AssetType::Other;
        return;
    }

    auto ptr = CommonWorker::MakeSharedPtr<AssetModelInfo>();
    CommonWorker::Convert(ptr->name, model->RootNode()->Name());
    if (!mesh_name.empty())
    {
        ptr->name += " / ";
        ptr->name.append(mesh_name.data(), mesh_name.size());
    }
    ptr->model = model;
    selected_asset_info_ = ptr;
    selected_asset_type_ = AssetType::Model;
}

bool EditorManager::IsHierarchyItemSelected(SceneNode const* node, std::string_view mesh_name) const
{
    if (!node || selected_scene_node_ != node)
    {
        return false;
    }
    return selected_mesh_name_ == mesh_name;
}

AssetType EditorManager::GetAssertType() const
{
    return selected_asset_type_;
}

std::string EditorManager::LoadTextFile(const std::string_view& path)
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

namespace EditorWorker
{
void EditorManager::PreviewMaterial(MaterialDocumentPtr const& doc)
{
    if (preview_document_ != doc) { preview_document_ = doc; preview_version_ = ~uint64_t(0); preview_focus_ = true; }
}
bool EditorManager::ConsumePreviewFocus() { bool focus = preview_focus_; preview_focus_ = false; return focus; }
void EditorManager::UpdateMaterialViews()
{
    // The previous frame may already contain an ImGui::Image of the preview.
    // Release its GPU resources at the next frame boundary, not inside Close.
    if (!preview_document_) {
        material_preview_.reset();
        preview_material_.reset();
        preview_version_ = ~uint64_t(0);
    }
    for (auto const& [path, doc] : materials_->Documents()) {
        auto found = synced_materials_.find(path);
        if (found == synced_materials_.end() || found->second.document.lock() != doc || found->second.version != doc->Version()) {
            scene_.RefreshMaterial(path, doc->Material().Clone());
            synced_materials_[path] = {doc, doc->Version()};
        }
    }
    if (preview_document_ && preview_version_ != preview_document_->Version()) {
        preview_material_ = preview_document_->Material().Clone(); preview_version_ = preview_document_->Version();
    }
}
void EditorManager::DrawMaterialPreview()
{
    if (!preview_document_) { ImGui::TextUnformatted("Open or create a material in Project to preview it."); return; }
    ImGui::TextWrapped("%s%s", preview_document_->Path().c_str(), preview_document_->Dirty() ? " * (unsaved)" : "");
    ImGui::Combo("Shape", &preview_shape_, "Sphere\0Cube\0Plane\0");
    ImGui::SliderFloat("Rotation", &preview_yaw_, -3.14159f, 3.14159f);
    ImGui::SliderFloat("Distance", &preview_distance_, 2.0f, 6.0f);
    if (!preview_error_.empty()) ImGui::TextWrapped("Preview: %s", preview_error_.c_str());
    if (material_preview_) if (auto texture = ImGuiTextureHandle(material_preview_->Image())) {
        auto available = ImGui::GetContentRegionAvail();
        float size = (std::max)(1.0f, (std::min)(available.x, available.y));
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(size,size));
    }
}
void EditorManager::RequestSceneDialog(bool save)
{
    scene_dialog_ = save ? 2 : 1; scene_dialog_pending_ = true; discard_scene_ = false; scene_dialog_error_.clear();
    std::snprintf(scene_dialog_path_, sizeof(scene_dialog_path_), "%s", scene_path_.c_str());
}
void EditorManager::SaveSceneFile(std::string const& path)
{
    auto file = materials_->Resolve(path);
    if (file.extension() != ".scene") throw std::runtime_error("Expected an Assets-relative .scene file");
    for (auto const& [name, doc] : materials_->Documents()) { (void)name; if (doc->Dirty()) materials_->Save(*doc); }
    scene_.SaveScene(file); scene_path_ = materials_->Identity(path);
    scene_status_ = "Saved " + scene_path_; assets_refresh_requested = true;
}
void EditorManager::SaveCurrentScene()
{
    try { SaveSceneFile(scene_path_); }
    catch (std::exception const& error) { scene_status_ = error.what(); }
}
void EditorManager::DrawSceneDialog()
{
    if (scene_dialog_pending_) { ImGui::OpenPopup("Scene file"); scene_dialog_pending_ = false; }
    if (ImGui::BeginPopupModal("Scene file", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        bool save = scene_dialog_ == 2;
        ImGui::TextUnformatted(save ? "Save materials and scene" : "Open scene");
        ImGui::TextUnformatted("Assets-relative path (.scene)");
        ImGui::SetNextItemWidth(440); ImGui::InputText("Path", scene_dialog_path_, sizeof(scene_dialog_path_));
        if (!save && scene_.Dirty()) ImGui::Checkbox("Discard current scene changes", &discard_scene_);
        if (!scene_dialog_error_.empty()) ImGui::TextWrapped("%s", scene_dialog_error_.c_str());
        ImGui::BeginDisabled(!save && scene_.Dirty() && !discard_scene_);
        if (ImGui::Button(save ? "Save" : "Open")) {
            try {
                if (save) SaveSceneFile(scene_dialog_path_);
                else {
                    auto file = materials_->Resolve(scene_dialog_path_);
                    if (file.extension() != ".scene") throw std::runtime_error("Expected a .scene file");
                    // Structural/asset validation runs in LoadScene before replacing live nodes.
                    scene_.LoadScene(MaterialUtf8(file)); ClearAssetSelection(); ApplySceneCamera();
                    scene_path_ = materials_->Identity(scene_dialog_path_); synced_materials_.clear();
                    scene_status_ = "Opened " + scene_path_;
                }
                ImGui::CloseCurrentPopup();
            } catch (std::exception const& error) { scene_dialog_error_ = error.what(); }
        }
        ImGui::EndDisabled(); ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
}

namespace EditorWorker
{
SceneNodePtr EditorManager::SelectedTransformNode() const
{
    if (!selected_scene_node_) return {};
    if (selected_scene_node_ == scene_.SceneCameraNode().get()) return scene_.SceneCameraNode();
    for (auto const& effect : scene_.ParticleEffects()) if (effect.node.get() == selected_scene_node_) return effect.node;
    if (auto model = scene_.FindModelForNode(*selected_scene_node_)) return model->RootNode();
    return {};
}

void EditorManager::FrameSelection()
{
    if (auto node = SelectedTransformNode()) {
        if (node == scene_.SceneCameraNode()) {
            auto const& camera = *scene_.SceneCamera();
            auto corners = GizmoMath::PerspectiveCorners(camera.FOV(),camera.Aspect(),camera.NearPlane(),camera.FarPlane(),camera.InverseViewMatrix());
            auto low=camera.EyePos(), high=low;
            for (auto const& corner:corners) for(size_t i=0;i<3;++i) {
                low[i]=(std::min)(low[i],corner[i]); high[i]=(std::max)(high[i],corner[i]);
            }
            auto center=(low+high)*.5f;
            float radius=(std::max)(1.f,MathWorker::length(high-low)*.5f);
            LookAt(center + float3(radius*1.8f, radius*1.2f, -radius*1.8f), center, float3(0,1,0));
            Proj(.01f, (std::max)(200.f, radius * 10));
            scene_.SetupCameraController(ActiveCamera());
            scene_.SetCameraControllerInputEnabled(false);
        } else if (auto particles = scene_.FindParticleEffect(node.get())) {
            node->UpdateTransforms();
            auto const& settings = particles->Settings();
            float const local_radius = (std::max)(1.f, settings.radius + settings.start_size + settings.speed_max * settings.lifetime_max * .5f);
            auto const& transform = node->TransformToWorld();
            float max_scale = 0;
            for (size_t row = 0; row < 3; ++row)
                max_scale = (std::max)(max_scale, MathWorker::length(float3(transform(row,0), transform(row,1), transform(row,2))));
            float const radius = (std::max)(.1f, local_radius * max_scale);
            auto center = MathWorker::transform_coord(float3(0, local_radius * .35f, 0), transform);
            LookAt(center + float3(radius * .6f, radius * .35f, radius * 2.7f), center, float3(0,1,0));
            Proj(.01f, (std::max)(200.f, radius * 30));
            scene_.SetupCameraController(ActiveCamera());
            scene_.SetCameraControllerInputEnabled(game_view_input_active_);
        } else FrameModel(scene_.FindModelForNode(*node));
        scene_focus_ = true;
    }
}

void EditorManager::CreateParticleEffect(ParticlePreset preset)
{
    try {
        wchar_t const* name = preset == ParticlePreset::Fire ? L"Fire" : preset == ParticlePreset::Smoke ? L"Smoke" :
            preset == ParticlePreset::Sparks ? L"Sparks" : preset == ParticlePreset::Explosion ? L"Explosion" : L"Heat haze";
        auto position = ActiveCamera().EyePos() + ActiveCamera().ForwardVec() * 6.f;
        auto node = scene_.AddParticleEffect(ParticleSettings::ForPreset(preset), name,
            MathWorker::translation(position.x(), position.y(), position.z()));
        SetSelectedSceneNode(node.get());
        FrameSelection();
        scene_status_ = "Particle effect created. Edit it in Inspector; save the scene to keep it.";
    } catch (std::exception const& error) { scene_status_ = error.what(); }
}

void EditorManager::FrameModel(RenderModelPtr const& model)
{
    if (!model) return;
    model->RootNode()->UpdateTransforms();
    std::optional<AABBox> bounds;
    model->RootNode()->Traverse([&](SceneNode& node) {
        node.ForEachComponentOfType<RenderableComponent>([&](RenderableComponent& component) {
            auto box = MathWorker::transform_aabb(component.BoundRenderable().PosBound(), node.TransformToWorld());
            if (bounds) *bounds |= box; else bounds = box;
        });
        return true;
    });
    if (!bounds) return;
    auto center = bounds->Center(); float radius = (std::max)(.1f, MathWorker::length(bounds->HalfSize()));
    LookAt(center + float3(radius*.6f, radius*.35f, radius*2.7f), center, float3(0,1,0));
    Proj((std::max)(.01f,radius/1000), (std::max)(200.f,radius*30));
    scene_.SetupCameraController(ActiveCamera());
    scene_.SetCameraControllerInputEnabled(game_view_input_active_);
}
}
