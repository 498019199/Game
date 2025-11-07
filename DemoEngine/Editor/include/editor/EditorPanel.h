#pragma once
#include <base/ZEngine.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>

namespace EditorWorker
{
struct EditorSetting
{
    int hierarchyWidth {0};
    int hierarchyHeight {0};
    int consoleWidth {0};
    int consoleHeight {0};
    int projectWidth {0};
    int projectHeight {0};
    int inspectorWidth {0};
    int inspectorHeight {0};
    int mainBarWidth {0};
    int mainBarHeight {0};

    int srcWidth {0};
    int srcHeight {0};

    int gameViewWidth {0};
    int gameViewHeight {0};

    bool is_game_started_ {false};
    bool is_game_paused_ {false};
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