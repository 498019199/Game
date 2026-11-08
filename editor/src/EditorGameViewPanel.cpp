#include <editor/EditorGameViewPanel.h>
#include <editor/EditorManager.h>
#include <editor/EditorSceneGizmos.h>
#include <editor/EditorProfilerPanel.h>
#include <base/ZEngine.h>
#include <base/UIManager.h>
#include <RmlUi/Core/Input.h>

namespace EditorWorker
{
using namespace RenderWorker;


EditorGameViewPanel::EditorGameViewPanel() : gizmos_(std::make_unique<EditorSceneGizmos>()) {}

EditorGameViewPanel::~EditorGameViewPanel() = default;

void EditorGameViewPanel::OnRender(const EditorSetting& setting)
{
	ImGui::SetNextWindowPos(ImVec2((float)setting.hierarchyWidth, (float)setting.mainBarHeight));
	ImGui::SetNextWindowSize(ImVec2((float)setting.gameViewWidth, (float)setting.gameViewHeight));

	bool game_view_input_active = false;

	if (ImGui::IsKeyPressed(ImGuiKey_F3, false))
	{
		EditorProfilerPanel::SetVisible(!EditorProfilerPanel::Visible());
	}

	if (!ImGui::GetIO().WantTextInput)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl, false) || ImGui::IsKeyPressed(ImGuiKey_RightCtrl, false))
		{
			mouse_look_suspended_ = !mouse_look_suspended_;
		}
	}

	EditorManager& editor = checked_cast<EditorManager&>(Context::Instance().AppInstance());
	editor.SetCameraMoveBoost(ImGui::GetIO().KeyShift);
    bool scene_visible = false;

	if (ImGui::Begin("Game", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
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
				if (ImGui::BeginTabItem("Scene", nullptr, editor.ConsumeSceneFocus() ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
				{
					view_tab = 1;
					ImGui::EndTabItem();
				}
                    auto material_flags = editor.ConsumePreviewFocus() ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
                    if (ImGui::BeginTabItem("Material", nullptr, material_flags)) {
                        view_tab = 2; ImGui::EndTabItem();
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

		scene_visible = view_tab == 1 && !setting.is_game_started_;
        editor.SceneViewActive(scene_visible);
		if (view_tab == 0 || view_tab == 1)
		{
			ImVec2 const avail = ImGui::GetContentRegionAvail();
			void* const game_srv = editor.GameViewShaderResourceView();
			ImVec2 overlay_anchor = ImGui::GetCursorScreenPos();
			overlay_anchor.x += 8.f;
			overlay_anchor.y += 8.f;

			ImVec2 image_min {};
			ImVec2 image_size = avail;
			bool image_hovered = false;

			if (game_srv != nullptr && avail.x > 1.f && avail.y > 1.f)
			{
				ImGui::Image((ImTextureID)(intptr_t)game_srv, avail);
				image_hovered = ImGui::IsItemHovered();
				game_view_input_active = !mouse_look_suspended_ && (image_hovered || ImGui::IsItemActive());
				image_min = ImGui::GetItemRectMin();
				image_size = ImGui::GetItemRectSize();
                if (scene_visible) {
                    gizmos_->Draw(editor, image_min, image_size, image_hovered);
                    game_view_input_active = false; // Scene owns its ImGui navigation input.
                }
				overlay_anchor = image_min;
				overlay_anchor.x += 8.f;
				overlay_anchor.y += 8.f;
			}
			else
			{
				ImGui::TextUnformatted("Game view RT unavailable");
			}

			auto& ui = Context::Instance().UIManagerInstance();

			if (game_srv != nullptr && image_size.x > 1.f && image_size.y > 1.f)
			{
				ImVec2 const origin = ImGui::GetMainViewport()->Pos;
				ui.SetInputViewport(view_tab == 0 && image_hovered, view_tab == 0 && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows),
					image_min.x - origin.x, image_min.y - origin.y, image_size.x, image_size.y);
			}

			if (EditorProfilerPanel::Visible())
			{
				EditorProfilerPanel::DrawOverlay(overlay_anchor);
				if (ImGui::IsItemHovered() || ImGui::IsItemActive())
				{
					game_view_input_active = false;
					ui.SetInputViewport(false, false, 0, 0, 1, 1);
				}
			}
		}
		else if (view_tab == 2)
		{
			editor.DrawMaterialPreview();
		}
		else
		{
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

    if (!scene_visible) { gizmos_->Deactivate(editor); editor.SceneViewActive(false); }
	editor.GameViewInputActive(game_view_input_active);
}

void EditorGameViewPanel::OnResize()
{
}

}
