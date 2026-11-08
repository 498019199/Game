#include <base/UIManager.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SDL.h>
#include <array>
#include <vector>
#include <set>

namespace RenderWorker
{
struct UIInputState
{
	struct Event { SDL_Event value; std::string text; };
	std::vector<Event> events;
	std::function<bool(int)> key_handler;
	bool mouse = false, keyboard = false;
	bool block_mouse = false, block_keyboard = false;
	bool consumed_mouse = false, consumed_keyboard = false;
	std::array<bool, SDL_SCANCODE_COUNT> held_keys {};
	SDL_MouseButtonFlags held_buttons = 0;
	std::set<Rml::Input::KeyIdentifier> rml_keys;
	bool text_input_owned = false;
	float x = 0, y = 0, width = 1, height = 1;
	std::array<bool, 5> buttons {};
};

void UIManager::QueueInput(SDL_Event const& event)
{
	if (!input_state_) input_state_ = std::make_shared<UIInputState>();
	if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP &&
		event.type != SDL_EVENT_TEXT_INPUT && event.type != SDL_EVENT_MOUSE_MOTION &&
		event.type != SDL_EVENT_MOUSE_BUTTON_DOWN && event.type != SDL_EVENT_MOUSE_BUTTON_UP &&
		event.type != SDL_EVENT_MOUSE_WHEEL) return;
	// SDL owns text until the next poll; retain our own copy for deferred routing.
	input_state_->events.push_back({event, event.type == SDL_EVENT_TEXT_INPUT ? event.text.text : ""});
}

void UIManager::SetInputViewport(bool mouse, bool keyboard, float x, float y, float width, float height)
{
	if (!input_state_) input_state_ = std::make_shared<UIInputState>();
	auto& s = *input_state_;
	s.mouse = mouse; s.keyboard = keyboard;
	s.x = x; s.y = y; s.width = width; s.height = height;
}

void UIManager::SetKeyHandler(std::function<bool(int)> handler)
{
	if (!input_state_) input_state_ = std::make_shared<UIInputState>();
	input_state_->key_handler = std::move(handler);
}

bool UIManager::GameKeyboardBlocked() const noexcept { return input_state_ && (input_state_->block_keyboard || input_state_->consumed_keyboard); }
bool UIManager::GamePointerBlocked() const noexcept { return input_state_ && (input_state_->block_mouse || input_state_->consumed_mouse); }
void UIManager::AcknowledgeGameInput() noexcept
{
	if (input_state_) { input_state_->consumed_mouse = false; input_state_->consumed_keyboard = false; }
}

