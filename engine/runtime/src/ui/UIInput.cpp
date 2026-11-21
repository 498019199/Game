#include <base/UIManager.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <RmlUi/Core.h>
#include <base/Input.h>
#include <array>
#include <vector>
#include <set>

namespace RenderWorker
{
namespace
{
Rml::Input::KeyIdentifier ConvertKey(int key)
{
    using namespace Rml::Input;
    switch (key)
    {
    case KS_A: return KI_A;
    case KS_B: return KI_B;
    case KS_C: return KI_C;
    case KS_D: return KI_D;
    case KS_E: return KI_E;
    case KS_F: return KI_F;
    case KS_G: return KI_G;
    case KS_H: return KI_H;
    case KS_I: return KI_I;
    case KS_J: return KI_J;
    case KS_K: return KI_K;
    case KS_L: return KI_L;
    case KS_M: return KI_M;
    case KS_N: return KI_N;
    case KS_O: return KI_O;
    case KS_P: return KI_P;
    case KS_Q: return KI_Q;
    case KS_R: return KI_R;
    case KS_S: return KI_S;
    case KS_T: return KI_T;
    case KS_U: return KI_U;
    case KS_V: return KI_V;
    case KS_W: return KI_W;
    case KS_X: return KI_X;
    case KS_Y: return KI_Y;
    case KS_Z: return KI_Z;
    case KS_0: return KI_0;
    case KS_1: return KI_1;
    case KS_2: return KI_2;
    case KS_3: return KI_3;
    case KS_4: return KI_4;
    case KS_5: return KI_5;
    case KS_6: return KI_6;
    case KS_7: return KI_7;
    case KS_8: return KI_8;
    case KS_9: return KI_9;
    case KS_BackSpace: return KI_BACK;
    case KS_Tab: return KI_TAB;
    case KS_Enter: return KI_RETURN;
    case KS_Pause: return KI_PAUSE;
    case KS_CapsLock: return KI_CAPITAL;
    case KS_Escape: return KI_ESCAPE;
    case KS_Space: return KI_SPACE;
    case KS_PageUp: return KI_PRIOR;
    case KS_PageDown: return KI_NEXT;
    case KS_End: return KI_END;
    case KS_Home: return KI_HOME;
    case KS_LeftArrow: return KI_LEFT;
    case KS_UpArrow: return KI_UP;
    case KS_RightArrow: return KI_RIGHT;
    case KS_DownArrow: return KI_DOWN;
    case KS_SysRQ: return KI_SNAPSHOT;
    case KS_Insert: return KI_INSERT;
    case KS_Delete: return KI_DELETE;
    case KS_LeftWin: return KI_LWIN;
    case KS_RightWin: return KI_RWIN;
    case KS_NumPad0: return KI_NUMPAD0;
    case KS_NumPad1: return KI_NUMPAD1;
    case KS_NumPad2: return KI_NUMPAD2;
    case KS_NumPad3: return KI_NUMPAD3;
    case KS_NumPad4: return KI_NUMPAD4;
    case KS_NumPad5: return KI_NUMPAD5;
    case KS_NumPad6: return KI_NUMPAD6;
    case KS_NumPad7: return KI_NUMPAD7;
    case KS_NumPad8: return KI_NUMPAD8;
    case KS_NumPad9: return KI_NUMPAD9;
    case KS_NumPadEnter: return KI_NUMPADENTER;
    case KS_NumPadStar: return KI_MULTIPLY;
    case KS_NumPadPlus: return KI_ADD;
    case KS_NumPadMinus: return KI_SUBTRACT;
    case KS_NumPadPeriod: return KI_DECIMAL;
    case KS_NumPadSlash: return KI_DIVIDE;
    case KS_F1: return KI_F1;
    case KS_F2: return KI_F2;
    case KS_F3: return KI_F3;
    case KS_F4: return KI_F4;
    case KS_F5: return KI_F5;
    case KS_F6: return KI_F6;
    case KS_F7: return KI_F7;
    case KS_F8: return KI_F8;
    case KS_F9: return KI_F9;
    case KS_F10: return KI_F10;
    case KS_F11: return KI_F11;
    case KS_F12: return KI_F12;
    case KS_F13: return KI_F13;
    case KS_F14: return KI_F14;
    case KS_F15: return KI_F15;
    case KS_NumLock: return KI_NUMLOCK;
    case KS_ScrollLock: return KI_SCROLL;
    case KS_LeftShift: return KI_LSHIFT;
    case KS_LeftCtrl: return KI_LCONTROL;
    case KS_RightShift: return KI_RSHIFT;
    case KS_RightCtrl: return KI_RCONTROL;
    case KS_Apps: return KI_LMENU;
    case KS_NumPadEquals: return KI_OEM_NEC_EQUAL;
    case KS_LeftAlt: return KI_LMENU;
    case KS_RightAlt: return KI_RMENU;
    case KS_Semicolon: return KI_OEM_1;
    case KS_Equals: return KI_OEM_PLUS;
    case KS_Comma: return KI_OEM_COMMA;
    case KS_Minus: return KI_OEM_MINUS;
    case KS_Period: return KI_OEM_PERIOD;
    case KS_Slash: return KI_OEM_2;
    case KS_Grave: return KI_OEM_3;
    case KS_LeftBracket: return KI_OEM_4;
    case KS_BackSlash: return KI_OEM_5;
    case KS_RightBracket: return KI_OEM_6;
    case KS_Apostrophe: return KI_OEM_7;
    case KS_OEM_102: return KI_OEM_102;
    default: return KI_UNKNOWN;
    }
}

int ConvertModifiers(int native)
{
    using namespace Rml::Input;
    int mods = 0;
    if (native & WindowInput::Shift) mods |= KM_SHIFT;
    if (native & WindowInput::Control) mods |= KM_CTRL;
    if (native & WindowInput::Alt) mods |= KM_ALT;
    if (native & WindowInput::Meta) mods |= KM_META;
    if (native & WindowInput::CapsLock) mods |= KM_CAPSLOCK;
    if (native & WindowInput::NumLock) mods |= KM_NUMLOCK;
    return mods;
}
}

struct UIInputState
{

