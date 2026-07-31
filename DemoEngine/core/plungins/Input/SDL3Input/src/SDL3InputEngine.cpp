/**
 * @file SDL3InputEngine.cpp
 *
 * @section DESCRIPTION
 *
 * SDL3 input engine. Devices poll SDL's cached state, which Window::PumpEvents
 * keeps fresh by draining the event queue once per frame. The only piece of
 * input that has no pollable state is the wheel, so it is accumulated from the
 * window's pointer wheel signal.
 */

#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Context.h>
#include <base/Window.h>

#include "SDL3Input.hpp"

#include <SDL3/SDL.h>

namespace RenderWorker
{
	SDL3InputEngine::SDL3InputEngine()
	{
		if (!SDL_WasInit(SDL_INIT_GAMEPAD))
		{
			if (SDL_InitSubSystem(SDL_INIT_GAMEPAD))
			{
				owns_gamepad_subsystem_ = true;
			}
			else
			{
				LogError() << "[SDL3Input] SDL_InitSubSystem(GAMEPAD) failed: " << SDL_GetError() << std::endl;
			}
		}
	}

	SDL3InputEngine::~SDL3InputEngine()
	{
		on_pointer_wheel_.Disconnect();
		devices_.clear();

		if (owns_gamepad_subsystem_)
		{
			SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
			owns_gamepad_subsystem_ = false;
		}
	}

	std::wstring const& SDL3InputEngine::Name() const
	{
		static std::wstring const name(L"SDL3 Input Engine");
		return name;
	}

	void SDL3InputEngine::EnumDevices()
	{
		devices_.push_back(MakeSharedPtr<SDL3InputKeyboard>());
		devices_.push_back(MakeSharedPtr<SDL3InputMouse>());

		int num_gamepads = 0;
		if (SDL_JoystickID* gamepad_ids = SDL_GetGamepads(&num_gamepads))
		{
			for (int i = 0; i < num_gamepads; ++i)
			{
				if (SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepad_ids[i]))
				{
					devices_.push_back(MakeSharedPtr<SDL3InputJoystick>(gamepad));
				}
			}
			SDL_free(gamepad_ids);
		}

		if (Context::Instance().AppValid())
		{
			if (WindowPtr const& main_wnd = Context::Instance().AppInstance().MainWnd())
			{
				on_pointer_wheel_ = main_wnd->OnPointerWheel().Connect(
					[this]([[maybe_unused]] Window const& wnd, int2 const& pt, uint32_t id, int32_t wheel_delta) {
						this->OnPointerWheel(pt, id, wheel_delta);
					});
			}
		}
	}

	void SDL3InputEngine::OnPointerWheel(
		[[maybe_unused]] int2 const& pt, [[maybe_unused]] uint32_t id, int32_t wheel_delta)
	{
		for (auto const& device : devices_)
		{
			if (InputEngine::IDT_Mouse == device->Type())
			{
				checked_pointer_cast<SDL3InputMouse>(device)->OnPointerWheel(wheel_delta);
			}
		}
	}

	void SDL3InputEngine::DoSuspend()
	{
	}

	void SDL3InputEngine::DoResume()
	{
	}
}