void UIManager::RouteInput(bool editor_mode, bool imgui_exclusive)
{
	if (!input_state_) input_state_ = std::make_shared<UIInputState>();
	auto& s = *input_state_;
	auto* window = Context::Instance().AppValid() ? Context::Instance().AppInstance().MainWnd().get() : nullptr;
	bool const active = window && window->Active();
	bool const mouse = active && !imgui_exclusive && (!editor_mode || s.mouse);
	bool const keyboard = active && !imgui_exclusive && (!editor_mode || s.keyboard);
	s.block_mouse = !mouse;
	s.block_keyboard = !keyboard;
	if (!editor_mode && window)
	{
		int w = 1, h = 1;
		SDL_GetWindowSize(window->GetSDLWindow(), &w, &h);
		s.x = s.y = 0; s.width = float(w); s.height = float(h);
	}
	if (rml_context_)
	{
		int const mods = RmlSDL::GetKeyModifierState();
		if (!keyboard)
		{
			for (auto key : s.rml_keys) rml_context_->ProcessKeyUp(key, mods);
			s.rml_keys.clear();
		}
		if (!mouse)
		{
			// Cancel a Rml drag before another UI takes ownership.
			rml_context_->ProcessMouseLeave();
			for (size_t i = 0; i < s.buttons.size(); ++i)
				if (s.buttons[i]) { rml_context_->ProcessMouseButtonUp(int(i), mods); s.buttons[i] = false; }
		}
		bool suppress_console_text = false;
		for (auto& pending : s.events)
		{
			auto& e = pending.value;
			if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP || e.type == SDL_EVENT_TEXT_INPUT)
			{
				if (!keyboard) continue;
				if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat && s.key_handler && s.key_handler(int(RmlSDL::ConvertKey(int(e.key.key)))))
				{ s.block_keyboard = true; suppress_console_text = e.key.key == SDLK_GRAVE; continue; }
				if (e.type == SDL_EVENT_TEXT_INPUT && suppress_console_text && (pending.text == "`" || pending.text == "~")) continue;
				if (e.type == SDL_EVENT_KEY_DOWN) s.rml_keys.insert(RmlSDL::ConvertKey(int(e.key.key)));
				if (e.type == SDL_EVENT_KEY_UP) s.rml_keys.erase(RmlSDL::ConvertKey(int(e.key.key)));
				if (e.type == SDL_EVENT_TEXT_INPUT) e.text.text = pending.text.c_str();
				if (!RmlSDL::InputEventHandler(rml_context_, window->GetSDLWindow(), e)) s.block_keyboard = true;
				continue;
			}
			if (!mouse) continue;
			float x = 0, y = 0;
			if (e.type == SDL_EVENT_MOUSE_MOTION) { x = e.motion.x; y = e.motion.y; }
			else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) { x = e.button.x; y = e.button.y; }
			else { x = e.wheel.mouse_x; y = e.wheel.mouse_y; }
			bool propagated = rml_context_->ProcessMouseMove(int((x - s.x) * width_ / (std::max)(s.width, 1.f)),
				int((y - s.y) * height_ / (std::max)(s.height, 1.f)), mods);
			if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP)
			{
				int const button = RmlSDL::ConvertMouseButton(e.button.button);
				if (button >= 0 && button < int(s.buttons.size()))
				{
					s.buttons[button] = e.type == SDL_EVENT_MOUSE_BUTTON_DOWN;
					propagated &= s.buttons[button] ? rml_context_->ProcessMouseButtonDown(button, mods) : rml_context_->ProcessMouseButtonUp(button, mods);
				}
			}
			else if (e.type == SDL_EVENT_MOUSE_WHEEL)
			{
				float const direction = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? 1.f : -1.f;
				propagated &= rml_context_->ProcessMouseWheel(Rml::Vector2f(e.wheel.x * direction, e.wheel.y * direction), mods);
			}
			if (!propagated) s.block_mouse = true;
		}
		auto* hover = rml_context_->GetHoverElement();
		mouse_on_ui_ = mouse && hover && hover != rml_context_->GetRootElement();
		auto* focus = rml_context_->GetFocusElement();
		bool const focused_ui = keyboard && focus && focus != rml_context_->GetRootElement() && focus->IsVisible();
		s.block_mouse |= mouse_on_ui_;
		s.block_keyboard |= focused_ui;
		if (focused_ui) { SDL_StartTextInput(window->GetSDLWindow()); s.text_input_owned = true; }
		else if (s.text_input_owned)
		{
			// ImGui may already own the text session this frame.
			if (window && !imgui_exclusive) SDL_StopTextInput(window->GetSDLWindow());
			s.text_input_owned = false;
		}
	}
	// Keep UI-owned holds from turning into gameplay presses when focus changes.
	int count = 0;
	auto const* keys = SDL_GetKeyboardState(&count);
	for (int i = 0; i < count && i < SDL_SCANCODE_COUNT; ++i)
	{
		if (s.held_keys[i]) s.consumed_keyboard = true;
		s.held_keys[i] = keys[i] && (s.held_keys[i] || s.block_keyboard);
	}
	auto const buttons = SDL_GetMouseState(nullptr, nullptr);
	if (s.held_buttons) s.consumed_mouse = true;
	s.held_buttons = buttons & (s.block_mouse ? ~SDL_MouseButtonFlags(0) : s.held_buttons);
	s.consumed_keyboard |= s.block_keyboard;
	s.consumed_mouse |= s.block_mouse;
	s.events.clear();
}
}
