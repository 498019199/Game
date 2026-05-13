#include <base/UIManager.h>

#include <base/Context.h>
#include <base/ResLoader.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Debugger/Debugger.h>

#include <base/App3D.h>


#include <algorithm>

namespace RenderWorker {

class EditorRmlSystemInterface final : public Rml::SystemInterface {
public:
    double GetElapsedTime() override { return Context::Instance().AppInstance().AppTime(); }
};

UIManager::UIManager() = default;
UIManager::~UIManager() = default;


void UIManager::Init()
{
	system_interface_ = CommonWorker::MakeUniquePtr<EditorRmlSystemInterface>();
	Rml::SetSystemInterface(reinterpret_cast<Rml::SystemInterface*>(system_interface_.get()));

	if (!Rml::Initialise())
	{
		system_interface_.reset();
		return;
	}

	rml_context_ = Rml::CreateContext("editor_game_view", Rml::Vector2i(width_, height_));
	if (!rml_context_)
	{
		Rml::Shutdown();
		system_interface_.reset();
		return;
	}

#ifdef _WIN32
	Rml::LoadFontFace("C:\\Windows\\Fonts\\segoeui.ttf", true);
#endif

	debugger_initialized_ = Rml::Debugger::Initialise(rml_context_);
	if (debugger_initialized_)
	{
		Rml::Debugger::SetContext(rml_context_);
		Rml::Debugger::SetVisible(false);
	}

	game_image_hovered_last_ = false;
}

void UIManager::Shutdown()
{
	if (rml_context_)
	{
		if (debugger_initialized_)
		{
			Rml::Debugger::Shutdown();
			debugger_initialized_ = false;
		}
		rml_context_->UnloadAllDocuments();
		rml_context_->Update();
		Rml::RemoveContext("editor_game_view");
		rml_context_ = nullptr;
	}

	Rml::ReleaseRenderManagers();
	Rml::Shutdown();

	system_interface_.reset();
}

void UIManager::SetDimensions(int width, int height)
{
	width_ = (std::max)(1, width);
	height_ = (std::max)(1, height);
	if (rml_context_)
	{
		rml_context_->SetDimensions(Rml::Vector2i(width_, height_));
	}
}

void UIManager::RenderIntoGameView()
{
	if ( !rml_context_ )
	{
		return;
	}
	
	rml_context_->Update();
	rml_context_->Render();
}

void UIManager::SetDebuggerVisible(bool visible)
{
	if (debugger_initialized_)
	{
		Rml::Debugger::SetVisible(visible);
	}
}

bool UIManager::IsDebuggerVisible() const
{
	return debugger_initialized_ && Rml::Debugger::IsVisible();
}

void UIManager::ProcessGameViewPointer(bool image_hovered, int mouse_x, int mouse_y, int key_modifier_state,
	bool left_pressed, bool left_released, bool right_pressed, bool right_released, bool middle_pressed,
	bool middle_released, float wheel_x, float wheel_y)
{
	if ( !rml_context_ )
	{
		return;
	}

	if (!image_hovered)
	{
		if (game_image_hovered_last_)
		{
			rml_context_->ProcessMouseLeave();
		}
		game_image_hovered_last_ = false;
		return;
	}

	game_image_hovered_last_ = true;

	int const mx = (std::clamp)(mouse_x, 0, width_ - 1);
	int const my = (std::clamp)(mouse_y, 0, height_ - 1);
	rml_context_->ProcessMouseMove(mx, my, key_modifier_state);

	if (left_pressed)
	{
		rml_context_->ProcessMouseButtonDown(0, key_modifier_state);
	}
	if (right_pressed)
	{
		rml_context_->ProcessMouseButtonDown(1, key_modifier_state);
	}
	if (middle_pressed)
	{
		rml_context_->ProcessMouseButtonDown(2, key_modifier_state);
	}

	if (left_released)
	{
		rml_context_->ProcessMouseButtonUp(0, key_modifier_state);
	}
	if (right_released)
	{
		rml_context_->ProcessMouseButtonUp(1, key_modifier_state);
	}
	if (middle_released)
	{
		rml_context_->ProcessMouseButtonUp(2, key_modifier_state);
	}

	if (wheel_x != 0.f || wheel_y != 0.f)
	{
		rml_context_->ProcessMouseWheel(Rml::Vector2f(wheel_x, wheel_y), key_modifier_state);
	}
}

} // namespace RenderWorker