    std::vector<WindowInput> events;
    std::function<bool(int)> key_handler;
    bool mouse = false, keyboard = false;
    bool block_mouse = false, block_keyboard = false;
    bool consumed_mouse = false, consumed_keyboard = false;
    std::array<bool, Window::KeyCount> held_keys{};
    unsigned held_buttons = 0;
    std::set<Rml::Input::KeyIdentifier> rml_keys;
    int modifiers = 0;
    float x = 0, y = 0, width = 1, height = 1;
    std::array<bool, 5> buttons{};
};

void UIManager::QueueInput(WindowInput const &event)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    input_state_->events.push_back(event);
}

void UIManager::SetInputViewport(bool mouse, bool keyboard, float x, float y, float width, float height)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    auto &s = *input_state_;
    s.mouse = mouse;
    s.keyboard = keyboard;
    s.x = x;
    s.y = y;
    s.width = width;
    s.height = height;
}

void UIManager::SetKeyHandler(std::function<bool(int)> handler)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    input_state_->key_handler = std::move(handler);
}

bool UIManager::GameKeyboardBlocked() const noexcept
{
    return input_state_ && (input_state_->block_keyboard || input_state_->consumed_keyboard);
}
bool UIManager::GamePointerBlocked() const noexcept
{
    return input_state_ && (input_state_->block_mouse || input_state_->consumed_mouse);
}
void UIManager::AcknowledgeGameInput() noexcept
{
    if (input_state_)
    {
        input_state_->consumed_mouse = false;
        input_state_->consumed_keyboard = false;
    }
}

