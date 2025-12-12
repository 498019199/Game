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
    void DrawMaterial(const AssertBaseInfo& info);
    void DrawModel(const AssertBaseInfo& info);
    void DrawAudio(const AssertBaseInfo& info);

private:
    int inspector_width_ {0};
};




}