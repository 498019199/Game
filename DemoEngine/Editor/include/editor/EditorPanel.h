#pragma once
#include <memory>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>

namespace EditorWorker
{
struct EditorSetting
{
    int hierarchyWidth;
    int hierarchyHeight;
    int consoleWidth;
    int consoleHeight;
    int projectWidth;
    int projectHeight;
    int inspectorWidth;
    int inspectorHeight;
    int mainBarWidth;
    int mainBarHeight;
};

class EditorPanel
{
public:
    EditorPanel() = default;
    ~EditorPanel() = default;

    virtual void OnRender(const EditorSetting& setting) = 0;
    virtual void OnResize() = 0;
private:
};
using EditorPanelPtr = std::shared_ptr<EditorPanel>;
}