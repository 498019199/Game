#include <editor/GameApp.h>

#include <base/App3D.h>
#include <base/InputFactory.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/Renderable.h>
#include <render/Light.h>
#include <render/Mesh.h>
#include <world/World.h>
#include <game/Model.h>

namespace
{
	enum
	{
		Exit,
	};

	RenderWorker::InputActionDefine actions[] =
	{
		RenderWorker::InputActionDefine(Exit, RenderWorker::KS_Escape),
	};
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
	LookAt(float3(-0.4f, 1.0f, 3.9f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));
	Proj(0.1f, 200.0f);

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

	camera_controller_ = MakeSharedPtr<TrackballCameraController>();
	camera_controller_->AttachCamera(ActiveCamera());

	auto& root_node = context.WorldInstance().SceneRootNode();

	auto ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->Color(float3(0.1f, 0.1f, 0.1f));
	root_node.AddComponent(ambient_light);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(1.5f, 1.5f, 1.5f));
	light_->Falloff(float3(1.0f, 0.5f, 0.0f));

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(
		MathWorker::scaling(0.05f, 0.05f, 0.05f) * light_proxy->RootNode()->TransformToParent());

	auto light_node = MakeSharedPtr<SceneNode>(L"LightNode", SceneNode::SOA_Cullable);
	light_node->TransformToParent(MathWorker::translation(0.0f, 2.0f, -3.0f));
	light_node->AddComponent(light_);
	light_node->AddChild(light_proxy->RootNode());
	root_node.AddChild(light_node);

	scene_.LoadScene(scene_path_);
}

void GameApp::OnResize(uint32_t width, uint32_t height)
{
	App3D::OnResize(width, height);

	auto& rf = Context::Instance().RenderFactoryInstance();
	RebuildBackFaceDepthTarget(rf, rf.RenderEngineInstance().DeviceCaps(), width, height);
}

void GameApp::OnDestroy()
{
	if (camera_controller_)
	{
		camera_controller_->DetachCamera();
		camera_controller_.reset();
	}
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

void GameApp::UpdateDetailedMeshes(bool back_face_depth_pass)
{
	float3 const eye_pos = ActiveCamera().EyePos();
	float3 const light_pos = light_->Position();
	float3 const light_color = light_->Color();
	float3 const light_falloff = light_->Falloff();

	Context::Instance().WorldInstance().SceneRootNode().Traverse([&](SceneNode& node) {
		node.ForEachComponentOfType<RenderableComponent>([&](RenderableComponent& comp) {
			if (auto* detailed_mesh = dynamic_cast<DetailedMesh*>(&comp.BoundRenderable()))
			{
				if (back_face_depth_pass)
				{
					detailed_mesh->BackFaceDepthPass(true);
				}
				else
				{
					detailed_mesh->EyePos(eye_pos);
					detailed_mesh->LightPos(light_pos);
					detailed_mesh->LightColor(light_color);
					detailed_mesh->LightFalloff(light_falloff);
					detailed_mesh->BackFaceDepthPass(false);
				}
			}
		});
		return true;
	});
}

uint32_t GameApp::DoUpdate(uint32_t pass)
{
	auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

	switch (pass)
	{
	case 0:
		re.BindFrameBuffer(back_face_depth_fb_);
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0, 0, 0, 0), 0.0f, 0);
		UpdateDetailedMeshes(true);
		return URV_NeedFlush;

	case 1:
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
		UpdateDetailedMeshes(false);
		return URV_Finished;

	default:
		COMMON_ASSERT(false);
		return URV_Finished;
	}
}

void GameApp::InputHandler(InputEngine const& /*sender*/, InputAction const& action)
{
	if (action.first == Exit)
	{
		Quit();
	}
}

} // namespace EditorWorker
