
#include <editor/EditorGameViewPanel.h>
#include <editor/EditorManagerD3D11.h>
#include <base/ZEngine.h>

namespace EditorWorker
{
using namespace RenderWorker;

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

    bool game_view_input_active = false;

    // 设置面板具体内容
	if (ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
        if (ImGui::BeginTabBar("ViewSwitchBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Game"))
            {
                EditorManagerD3D11& editor = checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance());
                ImVec2 const avail = ImGui::GetContentRegionAvail();
                void* const game_srv = editor.GameViewShaderResourceView();
                if (game_srv != nullptr && avail.x > 1.f && avail.y > 1.f)
                {
                    ImGui::Image((ImTextureID)(intptr_t)game_srv, avail);
                    game_view_input_active = ImGui::IsItemHovered() || ImGui::IsItemActive();
                }
                else
                {
                    ImGui::TextUnformatted("Game view RT unavailable");
                }
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

    checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance()).GameViewInputActive(game_view_input_active);
}

void EditorGameViewPanel::OnResize()
{
    
}

}