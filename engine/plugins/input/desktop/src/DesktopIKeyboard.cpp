#include <base/ZEngine.h>

#include "DesktopInput.hpp"

#include <GLFW/glfw3.h>
#include <base/App3D.h>

#include <algorithm>

namespace
{
using namespace RenderWorker;

using ScancodeMapping = std::array<uint8_t, GLFW_KEY_LAST + 1>;

ScancodeMapping BuildScancodeMapping()
{
    ScancodeMapping mapping{};

    auto set = [&mapping](int sc, KeyboardSemantic ks) { mapping[sc] = static_cast<uint8_t>(ks); };

    set(GLFW_KEY_A, KS_A);
    set(GLFW_KEY_B, KS_B);
    set(GLFW_KEY_C, KS_C);
    set(GLFW_KEY_D, KS_D);
    set(GLFW_KEY_E, KS_E);
    set(GLFW_KEY_F, KS_F);
    set(GLFW_KEY_G, KS_G);
    set(GLFW_KEY_H, KS_H);
    set(GLFW_KEY_I, KS_I);
    set(GLFW_KEY_J, KS_J);
    set(GLFW_KEY_K, KS_K);
    set(GLFW_KEY_L, KS_L);
    set(GLFW_KEY_M, KS_M);
    set(GLFW_KEY_N, KS_N);
    set(GLFW_KEY_O, KS_O);
    set(GLFW_KEY_P, KS_P);
    set(GLFW_KEY_Q, KS_Q);
    set(GLFW_KEY_R, KS_R);
    set(GLFW_KEY_S, KS_S);
    set(GLFW_KEY_T, KS_T);
    set(GLFW_KEY_U, KS_U);
    set(GLFW_KEY_V, KS_V);
    set(GLFW_KEY_W, KS_W);
    set(GLFW_KEY_X, KS_X);
    set(GLFW_KEY_Y, KS_Y);
    set(GLFW_KEY_Z, KS_Z);

    set(GLFW_KEY_1, KS_1);
    set(GLFW_KEY_2, KS_2);
    set(GLFW_KEY_3, KS_3);
    set(GLFW_KEY_4, KS_4);
    set(GLFW_KEY_5, KS_5);
    set(GLFW_KEY_6, KS_6);
    set(GLFW_KEY_7, KS_7);
    set(GLFW_KEY_8, KS_8);
    set(GLFW_KEY_9, KS_9);
    set(GLFW_KEY_0, KS_0);

    set(GLFW_KEY_ENTER, KS_Enter);
    set(GLFW_KEY_ESCAPE, KS_Escape);
    set(GLFW_KEY_BACKSPACE, KS_BackSpace);
    set(GLFW_KEY_TAB, KS_Tab);
    set(GLFW_KEY_SPACE, KS_Space);

    set(GLFW_KEY_MINUS, KS_Minus);
    set(GLFW_KEY_EQUAL, KS_Equals);
    set(GLFW_KEY_LEFT_BRACKET, KS_LeftBracket);
    set(GLFW_KEY_RIGHT_BRACKET, KS_RightBracket);
    set(GLFW_KEY_BACKSLASH, KS_BackSlash);
    set(GLFW_KEY_SEMICOLON, KS_Semicolon);
    set(GLFW_KEY_APOSTROPHE, KS_Apostrophe);
    set(GLFW_KEY_GRAVE_ACCENT, KS_Grave);
    set(GLFW_KEY_COMMA, KS_Comma);
    set(GLFW_KEY_PERIOD, KS_Period);
    set(GLFW_KEY_SLASH, KS_Slash);

    set(GLFW_KEY_CAPS_LOCK, KS_CapsLock);

    set(GLFW_KEY_F1, KS_F1);
    set(GLFW_KEY_F2, KS_F2);
    set(GLFW_KEY_F3, KS_F3);
    set(GLFW_KEY_F4, KS_F4);
    set(GLFW_KEY_F5, KS_F5);
    set(GLFW_KEY_F6, KS_F6);
    set(GLFW_KEY_F7, KS_F7);
    set(GLFW_KEY_F8, KS_F8);
    set(GLFW_KEY_F9, KS_F9);
    set(GLFW_KEY_F10, KS_F10);
    set(GLFW_KEY_F11, KS_F11);
    set(GLFW_KEY_F12, KS_F12);
    set(GLFW_KEY_F13, KS_F13);
    set(GLFW_KEY_F14, KS_F14);
    set(GLFW_KEY_F15, KS_F15);

    set(GLFW_KEY_PRINT_SCREEN, KS_SysRQ);
    set(GLFW_KEY_SCROLL_LOCK, KS_ScrollLock);
    set(GLFW_KEY_PAUSE, KS_Pause);
    set(GLFW_KEY_INSERT, KS_Insert);
    set(GLFW_KEY_HOME, KS_Home);
    set(GLFW_KEY_PAGE_UP, KS_PageUp);
    set(GLFW_KEY_DELETE, KS_Delete);
    set(GLFW_KEY_END, KS_End);
    set(GLFW_KEY_PAGE_DOWN, KS_PageDown);
    set(GLFW_KEY_RIGHT, KS_RightArrow);
    set(GLFW_KEY_LEFT, KS_LeftArrow);
    set(GLFW_KEY_DOWN, KS_DownArrow);
    set(GLFW_KEY_UP, KS_UpArrow);

    set(GLFW_KEY_NUM_LOCK, KS_NumLock);
    set(GLFW_KEY_KP_DIVIDE, KS_NumPadSlash);
    set(GLFW_KEY_KP_MULTIPLY, KS_NumPadStar);
    set(GLFW_KEY_KP_SUBTRACT, KS_NumPadMinus);
    set(GLFW_KEY_KP_ADD, KS_NumPadPlus);
    set(GLFW_KEY_KP_ENTER, KS_NumPadEnter);
    set(GLFW_KEY_KP_1, KS_NumPad1);
    set(GLFW_KEY_KP_2, KS_NumPad2);
    set(GLFW_KEY_KP_3, KS_NumPad3);
    set(GLFW_KEY_KP_4, KS_NumPad4);
    set(GLFW_KEY_KP_5, KS_NumPad5);
    set(GLFW_KEY_KP_6, KS_NumPad6);
    set(GLFW_KEY_KP_7, KS_NumPad7);
    set(GLFW_KEY_KP_8, KS_NumPad8);
    set(GLFW_KEY_KP_9, KS_NumPad9);
    set(GLFW_KEY_KP_0, KS_NumPad0);
    set(GLFW_KEY_KP_DECIMAL, KS_NumPadPeriod);
    set(GLFW_KEY_KP_EQUAL, KS_NumPadEquals);

    set(GLFW_KEY_LEFT_CONTROL, KS_LeftCtrl);
    set(GLFW_KEY_LEFT_SHIFT, KS_LeftShift);
    set(GLFW_KEY_LEFT_ALT, KS_LeftAlt);
    set(GLFW_KEY_LEFT_SUPER, KS_LeftWin);
    set(GLFW_KEY_RIGHT_CONTROL, KS_RightCtrl);
    set(GLFW_KEY_RIGHT_SHIFT, KS_RightShift);
    set(GLFW_KEY_RIGHT_ALT, KS_RightAlt);
    set(GLFW_KEY_RIGHT_SUPER, KS_RightWin);

    set(GLFW_KEY_MENU, KS_Apps);

    return mapping;
}

ScancodeMapping const &Scancode2KS()
{
    static ScancodeMapping const mapping = BuildScancodeMapping();
    return mapping;
}
} // namespace

namespace RenderWorker
{
DesktopInputKeyboard::DesktopInputKeyboard()
{
    keys_state_.fill(false);
}

std::wstring const &DesktopInputKeyboard::Name() const
{
    static std::wstring const name(L"Desktop Keyboard");
    return name;
}

void DesktopInputKeyboard::UpdateInputs()
{
    keys_state_.fill(false);

    auto &ctx = Context::Instance();
    auto *w = ctx.AppValid() ? ctx.AppInstance().MainWnd()->DesktopWindow() : nullptr;
    auto const &mapping = Scancode2KS();
    for (int key = GLFW_KEY_SPACE; w && key <= GLFW_KEY_LAST; ++key)
        if (mapping[key] && glfwGetKey(w, key))
            keys_state_[mapping[key]] = true;

    index_ = !index_;
    keys_[index_] = keys_state_;
}
}
