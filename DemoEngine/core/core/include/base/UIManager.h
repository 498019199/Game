#pragma once
#include <base/ZEngine.h>

#include <string>
#include <string_view>
#include <functional>

union SDL_Event;

namespace Rml {
	class Context;
	class ElementDocument;
}

namespace RenderWorker
{

class EditorRmlSystemInterface;
class RmlUiFileInterface;
class RmlUiRenderInterfaceD3D11;
struct UIInputState;

/// RmlUi overlay drawn into the game-view render target after the 3D pass.
/// All Rml document ops must go through this class (RmlUi is statically linked into core only).
class ZENGINE_CORE_API UIManager final
{
	ZENGINE_NONCOPYABLE(UIManager);
public:
	struct VertexFormat
	{
		float3 pos;
		Color clr;
		float2 tex;

		VertexFormat() = default;
		VertexFormat(float3 const& p, Color const& c, float2 const& t)
			: pos(p), clr(c), tex(t)
		{
		}
	};

	UIManager();
	~UIManager();

	void Init();
	void Destroy() noexcept;
	bool Valid() const noexcept;

	void SetDimensions(int width, int height);

	/// Call with game-view RT bound; runs layout/update/render for one frame.
	void RenderIntoGameView();

	Rml::Context* GetContext() noexcept { return rml_context_; }
	Rml::ElementDocument* LoadDocument(std::string_view path);
	Rml::ElementDocument* GetDocument(std::string_view id) noexcept;

	void ShowDocument(Rml::ElementDocument* document);
	void HideDocument(Rml::ElementDocument* document);
	void CloseDocument(Rml::ElementDocument* document);
	void PullDocumentToFront(Rml::ElementDocument* document);

	bool FocusElement(Rml::ElementDocument* document, std::string_view element_id);
	std::string GetInputValue(Rml::ElementDocument* document, std::string_view element_id) const;
	void SetInputValue(Rml::ElementDocument* document, std::string_view element_id, std::string_view value);
	std::string GetInnerRml(Rml::ElementDocument* document, std::string_view element_id) const;
	void SetInnerRml(Rml::ElementDocument* document, std::string_view element_id, std::string_view rml);

	void ProcessKeyDown(int rml_key_identifier, int key_modifier_state);
	void ProcessKeyUp(int rml_key_identifier, int key_modifier_state);
	void ProcessTextInput(char32_t character);


	void SetDebuggerVisible(bool visible);
	bool IsDebuggerVisible() const;


	bool MouseOnUI() const noexcept;

	// One event source, with Editor explicitly granting access to the game viewport.
	void QueueInput(SDL_Event const& event);
	void SetInputViewport(bool mouse, bool keyboard, float x, float y, float width, float height);
	void RouteInput(bool editor_mode, bool imgui_exclusive = false);
	bool GameKeyboardBlocked() const noexcept;
	bool GamePointerBlocked() const noexcept;
	void AcknowledgeGameInput() noexcept;
	void SetKeyHandler(std::function<bool(int)> handler);

private:
	std::shared_ptr<UIInputState> input_state_;
	bool debugger_initialized_ {false};
	int width_ {1};
	int height_ {1};
	bool mouse_on_ui_ {false};
	bool destroyed_ {true};

	std::unique_ptr<EditorRmlSystemInterface> system_interface_;
	std::unique_ptr<RmlUiFileInterface> file_interface_;
	std::unique_ptr<RmlUiRenderInterfaceD3D11> render_interface_;
	Rml::Context* rml_context_ {nullptr};
};

} // namespace RenderWorker
