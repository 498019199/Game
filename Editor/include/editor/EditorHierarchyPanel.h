#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
class EditorHierarchyPanel: public EditorPanel
{
public:
    EditorHierarchyPanel();
    ~EditorHierarchyPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;
};

}