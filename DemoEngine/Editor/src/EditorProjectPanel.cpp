#include <editor/EditorProjectPanel.h>


namespace EditorWorker
{

EditorProjectPanel::EditorProjectPanel()
{
    
}

EditorProjectPanel::~EditorProjectPanel()
{
    
}

void EditorProjectPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
    ImGui::SetNextWindowPos(ImVec2(0, (float)setting.mainBarHeight + (float)setting.hierarchyHeight));
    ImGui::SetNextWindowSize(ImVec2((float)setting.projectWidth, (float)setting.projectHeight));

    // 设置面板具体内容
    ImGui::Begin("Peoject", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    {
        // 记录一下按钮原本颜色
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4 btnColor = style.Colors[ImGuiCol_Button];
        ImVec4 selectBtnColor = ImVec4(btnColor.x - 0.1f, btnColor.y - 0.1f, btnColor.z - 0.1f, 1.0f);
        ImVec4 textColor = style.Colors[ImGuiCol_Text];
    }
}

void EditorProjectPanel::OnResize()
{
    
}

}