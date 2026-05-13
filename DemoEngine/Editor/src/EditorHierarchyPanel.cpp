#include <editor/EditorHierarchyPanel.h>
#include <base/Context.h>
#include <world/World.h>

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
        for( const auto& node : Context::Instance().WorldInstance().SceneRootNode().Children() )
        {
            std::string node_name;
            CommonWorker::Convert(node_name, node->Name());
            ImGui::BulletText(node_name.c_str());
        }
    }
    ImGui::End();
}

void EditorHierarchyPanel::OnResize()
{
    
}

}
