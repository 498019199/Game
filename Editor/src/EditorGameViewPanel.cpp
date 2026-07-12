#include <editor/EditorGameViewPanel.h>
#include <editor/EditorManagerD3D11.h>
#include <editor/EditorProfilerPanel.h>
#include <base/ZEngine.h>
#include <base/UIManager.h>
#include <RmlUi/Core/Input.h>

namespace EditorWorker
{
using namespace RenderWorker;

namespace
{
int ImGuiKeyModifiers()
{
	ImGuiIO const& io = ImGui::GetIO();
	int mods = 0;
	if (io.KeyCtrl)
	{
		mods |= Rml::Input::KM_CTRL;
	}
	if (io.KeyShift)
	{
		mods |= Rml::Input::KM_SHIFT;
	}
	if (io.KeyAlt)
	{
		mods |= Rml::Input::KM_ALT;
	}
	return mods;
}
} // namespace

EditorGameViewPanel::EditorGameViewPanel() = default;

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

			ImVec2 image_min {};
			ImVec2 image_size = avail;
			bool image_hovered = false;

			if (game_srv != nullptr && avail.x > 1.f && avail.y > 1.f)
			{
				ImGui::Image((ImTextureID)(intptr_t)game_srv, avail);
				image_hovered = ImGui::IsItemHovered();
				game_view_input_active = image_hovered || ImGui::IsItemActive();
				image_min = ImGui::GetItemRectMin();
				image_size = ImGui::GetItemRectSize();
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
				ImVec2 const mouse = ImGui::GetIO().MousePos;
				float const u = (mouse.x - image_min.x) / image_size.x;
				float const v = (mouse.y - image_min.y) / image_size.y;
				int const mx = static_cast<int>(u * setting.gameViewWidth);
				int const my = static_cast<int>(v * setting.gameViewHeight);
				int const mods = ImGuiKeyModifiers();
				ui.ProcessGameViewPointer(
					image_hovered,
					mx, my, mods,
					ImGui::IsMouseClicked(ImGuiMouseButton_Left),
					ImGui::IsMouseReleased(ImGuiMouseButton_Left),
					ImGui::IsMouseClicked(ImGuiMouseButton_Right),
					ImGui::IsMouseReleased(ImGuiMouseButton_Right),
					ImGui::IsMouseClicked(ImGuiMouseButton_Middle),
					ImGui::IsMouseReleased(ImGuiMouseButton_Middle),
					0.f, ImGui::GetIO().MouseWheel);
			}

			if (EditorProfilerPanel::Visible())
			{
				EditorProfilerPanel::DrawOverlay(overlay_anchor);
				if (ImGui::IsItemHovered() || ImGui::IsItemActive())
				{
					game_view_input_active = false;
				}
			}
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

	checked_cast<EditorManagerD3D11&>(Context::Instance().AppInstance()).GameViewInputActive(game_view_input_active);
}

void EditorGameViewPanel::OnResize()
{
}

}
