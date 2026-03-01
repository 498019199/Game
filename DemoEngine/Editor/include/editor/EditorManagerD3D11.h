#pragma once
#include <base/App3D.h>
#include <editor/EditorPanel.h>
#include <editor/EditorProjectPanel.h>

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
    virtual void OnDestroy() override;

    static void SetWindowSize(int hWidth, int pHeight, int iWidth);

    void SetSelectedAssert(const EditorAssetNodePtr node);
    const AssertBaseInfoPtr& GetSelectedAssert() const {  return selected_asset_info_; };
    AssetType GetAssertType() const;

    void SetTransformType(ETransformType type) { current_transform_type_ = type; }
    ETransformType GetTransformType() const { return current_transform_type_; }

    void GetEditorSetting( const EditorSetting& setting) { setting_ = setting; }
private :
    virtual uint32_t DoUpdate(uint32_t pass) override;

    
    std::string LoadTextFile(const std::string_view& path);
private:
    std::vector<EditorPanelPtr> panel_list_;
    EditorSetting setting_;

    EditorAssetNode* selected_asset_ptr_ { nullptr};
    AssertBaseInfoPtr selected_asset_info_ { nullptr };

    ETransformType current_transform_type_ { ETransformType::TransformType_Position };
};















}