void UIManager::RouteInput(bool editor_mode, bool imgui_exclusive)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    auto &s = *input_state_;
    auto *window = Context::Instance().AppValid() ? Context::Instance().AppInstance().MainWnd().get() : nullptr;
    bool const active = window && window->Active();
    bool const mouse = active && !imgui_exclusive && (!editor_mode || s.mouse);
    bool const keyboard = active && !imgui_exclusive && (!editor_mode || s.keyboard);
    s.block_mouse = !mouse;
    s.block_keyboard = !keyboard;
    if (!editor_mode && window)
    {
        s.x = s.y = 0;
        s.width = static_cast<float>(window->Width());
        s.height = static_cast<float>(window->Height());
    }
    if (rml_context_)
    {
        int mods = ConvertModifiers(s.modifiers);
        if (!keyboard)
        {
            for (auto key : s.rml_keys)
                rml_context_->ProcessKeyUp(key, mods);
            s.rml_keys.clear();
        }
        if (!mouse)
        {
            // Cancel a Rml drag before another UI takes ownership.
            rml_context_->ProcessMouseLeave();
            for (size_t i = 0; i < s.buttons.size(); ++i)
                if (s.buttons[i])
                {
                    rml_context_->ProcessMouseButtonUp(int(i), mods);
                    s.buttons[i] = false;
                }
        }
        bool suppress_console_text = false;
        for (auto const &e : s.events)
        {
            if (e.type != WindowInput::Character)
            {
                s.modifiers = e.modifiers;
                mods = ConvertModifiers(s.modifiers);
            }
            if (e.type == WindowInput::Key)
            {
                if (!keyboard)
                    continue;
                auto key = ConvertKey(e.key);
                if (e.action == WindowInput::Pressed && s.key_handler && s.key_handler(int(key)))
                {
                    s.block_keyboard = true;
                    suppress_console_text = e.key == KS_Grave;
                    continue;
                }
                if (key == Rml::Input::KI_UNKNOWN)
                    continue;
                if (e.action == WindowInput::Released)
                    s.rml_keys.erase(key);
                else
                    s.rml_keys.insert(key);
                bool propagated = e.action == WindowInput::Released ? rml_context_->ProcessKeyUp(key, mods)
                                                                    : rml_context_->ProcessKeyDown(key, mods);
                if (e.action != WindowInput::Released && (e.key == KS_Enter || e.key == KS_NumPadEnter))
                    propagated &= rml_context_->ProcessTextInput(U'\n');
                s.block_keyboard |= !propagated;
            }
            else if (e.type == WindowInput::Character)
            {
                if (keyboard && !(suppress_console_text && (e.codepoint == '`' || e.codepoint == '~')))
                    s.block_keyboard |= !rml_context_->ProcessTextInput(static_cast<Rml::Character>(e.codepoint));
            }
            else if (mouse)
            {
                bool propagated =
                    rml_context_->ProcessMouseMove(int((e.x - s.x) * width_ / std::max(s.width, 1.f)),
                                                   int((e.y - s.y) * height_ / std::max(s.height, 1.f)), mods);
                if (e.type == WindowInput::Button && e.key >= 0 && e.key < int(s.buttons.size()))
                {
                    s.buttons[e.key] = e.action == WindowInput::Pressed;
                    propagated &= s.buttons[e.key] ? rml_context_->ProcessMouseButtonDown(e.key, mods)
                                                   : rml_context_->ProcessMouseButtonUp(e.key, mods);
                }
                else if (e.type == WindowInput::Scroll)
                {
                    propagated &= rml_context_->ProcessMouseWheel(Rml::Vector2f(-e.horizontal, -e.vertical), mods);
                }
                if (!propagated)
                    s.block_mouse = true;
            }
        }
        auto *hover = rml_context_->GetHoverElement();
        mouse_on_ui_ = mouse && hover && hover != rml_context_->GetRootElement();
        auto *focus = rml_context_->GetFocusElement();
        bool const focused_ui = keyboard && focus && focus != rml_context_->GetRootElement() && focus->IsVisible();
        s.block_mouse |= mouse_on_ui_;
        s.block_keyboard |= focused_ui;
    }
    for (int i = 1; i < Window::KeyCount; ++i)
    {
        if (s.held_keys[i])
            s.consumed_keyboard = true;
        s.held_keys[i] = window && window->KeyDown(i) && (s.held_keys[i] || s.block_keyboard);
    }
    unsigned buttons = 0;
    for (int i = 0; window && i < Window::ButtonCount; ++i)
        if (window->ButtonDown(i))
            buttons |= 1u << i;
    if (s.held_buttons)
        s.consumed_mouse = true;
    s.held_buttons = buttons & (s.block_mouse ? ~0u : s.held_buttons);
    s.consumed_keyboard |= s.block_keyboard;
    s.consumed_mouse |= s.block_mouse;
    s.events.clear();
}
}
