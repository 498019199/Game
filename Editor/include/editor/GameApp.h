#pragma once

#include <base/App3D.h>
#include <render/FrameBuffer.h>
#include <render/RenderDeviceCaps.h>

#include <game/Scene.h>

#include <memory>
#include <string>
#include <string_view>

namespace EditorWorker
{
class GameApp : public RenderWorker::App3D
{
public:
	explicit GameApp(std::string_view scene_path = "Scenes/DefaultScene.scene");
	~GameApp() override;

	void OnCreate() override;
	void OnResize(uint32_t width, uint32_t height) override;
	void OnDestroy() override;

	void SetScenePath(std::string_view scene_path);

private:
	uint32_t DoUpdate(uint32_t pass) override;

	void InputHandler(RenderWorker::InputEngine const& sender, RenderWorker::InputAction const& action);
	void ApplySceneCamera();
	void RebuildBackFaceDepthTarget(RenderWorker::RenderFactory& rf, RenderWorker::RenderDeviceCaps const& caps, uint32_t width, uint32_t height);

private:
	std::string scene_path_;
	AScene scene_;


	bool depth_texture_support_ { false };
	RenderWorker::FrameBufferPtr back_face_depth_fb_;
};
} // namespace EditorWorker
