#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
class EditorInspectorPanel: public EditorPanel
{
public:
    EditorInspectorPanel();
    ~EditorInspectorPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;
};

}