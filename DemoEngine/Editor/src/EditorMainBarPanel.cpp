#include <editor/EditorMainBarPanel.h>
#include <editor/EditorDialogBoxManager.h>


namespace EditorWorker
{
EditorMainBarPanel::EditorMainBarPanel()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 btnColor = style.Colors[ImGuiCol_Button];
    selectBtnColor = ImVec4(btnColor.x - 0.1f, btnColor.y - 0.1f, btnColor.z - 0.1f, 1.0f);
    selectTextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
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
        if (setting.is_game_started_)
        {
            isPop = true;
            ImGui::PushStyleColor(ImGuiCol_Text, selectTextColor);
            ImGui::PushStyleColor(ImGuiCol_Button, selectBtnColor);
        }
        if (ImGui::Button(">>", buttonSize))
        {
            // 关掉游戏运行的时候，重置暂停按钮
            if (setting.is_game_started_)
            {
                //Time::curTime = 0.0f;
                // setting.is_game_started_ = false;
                // EditorDataManager::GetInstance()->selectedGO = nullptr;
                // EditorGUIManager::GetInstance()->ResetPanels();
                // SceneManager::GetInstance()->ReloadScene();
            }
            //EditorDataManager::isGameStart = !EditorDataManager::isGameStart;
        }
        if (isPop)
        {
            isPop = false;
            ImGui::PopStyleColor(2);
        }


        // Pause
        if (setting.is_game_paused_)
        {
            isPop = true;
            ImGui::PushStyleColor(ImGuiCol_Text, selectTextColor);
            ImGui::PushStyleColor(ImGuiCol_Button, selectBtnColor);
        }
        ImGui::SameLine();
        if (ImGui::Button("||", buttonSize))
        {
            //EditorDataManager::isGamePause = !EditorDataManager::isGamePause;
            //AudioEngine::GetInstance()->SetAllPause(EditorDataManager::isGamePause);
        }
        if (isPop)
        {
            isPop = false;
            ImGui::PopStyleColor(2);
        }

        // Step
        ImGui::SameLine();
        if (ImGui::Button(">|", buttonSize))
        {
            if (setting.is_game_started_ && setting.is_game_paused_)
            {
                // SceneManager::GetInstance()->GetCurScene()->Update();
            }
        }
		ImGui::End();
    }

}

void EditorMainBarPanel::OnResize()
{


}
}