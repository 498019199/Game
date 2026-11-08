/**
 * @file SDL3Input.hpp
 *
 * @section DESCRIPTION
 *
 * SDL3 based input plugin. Keyboard / mouse / gamepad states are polled from
 * SDL every frame; SDL's event queue itself is drained by Window::PumpEvents,
 * so this plugin never calls SDL_PollEvent and can't steal events from it.
 */

#ifndef _SDL3INPUT_HPP
#define _SDL3INPUT_HPP

#pragma once

#include <base/Input.h>
#include <base/Window.h>

#include <array>
#include <vector>

struct SDL_Gamepad;

namespace RenderWorker
{
	class SDL3InputEngine final : public InputEngine
	{
	public:
		SDL3InputEngine();
		~SDL3InputEngine() override;

		std::wstring const& Name() const override;
		void EnumDevices() override;

	private:
		void DoSuspend() override;
		void DoResume() override;

		void OnPointerWheel(int2 const& pt, uint32_t id, int32_t wheel_delta);

	private:
		Signal::Connection on_pointer_wheel_;
		bool owns_gamepad_subsystem_{false};
	};

	class SDL3InputKeyboard final : public InputKeyboard
	{
	public:
		SDL3InputKeyboard();

		std::wstring const& Name() const override;

	private:
		void UpdateInputs() override;

	private:
		std::array<bool, 256> keys_state_{};
	};

	class SDL3InputMouse final : public InputMouse
	{
	public:
		SDL3InputMouse();

		std::wstring const& Name() const override;

		void OnPointerWheel(int32_t wheel_delta);

	private:
		void UpdateInputs() override;

	private:
		int2 last_abs_state_{0, 0};
		bool has_last_abs_{false};
		int32_t wheel_state_{0};
	};

	class SDL3InputJoystick final : public InputJoystick
	{
	public:
		explicit SDL3InputJoystick(SDL_Gamepad* gamepad);
		~SDL3InputJoystick() override;

		std::wstring const& Name() const override;

		void VibrationMotorSpeed(uint32_t n, float motor_speed) override;

	private:
		void UpdateInputs() override;

	private:
		SDL_Gamepad* gamepad_;
		std::wstring name_;
		std::array<uint16_t, 2> motor_speeds_{};
	};
}

#endif // _SDL3INPUT_HPP
