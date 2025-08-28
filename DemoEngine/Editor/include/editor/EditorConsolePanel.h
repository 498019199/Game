
#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
class EditorConsolePanel: public EditorPanel
{
public:
    EditorConsolePanel();
    ~EditorConsolePanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;
};



}