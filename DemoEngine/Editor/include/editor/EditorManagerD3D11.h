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

private:
    std::vector<EditorPanel> PanelList_;
};


}