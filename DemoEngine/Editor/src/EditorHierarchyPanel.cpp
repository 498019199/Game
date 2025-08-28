#include <editor/EditorHierarchyPanel.h>

namespace EditorWorker
{

EditorHierarchyPanel::EditorHierarchyPanel()
{
    
}

EditorHierarchyPanel::~EditorHierarchyPanel()
{
    
}

void EditorHierarchyPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2(0, (float)setting.mainBarHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.hierarchyWidth, (float)setting.hierarchyHeight));

    // 设置面板具体内容
    if (ImGui::Begin("Hierarchy", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
    {
        // nodeIdx = 0;
        // auto scene = SceneManager::GetInstance()->GetCurScene();
        // for (auto gameObject : scene->gameObjects)
        //     DrawNode(gameObject);
    }
    ImGui::End();
}

void EditorHierarchyPanel::OnResize()
{
    
}

}
