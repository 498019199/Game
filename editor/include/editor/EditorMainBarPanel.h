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

private:
    ImVec4 selectBtnColor;
    ImVec4 selectTextColor;
	const ImVec2 buttonSize = ImVec2(22.0f, 22.0f);
};


}