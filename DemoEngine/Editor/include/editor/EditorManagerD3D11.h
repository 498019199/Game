#pragma once
#include <editor/EditorPanel.h>
#include <editor/EditorProjectPanel.h>

#include <vector>

namespace EditorWorker
{
class EditorManagerD3D11
{
public:
    EditorManagerD3D11();
    ~EditorManagerD3D11();

    void Init();
    void SetWindowSize(int hWidth, int pHeight, int iWidth);

    void BeginRender();
    void Render();
    void EndRender();

private:
    std::vector<EditorPanelPtr> panel_list_;
    EditorSetting setting_;
};





}