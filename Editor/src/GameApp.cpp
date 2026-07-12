#include <editor/GameApp.h>

#include <base/App3D.h>
#include <base/InputFactory.h>
#include <base/UIManager.h>
#include <base/Window.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/Renderable.h>
#include <render/Mesh.h>
#include <world/World.h>
#include <game/GameContext.h>
#include <game/Model.h>
#include <common/Log.h>
#include <common/Profiler.h>
#include <common/Util.h>

#include <windows.h>

namespace
{
	enum
	{
		Exit,
		ToggleGm,
		GmSubmit,
	};

	RenderWorker::InputActionDefine actions[] =
	{
		RenderWorker::InputActionDefine(Exit, RenderWorker::KS_Escape),
		RenderWorker::InputActionDefine(ToggleGm, RenderWorker::KS_Grave),
		RenderWorker::InputActionDefine(GmSubmit, RenderWorker::KS_Enter),
	};

	LRESULT CALLBACK GmWndProc(HWND /*hWnd*/, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (GameContext::Instance().GmDebugWindowInstance().Visible())
		{
			RenderWorker::Context::Instance().UIManagerInstance().ProcessGmWin32Message(msg, wParam, lParam);
		}
		return -1;
	}
}

namespace EditorWorker
{
using namespace RenderWorker;
using namespace CommonWorker;

GameApp::GameApp(std::string_view scene_path)
	: App3D("Game App <DirectX 11>")
	, scene_path_(scene_path)
{
}

GameApp::~GameApp() = default;

void GameApp::SetScenePath(std::string_view scene_path)
{
	scene_path_ = scene_path;
}

void GameApp::OnCreate()
{
	GameContext::Instance().Startup();

	auto& context = Context::Instance();
	RenderFactory& rf = context.RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();

	InputEngine& input_engine = context.InputFactoryInstance().InputEngineInstance();
	InputActionMap action_map;
	action_map.AddActions(actions, actions + std::size(actions));
	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const& sender, InputAction const& action) {
			this->InputHandler(sender, action);
		});
	input_engine.ActionMap(action_map, input_handler);

	depth_texture_support_ = re.DeviceCaps().depth_texture_support;

	back_face_depth_fb_ = rf.MakeFrameBuffer();
	FrameBufferPtr screen_buffer = re.CurFrameBuffer();
	back_face_depth_fb_->Viewport()->Camera(screen_buffer->Viewport()->Camera());
	scene_.LoadScene(scene_path_);
	ApplySceneCamera();

	auto& ui = context.UIManagerInstance();
	ui.SetDimensions(static_cast<int>(re.CurFrameBuffer()->Width()), static_cast<int>(re.CurFrameBuffer()->Height()));
	if (!GameContext::Instance().GmDebugWindowInstance().Initialize())
	{
		LogError() << "GameApp: GM debug window failed to initialize." << std::endl;
	}

	if (MainWnd())
	{
		MainWnd()->BindMsgProc(GmWndProc);
	}
}

void GameApp::ApplySceneCamera()
{
	if (scene_.HasSceneCamera())
	{
		LookAt(scene_.CameraEye(), scene_.CameraLookAt(), scene_.CameraUp());
		Proj(scene_.CameraNear(), scene_.CameraFar());

		if (scene_.CameraFovDeg() > 0.0f)
		{
			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			FrameBuffer& fb = *re.CurFrameBuffer();
			ActiveCamera().ProjParams(
				scene_.CameraFovDeg() * 3.14159265358979323846f / 180.0f,
				static_cast<float>(fb.Width()) / fb.Height(),
				scene_.CameraNear(),
				scene_.CameraFar());
		}
	}
	else
	{
		LookAt(float3(-0.4f, 1.0f, 3.9f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
		Proj(0.1f, 200.0f);
	}

	scene_.SetupCameraController(ActiveCamera());
}

void GameApp::OnResize(uint32_t width, uint32_t height)
{
	App3D::OnResize(width, height);

	auto& rf = Context::Instance().RenderFactoryInstance();
	RebuildBackFaceDepthTarget(rf, rf.RenderEngineInstance().DeviceCaps(), width, height);
	Context::Instance().UIManagerInstance().SetDimensions(static_cast<int>(width), static_cast<int>(height));
}

void GameApp::OnDestroy()
{
	GameContext::Instance().Shutdown();
}

void GameApp::RebuildBackFaceDepthTarget(RenderFactory& rf, RenderDeviceCaps const& caps, uint32_t width, uint32_t height)
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

uint32_t GameApp::DoUpdate(uint32_t pass)
{
	auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
	{
		ZENGINE_ZONE("GameApp::Pass0_BackFaceDepth");
		re.BindFrameBuffer(back_face_depth_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 0.0f, 0);
		scene_.UpdateDetailedMeshes(ActiveCamera().EyePos(), true);
		return URV_NeedFlush;
	}

	case 1:
	{
		ZENGINE_ZONE("GameApp::Pass1_Main");
		re.BindFrameBuffer(FrameBufferPtr());
		{
			Color clear_clr(0.2f, 0.4f, 0.6f, 1.0f);
			if (Context::Instance().Config().graphics_cfg.gamma)
			{
				clear_clr.r() = 0.029f;
				clear_clr.g() = 0.133f;
				clear_clr.b() = 0.325f;
			}
			re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);
		}
		scene_.UpdateDetailedMeshes(ActiveCamera().EyePos(), false);
		// Flush scene first; UI is drawn in pass 2 so it is not overwritten by 3D.
		return URV_NeedFlush;
	}

	case 2:
	{
		ZENGINE_ZONE("GameApp::Pass2_RmlUI");
		Context::Instance().UIManagerInstance().RenderIntoGameView();
		return URV_Finished;
	}

	default:
		COMMON_ASSERT(false);
		return URV_Finished;
	}
}

void GameApp::InputHandler(InputEngine const& /*sender*/, InputAction const& action)
{
	auto const* kb = dynamic_cast<InputKeyboardActionParam const*>(action.second.get());
	if (!kb)
	{
		return;
	}

	if (action.first == Exit && kb->buttons_down.test(KS_Escape))
	{
		Quit();
	}
	else if (action.first == ToggleGm && kb->buttons_down.test(KS_Grave))
	{
		GameContext::Instance().GmDebugWindowInstance().ToggleVisible();
	}
	else if (action.first == GmSubmit && kb->buttons_down.test(KS_Enter))
	{
		if (GameContext::Instance().GmDebugWindowInstance().Visible())
		{
			GameContext::Instance().GmDebugWindowInstance().Submit();
		}
	}
}

} // namespace EditorWorker
