#include <editor/EditorMainBarPanel.h>
#include <editor/EditorDialogBoxManager.h>


namespace EditorWorker
{
EditorMainBarPanel::EditorMainBarPanel()
{

}

EditorMainBarPanel::~EditorMainBarPanel()
{

}

void EditorMainBarPanel::OnRender(const EditorSetting& setting)
{
    // 面板大小和位置
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2((float)setting.mainBarWidth, (float)setting.mainBarHeight));

    // 设置面板具体内容
    if (ImGui::Begin("Editor", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Scene"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");
                if (ImGui::MenuItem("Open Scene"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");
                if (ImGui::MenuItem("Save Scene"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");

                ImGui::Separator();

                if (ImGui::MenuItem("New Project"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");
                if (ImGui::MenuItem("Open Project"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");
                if (ImGui::MenuItem("Save Project"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Project Settings"))
                    POP_MESSAGE("Notice", "This feature is not implemented yet.");

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Assets"))
            {

            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // 3个按钮从左到右的宽度
    float threeButtonSize = 80.0f;
    float avail = ImGui::GetContentRegionAvail().x;
    float offset = (avail - threeButtonSize) * 0.5f;
    // 计算3个按钮要居中的话，第一个按钮的起始位置
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

    bool isPop = false;
}

void EditorMainBarPanel::OnResize()
{


}
}