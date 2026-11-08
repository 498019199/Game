#pragma once

#include <editor/EditorPanel.h>

namespace EditorWorker
{

/// Small profiler overlay drawn on top of the Game view.
class EditorProfilerPanel
{
public:
	static constexpr float kOverlayWidth = 360.f;
	static constexpr float kOverlayHeight = 280.f;

	static void DrawOverlay(ImVec2 const& anchor_screen_pos);

	static bool& Visible();
	static void SetVisible(bool visible);
};

} // namespace EditorWorker
