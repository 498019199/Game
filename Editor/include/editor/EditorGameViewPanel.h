
#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
class EditorGameViewPanel: public EditorPanel
{
public:
    EditorGameViewPanel();
    ~EditorGameViewPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;

private:
    bool mouse_look_suspended_ { false };
};



}