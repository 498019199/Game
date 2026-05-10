#include <editor/EditorRmlUiHost.h>
#include <editor/RmlUiRenderInterfaceD3D11.h>

#include <base/Context.h>
#include <base/ResLoader.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Debugger/Debugger.h>

#include <base/App3D.h>

#include <imgui/imgui.h>

#include <d3d11.h>

#include <algorithm>

namespace EditorWorker {

class EditorRmlSystemInterface final : public Rml::SystemInterface {
public:
    double GetElapsedTime() override { return RenderWorker::Context::Instance().AppInstance().AppTime(); }
};

EditorRmlUiHost::EditorRmlUiHost() = default;

EditorRmlUiHost::~EditorRmlUiHost()
{
	Shutdown();
}

void EditorRmlUiHost::Init(ID3D11Device* device, ID3D11DeviceContext* ctx, int width, int height)
{
	if (initialized_)
	{
		return;
	}
	if (!device || !ctx)
	{
		return;
	}

	system_interface_ = std::make_unique<EditorRmlSystemInterface>();
	render_interface_ = std::make_unique<RmlUiRenderInterfaceD3D11>(device, ctx);
	Rml::SetSystemInterface(reinterpret_cast<Rml::SystemInterface*>(system_interface_.get()));
	Rml::SetRenderInterface(render_interface_.get());

	if (!Rml::Initialise())
	{
		render_interface_.reset();
		system_interface_.reset();
		return;
	}

	width_ = (std::max)(1, width);
	height_ = (std::max)(1, height);
	render_interface_->SetViewportSize(width_, height_);

	context_ = Rml::CreateContext("editor_game_view", Rml::Vector2i(width_, height_), render_interface_.get());
	if (!context_)
	{
		Rml::Shutdown();
		render_interface_.reset();
		system_interface_.reset();
		return;
	}

#ifdef _WIN32
	Rml::LoadFontFace("C:\\Windows\\Fonts\\segoeui.ttf", true);
#endif

	std::string const doc_path = RenderWorker::Context::Instance().ResLoaderInstance().Locate("EditorAssets/rmlui/login.rml");
	if (!doc_path.empty())
	{
		// RmlUi 6：LoadDocument 结束后 ProcessHeader 会把文档设为 Visibility::Hidden，未 Show 时不会进入 stacking context，
		// Render() 不会产生任何 Geometry，CompileGeometry / RenderGeometry 永远不会被调用。
		if (Rml::ElementDocument* doc = context_->LoadDocument(Rml::String(doc_path)))
		{
			doc->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
		}
	}

	debugger_initialized_ = Rml::Debugger::Initialise(context_);
	if (debugger_initialized_)
	{
		Rml::Debugger::SetContext(context_);
		Rml::Debugger::SetVisible(false);
	}

	game_image_hovered_last_ = false;
	initialized_ = true;
}

void EditorRmlUiHost::Shutdown()
{
	if (!initialized_)
	{
		return;
	}

	if (context_)
	{
		if (debugger_initialized_)
		{
			Rml::Debugger::Shutdown();
			debugger_initialized_ = false;
		}
		context_->UnloadAllDocuments();
		context_->Update();
		Rml::RemoveContext("editor_game_view");
		context_ = nullptr;
	}

	Rml::ReleaseRenderManagers();
	Rml::Shutdown();

	render_interface_.reset();
	system_interface_.reset();
	initialized_ = false;
}

void EditorRmlUiHost::SetDimensions(int width, int height)
{
	width_ = (std::max)(1, width);
	height_ = (std::max)(1, height);
	if (render_interface_)
	{
		render_interface_->SetViewportSize(width_, height_);
	}
	if (context_)
	{
		context_->SetDimensions(Rml::Vector2i(width_, height_));
	}
}

void EditorRmlUiHost::RenderIntoGameView()
{
	if (!initialized_ || !context_ || !render_interface_)
	{
		return;
	}
	render_interface_->SetViewportSize(width_, height_);
	context_->Update();
	context_->Render();
}

void EditorRmlUiHost::SetDebuggerVisible(bool visible)
{
	if (debugger_initialized_)
	{
		Rml::Debugger::SetVisible(visible);
	}
}

bool EditorRmlUiHost::IsDebuggerVisible() const
{
	return debugger_initialized_ && Rml::Debugger::IsVisible();
}

void EditorRmlUiHost::ProcessGameViewPointer(bool image_hovered, int mouse_x, int mouse_y, int key_modifier_state,
	bool left_pressed, bool left_released, bool right_pressed, bool right_released, bool middle_pressed,
	bool middle_released, float wheel_x, float wheel_y)
{
	if (!initialized_ || !context_)
	{
		return;
	}

	if (!image_hovered)
	{
		if (game_image_hovered_last_)
		{
			context_->ProcessMouseLeave();
		}
		game_image_hovered_last_ = false;
		return;
	}

	game_image_hovered_last_ = true;

	int const mx = (std::clamp)(mouse_x, 0, width_ - 1);
	int const my = (std::clamp)(mouse_y, 0, height_ - 1);
	context_->ProcessMouseMove(mx, my, key_modifier_state);

	if (left_pressed)
	{
		context_->ProcessMouseButtonDown(0, key_modifier_state);
	}
	if (right_pressed)
	{
		context_->ProcessMouseButtonDown(1, key_modifier_state);
	}
	if (middle_pressed)
	{
		context_->ProcessMouseButtonDown(2, key_modifier_state);
	}

	if (left_released)
	{
		context_->ProcessMouseButtonUp(0, key_modifier_state);
	}
	if (right_released)
	{
		context_->ProcessMouseButtonUp(1, key_modifier_state);
	}
	if (middle_released)
	{
		context_->ProcessMouseButtonUp(2, key_modifier_state);
	}

	if (wheel_x != 0.f || wheel_y != 0.f)
	{
		context_->ProcessMouseWheel(Rml::Vector2f(wheel_x, wheel_y), key_modifier_state);
	}
}

void EditorRmlUiHost::ProcessGameViewImGuiKeyboardRelay(bool enabled, ImGuiIO const* io)
{
	if (!enabled || !initialized_ || !context_ || !io)
	{
		return;
	}

	int mods = 0;
	if (io->KeyCtrl)
	{
		mods |= int(Rml::Input::KM_CTRL);
	}
	if (io->KeyShift)
	{
		mods |= int(Rml::Input::KM_SHIFT);
	}
	if (io->KeyAlt)
	{
		mods |= int(Rml::Input::KM_ALT);
	}
	if (io->KeySuper)
	{
		mods |= int(Rml::Input::KM_META);
	}

	for (int n = 0; n < io->InputQueueCharacters.Size; n++)
	{
		unsigned int const c = static_cast<unsigned int>(io->InputQueueCharacters[n]);
		if (c > 0 && c < 0x110000u)
		{
			context_->ProcessTextInput(static_cast<Rml::Character>(c));
		}
	}

	auto relay = [this, mods](ImGuiKey key, Rml::Input::KeyIdentifier kid) {
		if (ImGui::IsKeyPressed(key, false))
		{
			context_->ProcessKeyDown(kid, mods);
		}
		if (ImGui::IsKeyReleased(key))
		{
			context_->ProcessKeyUp(kid, mods);
		}
	};

	relay(ImGuiKey_Backspace, Rml::Input::KI_BACK);
	relay(ImGuiKey_Delete, Rml::Input::KI_DELETE);
	relay(ImGuiKey_Tab, Rml::Input::KI_TAB);
	relay(ImGuiKey_Enter, Rml::Input::KI_RETURN);
	relay(ImGuiKey_LeftArrow, Rml::Input::KI_LEFT);
	relay(ImGuiKey_RightArrow, Rml::Input::KI_RIGHT);
	relay(ImGuiKey_UpArrow, Rml::Input::KI_UP);
	relay(ImGuiKey_DownArrow, Rml::Input::KI_DOWN);
	relay(ImGuiKey_Home, Rml::Input::KI_HOME);
	relay(ImGuiKey_End, Rml::Input::KI_END);
	relay(ImGuiKey_Escape, Rml::Input::KI_ESCAPE);
}

} // namespace EditorWorker
