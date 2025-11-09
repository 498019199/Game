#pragma once
#include <editor/EditorPanel.h>
#include <base/ZEngine.h>
#include <render/RenderView.h> 
#include <render/RenderMaterial.h> 
#include <map>

namespace EditorWorker
{
struct EditorAssetNode;
using EditorAssetNodePtr = std::shared_ptr<EditorAssetNode>;
struct EditorAssetNode
{
    std::string path;
    std::string name;
    std::string extension;
    uint32_t size = 0;
    AssetType type = AssetType::Other;
    EditorAssetNodePtr parent;
    std::vector<EditorAssetNodePtr> children;
};


class EditorProjectPanel: public EditorPanel
{
    // 文件名大小
    const ImVec2 nameSize = ImVec2(72.0f, 20.0f);
    // 文件icon大小
    const ImVec2 iconSize = ImVec2(64.0f, 64.0f);
public:
    EditorProjectPanel();
    ~EditorProjectPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;

private:
	AssetType GetAssetType(const std::string& extension);

    void SetCurNode(const EditorAssetNodePtr& node);
    void GetChildren(const EditorAssetNodePtr& node);
private:
    // 当前选中的id
    int selected_ {-1};
    EditorAssetNodePtr root_ ;
    EditorAssetNodePtr cur_ ;

    std::map<std::string, AssetType> ext_type_map_;
    RenderWorker::ShaderResourceViewPtr fileIcons_[static_cast<std::size_t>(AssetType::Count)];
};




}