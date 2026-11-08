
#pragma once
#include <editor/EditorPanel.h>
#include <memory>

namespace EditorWorker
{
class EditorSceneGizmos;
class EditorGameViewPanel: public EditorPanel
{
public:
    EditorGameViewPanel();
    ~EditorGameViewPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;

private:
    std::unique_ptr<EditorSceneGizmos> gizmos_;
    bool mouse_look_suspended_ { false };
};



}