/**
 * @file SDL3IKeyboard.cpp
 *
 * @section DESCRIPTION
 *
 * SDL3 keyboard device. State is polled from SDL_GetKeyboardState and
 * translated from USB HID scancodes into KeyboardSemantic.
 */

#include <base/ZEngine.h>

#include "SDL3Input.hpp"

#include <SDL3/SDL.h>

#include <algorithm>

namespace
{
	using namespace RenderWorker;

	using ScancodeMapping = std::array<uint8_t, SDL_SCANCODE_COUNT>;

	ScancodeMapping BuildScancodeMapping()
	{
		ScancodeMapping mapping{};

		auto set = [&mapping](SDL_Scancode sc, KeyboardSemantic ks) { mapping[sc] = static_cast<uint8_t>(ks); };

		set(SDL_SCANCODE_A, KS_A);
		set(SDL_SCANCODE_B, KS_B);
		set(SDL_SCANCODE_C, KS_C);
		set(SDL_SCANCODE_D, KS_D);
		set(SDL_SCANCODE_E, KS_E);
		set(SDL_SCANCODE_F, KS_F);
		set(SDL_SCANCODE_G, KS_G);
		set(SDL_SCANCODE_H, KS_H);
		set(SDL_SCANCODE_I, KS_I);
		set(SDL_SCANCODE_J, KS_J);
		set(SDL_SCANCODE_K, KS_K);
		set(SDL_SCANCODE_L, KS_L);
		set(SDL_SCANCODE_M, KS_M);
		set(SDL_SCANCODE_N, KS_N);
		set(SDL_SCANCODE_O, KS_O);
		set(SDL_SCANCODE_P, KS_P);
		set(SDL_SCANCODE_Q, KS_Q);
		set(SDL_SCANCODE_R, KS_R);
		set(SDL_SCANCODE_S, KS_S);
		set(SDL_SCANCODE_T, KS_T);
		set(SDL_SCANCODE_U, KS_U);
		set(SDL_SCANCODE_V, KS_V);
		set(SDL_SCANCODE_W, KS_W);
		set(SDL_SCANCODE_X, KS_X);
		set(SDL_SCANCODE_Y, KS_Y);
		set(SDL_SCANCODE_Z, KS_Z);

		set(SDL_SCANCODE_1, KS_1);
		set(SDL_SCANCODE_2, KS_2);
		set(SDL_SCANCODE_3, KS_3);
		set(SDL_SCANCODE_4, KS_4);
		set(SDL_SCANCODE_5, KS_5);
		set(SDL_SCANCODE_6, KS_6);
		set(SDL_SCANCODE_7, KS_7);
		set(SDL_SCANCODE_8, KS_8);
		set(SDL_SCANCODE_9, KS_9);
		set(SDL_SCANCODE_0, KS_0);

		set(SDL_SCANCODE_RETURN, KS_Enter);
		set(SDL_SCANCODE_ESCAPE, KS_Escape);
		set(SDL_SCANCODE_BACKSPACE, KS_BackSpace);
		set(SDL_SCANCODE_TAB, KS_Tab);
		set(SDL_SCANCODE_SPACE, KS_Space);

		set(SDL_SCANCODE_MINUS, KS_Minus);
		set(SDL_SCANCODE_EQUALS, KS_Equals);
		set(SDL_SCANCODE_LEFTBRACKET, KS_LeftBracket);
		set(SDL_SCANCODE_RIGHTBRACKET, KS_RightBracket);
		set(SDL_SCANCODE_BACKSLASH, KS_BackSlash);
		set(SDL_SCANCODE_NONUSHASH, KS_BackSlash);
		set(SDL_SCANCODE_SEMICOLON, KS_Semicolon);
		set(SDL_SCANCODE_APOSTROPHE, KS_Apostrophe);
		set(SDL_SCANCODE_GRAVE, KS_Grave);
		set(SDL_SCANCODE_COMMA, KS_Comma);
		set(SDL_SCANCODE_PERIOD, KS_Period);
		set(SDL_SCANCODE_SLASH, KS_Slash);
		set(SDL_SCANCODE_NONUSBACKSLASH, KS_OEM_102);

		set(SDL_SCANCODE_CAPSLOCK, KS_CapsLock);

		set(SDL_SCANCODE_F1, KS_F1);
		set(SDL_SCANCODE_F2, KS_F2);
		set(SDL_SCANCODE_F3, KS_F3);
		set(SDL_SCANCODE_F4, KS_F4);
		set(SDL_SCANCODE_F5, KS_F5);
		set(SDL_SCANCODE_F6, KS_F6);
		set(SDL_SCANCODE_F7, KS_F7);
		set(SDL_SCANCODE_F8, KS_F8);
		set(SDL_SCANCODE_F9, KS_F9);
		set(SDL_SCANCODE_F10, KS_F10);
		set(SDL_SCANCODE_F11, KS_F11);
		set(SDL_SCANCODE_F12, KS_F12);
		set(SDL_SCANCODE_F13, KS_F13);
		set(SDL_SCANCODE_F14, KS_F14);
		set(SDL_SCANCODE_F15, KS_F15);

		set(SDL_SCANCODE_PRINTSCREEN, KS_SysRQ);
		set(SDL_SCANCODE_SCROLLLOCK, KS_ScrollLock);
		set(SDL_SCANCODE_PAUSE, KS_Pause);
		set(SDL_SCANCODE_INSERT, KS_Insert);
		set(SDL_SCANCODE_HOME, KS_Home);
		set(SDL_SCANCODE_PAGEUP, KS_PageUp);
		set(SDL_SCANCODE_DELETE, KS_Delete);
		set(SDL_SCANCODE_END, KS_End);
		set(SDL_SCANCODE_PAGEDOWN, KS_PageDown);
		set(SDL_SCANCODE_RIGHT, KS_RightArrow);
		set(SDL_SCANCODE_LEFT, KS_LeftArrow);
		set(SDL_SCANCODE_DOWN, KS_DownArrow);
		set(SDL_SCANCODE_UP, KS_UpArrow);

		set(SDL_SCANCODE_NUMLOCKCLEAR, KS_NumLock);
		set(SDL_SCANCODE_KP_DIVIDE, KS_NumPadSlash);
		set(SDL_SCANCODE_KP_MULTIPLY, KS_NumPadStar);
		set(SDL_SCANCODE_KP_MINUS, KS_NumPadMinus);
		set(SDL_SCANCODE_KP_PLUS, KS_NumPadPlus);
		set(SDL_SCANCODE_KP_ENTER, KS_NumPadEnter);
		set(SDL_SCANCODE_KP_1, KS_NumPad1);
		set(SDL_SCANCODE_KP_2, KS_NumPad2);
		set(SDL_SCANCODE_KP_3, KS_NumPad3);
		set(SDL_SCANCODE_KP_4, KS_NumPad4);
		set(SDL_SCANCODE_KP_5, KS_NumPad5);
		set(SDL_SCANCODE_KP_6, KS_NumPad6);
		set(SDL_SCANCODE_KP_7, KS_NumPad7);
		set(SDL_SCANCODE_KP_8, KS_NumPad8);
		set(SDL_SCANCODE_KP_9, KS_NumPad9);
		set(SDL_SCANCODE_KP_0, KS_NumPad0);
		set(SDL_SCANCODE_KP_PERIOD, KS_NumPadPeriod);
		set(SDL_SCANCODE_KP_EQUALS, KS_NumPadEquals);
		set(SDL_SCANCODE_KP_COMMA, KS_NumPadComma);

		set(SDL_SCANCODE_LCTRL, KS_LeftCtrl);
		set(SDL_SCANCODE_LSHIFT, KS_LeftShift);
		set(SDL_SCANCODE_LALT, KS_LeftAlt);
		set(SDL_SCANCODE_LGUI, KS_LeftWin);
		set(SDL_SCANCODE_RCTRL, KS_RightCtrl);
		set(SDL_SCANCODE_RSHIFT, KS_RightShift);
		set(SDL_SCANCODE_RALT, KS_RightAlt);
		set(SDL_SCANCODE_RGUI, KS_RightWin);

		set(SDL_SCANCODE_APPLICATION, KS_Apps);
		set(SDL_SCANCODE_POWER, KS_Power);
		set(SDL_SCANCODE_SLEEP, KS_Sleep);
		set(SDL_SCANCODE_STOP, KS_Stop);
		set(SDL_SCANCODE_MUTE, KS_Mute);
		set(SDL_SCANCODE_VOLUMEUP, KS_VolumeUp);
		set(SDL_SCANCODE_VOLUMEDOWN, KS_VolumeDown);
		set(SDL_SCANCODE_MEDIA_PLAY_PAUSE, KS_PlayPause);
		set(SDL_SCANCODE_MEDIA_STOP, KS_MediaStop);
		set(SDL_SCANCODE_MEDIA_NEXT_TRACK, KS_NextTrack);
		set(SDL_SCANCODE_MEDIA_PREVIOUS_TRACK, KS_PrevTrack);
		set(SDL_SCANCODE_MEDIA_SELECT, KS_MediaSelect);
		set(SDL_SCANCODE_AC_SEARCH, KS_WebSearch);
		set(SDL_SCANCODE_AC_HOME, KS_WebHome);
		set(SDL_SCANCODE_AC_BACK, KS_WebBack);
		set(SDL_SCANCODE_AC_FORWARD, KS_WebForward);
		set(SDL_SCANCODE_AC_STOP, KS_WebStop);
		set(SDL_SCANCODE_AC_REFRESH, KS_WebRefresh);
		set(SDL_SCANCODE_AC_BOOKMARKS, KS_WebFavorites);

		set(SDL_SCANCODE_INTERNATIONAL1, KS_ABNT_C1);
		set(SDL_SCANCODE_INTERNATIONAL3, KS_Yen);
		set(SDL_SCANCODE_INTERNATIONAL4, KS_Convert);
		set(SDL_SCANCODE_INTERNATIONAL5, KS_NoConvert);
		set(SDL_SCANCODE_LANG1, KS_Kanji);
		set(SDL_SCANCODE_LANG2, KS_Kana);

		return mapping;
	}

	ScancodeMapping const& Scancode2KS()
	{
		static ScancodeMapping const mapping = BuildScancodeMapping();
		return mapping;
	}
} // namespace

namespace RenderWorker
{
	SDL3InputKeyboard::SDL3InputKeyboard()
	{
		keys_state_.fill(false);
	}

	std::wstring const& SDL3InputKeyboard::Name() const
	{
		static std::wstring const name(L"SDL3 Keyboard");
		return name;
	}

	void SDL3InputKeyboard::UpdateInputs()
	{
		keys_state_.fill(false);

		int num_keys = 0;
		bool const* sdl_keys = SDL_GetKeyboardState(&num_keys);
		if (sdl_keys != nullptr)
		{
			auto const& mapping = Scancode2KS();
			int const count = std::min(num_keys, static_cast<int>(mapping.size()));
			for (int sc = 0; sc < count; ++sc)
			{
				uint8_t const ks = mapping[sc];
				if ((ks != 0) && sdl_keys[sc])
				{
					keys_state_[ks] = true;
				}
			}
		}

		index_ = !index_;
		keys_[index_] = keys_state_;
	}
}
