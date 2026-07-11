#pragma once

#include <game/GameApi.h>

#include <string>
#include <string_view>

namespace Rml
{
class ElementDocument;
}

class GAME_API GmDebugWindow
{
public:
	GmDebugWindow();
	~GmDebugWindow();

	GmDebugWindow(GmDebugWindow const&) = delete;
	GmDebugWindow& operator=(GmDebugWindow const&) = delete;

	bool Initialize();
	void Shutdown();

	void ToggleVisible();
	void SetVisible(bool visible);
	bool Visible() const noexcept { return visible_; }

	void FocusInput();
	/// Read current input text, run command, clear the field.
	void Submit();
	void ExecuteCommand(std::string_view command);

private:
	void AppendLog(std::string_view text);

	Rml::ElementDocument* document_ {nullptr};
	bool visible_ {false};
	bool initialized_ {false};
};
