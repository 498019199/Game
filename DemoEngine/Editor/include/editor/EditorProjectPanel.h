#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
class EditorProjectPanel: public EditorPanel
{
public:
    EditorProjectPanel();
    ~EditorProjectPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;

private:

};

}