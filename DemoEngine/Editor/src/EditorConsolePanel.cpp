
#include <editor/EditorConsolePanel.h>

namespace EditorWorker
{

EditorConsolePanel::EditorConsolePanel()
{
    
}

EditorConsolePanel::~EditorConsolePanel()
{
    
}

void EditorConsolePanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2((float)setting.projectWidth, (float)setting.mainBarHeight + (float)setting.hierarchyHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.consoleWidth, (float)setting.consoleHeight));

    // 设置面板具体内容
    ImGui::Begin("Console", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    {
        // 默认拉到最底部
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }

    ImGui::End();
}

void EditorConsolePanel::OnResize()
{
    
}

}
