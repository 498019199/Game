/**
 * @file SDL3IJoystick.cpp
 *
 * @section DESCRIPTION
 *
 * SDL3 gamepad device. The axis ranges, dead zones and button ordering mirror
 * the existing gamepad action layout, so existing action maps keep working unchanged.
 */

#include <base/ZEngine.h>

#include "SDL3Input.hpp"

#include <SDL3/SDL.h>

#include <algorithm>

namespace
{
	int16_t constexpr LEFT_THUMB_DEADZONE = 7849;
	int16_t constexpr RIGHT_THUMB_DEADZONE = 8689;
	int16_t constexpr TRIGGER_THRESHOLD = 30 * 32767 / 255;

	// SDL's Y axis grows downwards, XInput's grows upwards.
	float ThumbAxis(int16_t value, int16_t deadzone, float sign)
	{
		if ((value > +deadzone) || (value < -deadzone))
		{
			return sign * std::clamp(value / 32768.0f, -1.0f, 1.0f);
		}
		return 0;
	}

	constexpr SDL_GamepadButton BUTTON_MAPPING[] = {
		SDL_GAMEPAD_BUTTON_DPAD_UP,
		SDL_GAMEPAD_BUTTON_DPAD_DOWN,
		SDL_GAMEPAD_BUTTON_DPAD_LEFT,
		SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
		SDL_GAMEPAD_BUTTON_START,
		SDL_GAMEPAD_BUTTON_BACK,
		SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
		SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
		SDL_GAMEPAD_BUTTON_SOUTH,
		SDL_GAMEPAD_BUTTON_EAST,
		SDL_GAMEPAD_BUTTON_WEST,
		SDL_GAMEPAD_BUTTON_NORTH,
	};
} // namespace

namespace RenderWorker
{
	SDL3InputJoystick::SDL3InputJoystick(SDL_Gamepad* gamepad) : gamepad_(gamepad)
	{
		num_buttons_ = static_cast<uint32_t>(std::size(BUTTON_MAPPING));
		num_vibration_motors_ = 2;

		char const* sdl_name = gamepad_ ? SDL_GetGamepadName(gamepad_) : nullptr;
		std::string const narrow_name = sdl_name ? sdl_name : "SDL3 Gamepad";
		name_.assign(narrow_name.begin(), narrow_name.end());
	}

	SDL3InputJoystick::~SDL3InputJoystick()
	{
		if (gamepad_)
		{
			SDL_CloseGamepad(gamepad_);
			gamepad_ = nullptr;
		}
	}

	std::wstring const& SDL3InputJoystick::Name() const
	{
		return name_;
	}

	void SDL3InputJoystick::VibrationMotorSpeed(uint32_t n, float motor_speed)
	{
		if (!gamepad_ || (n >= motor_speeds_.size()))
		{
			return;
		}

		motor_speeds_[n] = static_cast<uint16_t>(std::clamp(motor_speed, 0.0f, 1.0f) * 65535.0f);
		SDL_RumbleGamepad(gamepad_, motor_speeds_[0], motor_speeds_[1], 0);
	}

	void SDL3InputJoystick::UpdateInputs()
	{
		index_ = !index_;

		thumbs_.fill(float3(0, 0, 0));
		triggers_.fill(0);
		buttons_[index_].fill(false);

		if (!gamepad_)
		{
			return;
		}

		thumbs_[0].x() = ThumbAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTX), LEFT_THUMB_DEADZONE, +1.0f);
		thumbs_[0].y() = ThumbAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFTY), LEFT_THUMB_DEADZONE, -1.0f);
		thumbs_[1].x() = ThumbAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHTX), RIGHT_THUMB_DEADZONE, +1.0f);
		thumbs_[1].y() = ThumbAxis(SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHTY), RIGHT_THUMB_DEADZONE, -1.0f);

		if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_LEFT_STICK))
		{
			thumbs_[0].z() = 1.0f;
		}
		if (SDL_GetGamepadButton(gamepad_, SDL_GAMEPAD_BUTTON_RIGHT_STICK))
		{
			thumbs_[1].z() = 1.0f;
		}

		int16_t const left_trigger = SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
		if (left_trigger > TRIGGER_THRESHOLD)
		{
			triggers_[0] = std::clamp(left_trigger / 32767.0f, 0.0f, 1.0f);
		}
		int16_t const right_trigger = SDL_GetGamepadAxis(gamepad_, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
		if (right_trigger > TRIGGER_THRESHOLD)
		{
			triggers_[1] = std::clamp(right_trigger / 32767.0f, 0.0f, 1.0f);
		}

		for (size_t i = 0; i < std::size(BUTTON_MAPPING); ++i)
		{
			buttons_[index_][i] = SDL_GetGamepadButton(gamepad_, BUTTON_MAPPING[i]);
		}
	}
}
