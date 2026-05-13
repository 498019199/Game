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
    void DrawScript(AssertBaseInfo& info);
    void DrawShader(AssertBaseInfo& info);
    void DrawTexture(AssertBaseInfo& info);
    void DrawMaterial(AssertBaseInfo& info);
    void DrawModel(AssertBaseInfo& info);
    void DrawAudio(AssertBaseInfo& info);

private:
    int inspector_width_ {0};
};




}