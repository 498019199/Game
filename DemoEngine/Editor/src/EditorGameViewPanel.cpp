
#include <editor/EditorGameViewPanel.h>

namespace EditorWorker
{

EditorGameViewPanel::EditorGameViewPanel()
{
    
}

EditorGameViewPanel::~EditorGameViewPanel()
{
    
}

void EditorGameViewPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2((float)setting.hierarchyWidth, (float)setting.mainBarHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.gameViewWidth, (float)setting.gameViewHeight));

    // 设置面板具体内容
	if (ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
        if (ImGui::BeginTabBar("ViewSwitchBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Game"))
            {
                //EditorDataManager::GetInstance()->isGameView = true;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scene"))
            {
                //EditorDataManager::GetInstance()->isGameView = false;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    ImGui::End();
}

void EditorGameViewPanel::OnResize()
{
    
}

}