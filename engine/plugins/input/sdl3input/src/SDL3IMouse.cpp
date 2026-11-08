/**
 * @file SDL3IMouse.cpp
 *
 * @section DESCRIPTION
 *
 * SDL3 mouse device. Buttons and position come from SDL_GetMouseState; the
 * relative motion is derived from consecutive absolute positions rather than
 * SDL_GetRelativeMouseState, which is a consuming read that other subsystems
 * (e.g. an ImGui backend) may also want.
 */

#include <base/ZEngine.h>

#include "SDL3Input.hpp"

#include <SDL3/SDL.h>

namespace RenderWorker
{
	SDL3InputMouse::SDL3InputMouse()
	{
		num_buttons_ = 5;
	}

	std::wstring const& SDL3InputMouse::Name() const
	{
		static std::wstring const name(L"SDL3 Mouse");
		return name;
	}

	void SDL3InputMouse::OnPointerWheel(int32_t wheel_delta)
	{
		wheel_state_ += wheel_delta;
	}

	void SDL3InputMouse::UpdateInputs()
	{
		float x = 0;
		float y = 0;
		SDL_MouseButtonFlags const button_mask = SDL_GetMouseState(&x, &y);

		int2 const abs_pos(static_cast<int32_t>(x), static_cast<int32_t>(y));
		if (!has_last_abs_)
		{
			last_abs_state_ = abs_pos;
			has_last_abs_ = true;
		}

		abs_pos_ = abs_pos;
		offset_ = int3(abs_pos.x() - last_abs_state_.x(), abs_pos.y() - last_abs_state_.y(), wheel_state_);
		last_abs_state_ = abs_pos;
		wheel_state_ = 0;

		std::array<bool, 8> buttons_state{};
		buttons_state[0] = (button_mask & SDL_BUTTON_LMASK) != 0;
		buttons_state[1] = (button_mask & SDL_BUTTON_RMASK) != 0;
		buttons_state[2] = (button_mask & SDL_BUTTON_MMASK) != 0;
		buttons_state[3] = (button_mask & SDL_BUTTON_X1MASK) != 0;
		buttons_state[4] = (button_mask & SDL_BUTTON_X2MASK) != 0;

		index_ = !index_;
		buttons_[index_] = buttons_state;

		SDL_Keymod const mod_state = SDL_GetModState();
		shift_ctrl_alt_ = static_cast<uint16_t>(((mod_state & SDL_KMOD_SHIFT) ? MB_Shift : 0) |
											   ((mod_state & SDL_KMOD_CTRL) ? MB_Ctrl : 0) |
											   ((mod_state & SDL_KMOD_ALT) ? MB_Alt : 0));
	}
}
