#pragma once
#include <base/App3D.h>
#include <world/CameraController.h>
#include <editor/EditorPanel.h>
#include <editor/EditorProjectPanel.h>

#include <render/FrameBuffer.h>
#include <render/Texture.h>
#include <render/RenderView.h>
#include <render/RenderDeviceCaps.h>
#include <render/RenderFactory.h>

#include <memory>
#include <vector>

enum class ETransformType
{
    TransformType_Position,
    TransformType_Rotation,
    TransformType_Scale
};

namespace EditorWorker
{
class EditorManagerD3D11: public RenderWorker::App3D
{
public:
    EditorManagerD3D11();
    ~EditorManagerD3D11();

    virtual void OnCreate() override;
	void OnResize(uint32_t width, uint32_t height) override;
    void DoUpdateOverlay();
    virtual void OnDestroy() override;

    static void SetWindowSize(int hWidth, int pHeight, int iWidth);

    void SetSelectedAssert(const EditorAssetNodePtr node);
    const AssertBaseInfoPtr& GetSelectedAssert() const {  return selected_asset_info_; };
    AssetType GetAssertType() const;

    void SetTransformType(ETransformType type) { current_transform_type_ = type; }
    ETransformType GetTransformType() const { return current_transform_type_; }

    void GetEditorSetting( const EditorSetting& setting) { setting_ = setting; }
    void RenderEditorPanels() const;

    /// ID3D11ShaderResourceView* for ImGui::Image；场景先渲染到离屏 RT 再采样（勿在绑定为 RT 时采样）
    void* GameViewShaderResourceView() const;

    void InputHandler(RenderWorker::InputEngine const & sender, RenderWorker::InputAction const & action);

private :
    virtual uint32_t DoUpdate(uint32_t pass) override;

    
    std::string LoadTextFile(const std::string_view& path);

    void RebuildGameViewRenderTarget(RenderWorker::RenderFactory& rf, RenderWorker::RenderDeviceCaps const& caps);
private:
    std::vector<EditorPanelPtr> panel_list_;
    EditorSetting setting_;

    EditorAssetNode* selected_asset_ptr_ { nullptr};
    AssertBaseInfoPtr selected_asset_info_ { nullptr };

    RenderWorker::RenderModelPtr model_;
    RenderWorker::LightSourcePtr light_;
	bool depth_texture_support_;
	RenderWorker::FrameBufferPtr back_face_depth_fb_;

    RenderWorker::FrameBufferPtr game_view_fb_;
    RenderWorker::TexturePtr game_view_color_tex_;
    RenderWorker::ShaderResourceViewPtr game_view_srv_;

    ETransformType current_transform_type_ { ETransformType::TransformType_Position };


};















}