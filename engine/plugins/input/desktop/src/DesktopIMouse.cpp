#include <base/ZEngine.h>

#include "DesktopInput.hpp"

#include <GLFW/glfw3.h>
#include <base/App3D.h>

namespace RenderWorker
{
DesktopInputMouse::DesktopInputMouse()
{
    num_buttons_ = 5;
}

std::wstring const &DesktopInputMouse::Name() const
{
    static std::wstring const name(L"Desktop Mouse");
    return name;
}

void DesktopInputMouse::OnPointerWheel(int32_t wheel_delta)
{
    wheel_state_ += wheel_delta;
}

void DesktopInputMouse::UpdateInputs()
{
    auto &ctx = Context::Instance();
    auto *w = ctx.AppValid() ? ctx.AppInstance().MainWnd()->DesktopWindow() : nullptr;
    double x = 0, y = 0;
    if (w)
        glfwGetCursorPos(w, &x, &y);
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
    for (int i = 0; w && i < 5; ++i)
        buttons_state[i] = glfwGetMouseButton(w, i);
    index_ = !index_;
    buttons_[index_] = buttons_state;

    auto held = [w](int l, int r) { return w && (glfwGetKey(w, l) || glfwGetKey(w, r)); };
    shift_ctrl_alt_ = (held(GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT) ? MB_Shift : 0) |
                      (held(GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL) ? MB_Ctrl : 0) |
                      (held(GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT) ? MB_Alt : 0);
}
}
