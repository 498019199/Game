
#include <editor/EditorGameViewPanel.h>
#include <editor/EditorManagerD3D11.h>
#include <editor/EditorProfilerPanel.h>
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

    if (ImGui::IsKeyPressed(ImGuiKey_F3, false))
    {
        EditorProfilerPanel::SetVisible(!EditorProfilerPanel::Visible());
    }

    // 设置面板具体内容
	if (ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse))
	{
        ImGuiStyle const& style = ImGui::GetStyle();
        float const profiler_btn_w =
            ImGui::CalcTextSize("Profiler").x + style.FramePadding.x * 2.f;
        float const tab_row_h = ImGui::GetFrameHeightWithSpacing();

        int view_tab = 0;

        ImGui::BeginChild("ViewSwitchRow", ImVec2(0.f, tab_row_h), ImGuiChildFlags_None,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::BeginChild("ViewSwitchTabs", ImVec2(-profiler_btn_w - style.ItemSpacing.x, 0.f),
                ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            if (ImGui::BeginTabBar("ViewSwitchBar", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Game"))
                {
                    view_tab = 0;
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Scene"))
                {
                    view_tab = 1;
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();

            ImGui::SameLine(0.f, style.ItemSpacing.x);
            {
                bool const profiler_on = EditorProfilerPanel::Visible();
                if (profiler_on)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                }
                if (ImGui::Button("Profiler", ImVec2(profiler_btn_w, 0.f)))
                {
                    EditorProfilerPanel::SetVisible(!profiler_on);
                }
                if (profiler_on)
                {
                    ImGui::PopStyleColor();
                }
            }
        }
        ImGui::EndChild();

        if (view_tab == 0)
        {
            EditorManagerD3D11& editor = checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance());
            ImVec2 const avail = ImGui::GetContentRegionAvail();
            void* const game_srv = editor.GameViewShaderResourceView();
            ImVec2 overlay_anchor = ImGui::GetCursorScreenPos();
            overlay_anchor.x += 8.f;
            overlay_anchor.y += 8.f;

            if (game_srv != nullptr && avail.x > 1.f && avail.y > 1.f)
            {
                ImGui::Image((ImTextureID)(intptr_t)game_srv, avail);
                game_view_input_active = ImGui::IsItemHovered() || ImGui::IsItemActive();
                // Anchor to top-left of the game image so overlay stays above it.
                overlay_anchor = ImGui::GetItemRectMin();
                overlay_anchor.x += 8.f;
                overlay_anchor.y += 8.f;
            }
            else
            {
                ImGui::TextUnformatted("Game view RT unavailable");
            }

            // Draw after Image so the overlay is on top and not covered by Game content.
            if (EditorProfilerPanel::Visible())
            {
                EditorProfilerPanel::DrawOverlay(overlay_anchor);
                // Overlay captures mouse; don't treat game view as input-active over it.
                if (ImGui::IsItemHovered() || ImGui::IsItemActive())
                {
                    game_view_input_active = false;
                }
            }
        }
        else
        {
            // Scene tab placeholder
            if (EditorProfilerPanel::Visible())
            {
                ImVec2 anchor = ImGui::GetCursorScreenPos();
                anchor.x += 8.f;
                anchor.y += 8.f;
                EditorProfilerPanel::DrawOverlay(anchor);
            }
        }
    }

    ImGui::End();

    checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance()).GameViewInputActive(game_view_input_active);
}

void EditorGameViewPanel::OnResize()
{
    
}

}
