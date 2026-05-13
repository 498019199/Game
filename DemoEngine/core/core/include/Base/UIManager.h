#pragma once

#include <memory>


namespace Rml {
class Context;
}

struct ImGuiIO; // Dear ImGui (forward declaration)

namespace EditorWorker 
{
class EditorRmlSystemInterface;

/// RmlUi overlay drawn into the game-view render target after the 3D pass (same RT as ImGui preview samples).
class UIManager 
{
	ZENGINE_NONCOPYABLE(UIManager);
public:
	struct VertexFormat
	{
		float3 pos;
		Color clr;
		float2 tex;

		VertexFormat()
		{
		}
		VertexFormat(float3 const & p, Color const & c, float2 const & t)
			: pos(p), clr(c), tex(t)
		{
		}
	};

	UIManager();
	~UIManager();

	void Init(int width, int height);
	void Shutdown();

	void SetDimensions(int width, int height);

	/// Call with `game_view_fb_` bound; runs layout/update/render for one frame.
	void RenderIntoGameView();

	/// RmlUi built-in debugger (element tree / styles). Off by default.
	void SetDebuggerVisible(bool visible);
	bool IsDebuggerVisible() const;

	/// Pointer events in game-view RT pixel space (same resolution as SetDimensions). Call from Game panel after `ImGui::Image`.
	void ProcessGameViewPointer(bool image_hovered, int mouse_x, int mouse_y, int key_modifier_state, bool left_pressed,
		bool left_released, bool right_pressed, bool right_released, bool middle_pressed, bool middle_released,
		float wheel_x, float wheel_y);

	/// When debugger is visible and the Game image is hovered, relay WM_CHAR / keys (pass `ImGui::GetIO()`).
	void ProcessGameViewImGuiKeyboardRelay(bool enabled, ImGuiIO const* io);

private:
	bool debugger_initialized_{false};

	bool game_image_hovered_last_{false};
	int width_{1};
	int height_{1};

	std::unique_ptr<EditorRmlSystemInterface> system_interface_;
	Rml::Context* rml_context_{nullptr};
};

} // namespace EditorWorker
