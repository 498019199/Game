#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
struct EditorAssetNode;

class EditorInspectorPanel: public EditorPanel
{
public:
    EditorInspectorPanel();
    ~EditorInspectorPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;


private:
    // Asset
    void DrawScript(const AssertBaseInfo& info);
    void DrawShader(const AssertBaseInfo& info);
    void DrawTexture(const AssertBaseInfo& info);
    // void DrawMaterial(AssetMaterialInfo* info);
    // void DrawModel(AssetModelInfo* info);
    // void DrawAudio(AssetAudioInfo* info);

private:
    int inspector_width_ {0};
};

}