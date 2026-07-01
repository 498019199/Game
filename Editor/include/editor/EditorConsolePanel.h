
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

private:
	const ImVec2 iconSize = ImVec2(20, 20);
	bool showMessage = true;
	bool showWarning = true;
	bool showError = true;
};



}