#pragma once
#include <base/App3D.h>
#include <editor/EditorPanel.h>
#include <editor/EditorProjectPanel.h>
#include <game/Scene.h>

#include <render/FrameBuffer.h>
#include <render/Texture.h>
#include <render/RenderView.h>
#include <render/RenderDeviceCaps.h>
#include <render/RenderFactory.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#endif

struct ImDrawData;

enum class ETransformType
{
    TransformType_Position,
    TransformType_Rotation,
    TransformType_Scale
};

namespace EditorWorker
{
class EditorManager: public RenderWorker::App3D
{
public:
    EditorManager();
    ~EditorManager() override;

    virtual void OnCreate() override;
	void OnResize(uint32_t width, uint32_t height) override;
    void DoUpdateOverlay();
    virtual void OnDestroy() override;

    static void SetWindowSize(int hWidth, int pHeight, int iWidth);

    void SetSelectedAssert(const EditorAssetNodePtr node);
    void SetSelectedSceneNode(RenderWorker::SceneNode const* node, std::string_view mesh_name = {});
    bool IsHierarchyItemSelected(RenderWorker::SceneNode const* node, std::string_view mesh_name = {}) const;
    const AssertBaseInfoPtr& GetSelectedAssert() const {  return selected_asset_info_; };
    AssetType GetAssertType() const;

    void SetTransformType(ETransformType type) { current_transform_type_ = type; }
    ETransformType GetTransformType() const { return current_transform_type_; }

    void GetEditorSetting( const EditorSetting& setting) { setting_ = setting; }
    void RenderEditorPanels() const;

    /// Native texture handle for ImGui::Image; the scene is rendered to an
    /// off-screen target before sampling (never sample while bound as an RT).
    void* GameViewShaderResourceView() const;
    void* ImGuiTextureHandle(RenderWorker::ShaderResourceViewPtr const& srv) const;
    void GameViewInputActive(bool active);
    void SetCameraMoveBoost(bool boost);

    void InputHandler(RenderWorker::InputEngine const & sender, RenderWorker::InputAction const & action);

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    virtual void ProcessWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
#endif

private :
    virtual uint32_t DoUpdate(uint32_t pass) override;

    std::string LoadTextFile(const std::string_view& path);
    void ApplySceneCamera();
    void RebuildBackFaceDepthTarget(RenderWorker::RenderFactory& rf, RenderWorker::RenderDeviceCaps const& caps, uint32_t width, uint32_t height);
    void RebuildGameViewRenderTarget(RenderWorker::RenderFactory& rf, RenderWorker::RenderDeviceCaps const& caps);

    virtual void InitializeImGui() = 0;
    virtual void ShutdownImGui() = 0;
    virtual void NewImGuiFrame() const = 0;
    virtual void RenderImGuiDrawData(ImDrawData* draw_data) const = 0;
    virtual void* GameViewTextureHandle() const = 0;
    virtual void* TextureHandle(RenderWorker::ShaderResourceView const* srv) const = 0;

protected:
    RenderWorker::TexturePtr const& GameViewColorTexture() const { return game_view_color_tex_; }
    RenderWorker::ShaderResourceViewPtr const& GameViewShaderResource() const { return game_view_srv_; }

    std::vector<EditorPanelPtr> panel_list_;
    EditorSetting setting_;

    EditorAssetNode* selected_asset_ptr_ { nullptr};
    AssertBaseInfoPtr selected_asset_info_ { nullptr };
    AssetType selected_asset_type_ { AssetType::Other };
    RenderWorker::SceneNode const* selected_scene_node_ { nullptr };
    std::string selected_mesh_name_;

    std::string scene_path_;
    AScene scene_;

	bool depth_texture_support_;
	RenderWorker::FrameBufferPtr back_face_depth_fb_;

    RenderWorker::FrameBufferPtr game_view_fb_;
    RenderWorker::TexturePtr game_view_color_tex_;
    RenderWorker::ShaderResourceViewPtr game_view_srv_;
    bool game_view_input_active_ { false };

    ETransformType current_transform_type_ { ETransformType::TransformType_Position };
};

}
