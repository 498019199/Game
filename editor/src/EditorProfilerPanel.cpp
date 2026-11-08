#include <editor/EditorProfilerPanel.h>

#include <common/CpuProfiler.h>

#include <string>

namespace EditorWorker
{

namespace
{
bool g_profiler_visible = false;
}

bool& EditorProfilerPanel::Visible()
{
	return g_profiler_visible;
}

void EditorProfilerPanel::SetVisible(bool visible)
{
	g_profiler_visible = visible;
}

void EditorProfilerPanel::DrawOverlay(ImVec2 const& anchor_screen_pos)
{
	if (!g_profiler_visible)
	{
		return;
	}

	ImGui::SetCursorScreenPos(anchor_screen_pos);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.10f, 0.94f));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));

	if (ImGui::BeginChild("ProfilerOverlay", ImVec2(kOverlayWidth, kOverlayHeight),
			ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar))
	{
		auto const& profiler = CommonWorker::CpuProfiler::Instance();

		ImGui::TextUnformatted("Profiler");
		ImGui::SameLine(kOverlayWidth - 28.f);
		if (ImGui::SmallButton("X"))
		{
			g_profiler_visible = false;
		}

		ImGui::Text("FPS: %.1f", profiler.Fps());
		ImGui::Text("Frame: %.2f ms", profiler.FrameMs());
		ImGui::TextDisabled("F3 toggle");
		ImGui::Separator();

		float const table_h = ImGui::GetContentRegionAvail().y;
		if (ImGui::BeginTable("zones", 2,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
				ImVec2(0.f, table_h)))
		{
			ImGui::TableSetupColumn("Zone", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("ms", ImGuiTableColumnFlags_WidthFixed, 64.f);
			ImGui::TableHeadersRow();

			for (auto const& zone : profiler.LastFrameZones())
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				std::string label(static_cast<size_t>(zone.depth) * 2u, ' ');
				label += zone.name ? zone.name : "";
				ImGui::TextUnformatted(label.c_str());
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", zone.duration_ms);
			}

			ImGui::EndTable();
		}
	}
	ImGui::EndChild();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor();
}

} // namespace EditorWorker
