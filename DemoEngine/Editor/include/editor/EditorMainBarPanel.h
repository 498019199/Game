#pragma once
#include <editor/EditorPanel.h>

namespace EditorWorker
{
class EditorMainBarPanel: public EditorPanel
{
public:
    EditorMainBarPanel();
    ~EditorMainBarPanel();

    virtual void OnRender(const EditorSetting& setting) override;
    virtual void OnResize() override;
};


}