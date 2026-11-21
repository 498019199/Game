#include <base/ZEngine.h>

#include "DesktopInput.hpp"

#include <base/Window.h>
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
    auto const* window = ctx.AppValid() ? ctx.AppInstance().MainWnd().get() : nullptr;
    int2 const abs_pos = window ? window->PointerPosition() : int2(0, 0);
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
    for (int i = 0; window && i < num_buttons_; ++i)
        buttons_state[i] = window->ButtonDown(i);
    index_ = !index_;
    buttons_[index_] = buttons_state;

    auto held = [window](int l, int r) { return window && (window->KeyDown(l) || window->KeyDown(r)); };
    shift_ctrl_alt_ = (held(KS_LeftShift, KS_RightShift) ? MB_Shift : 0) |
                      (held(KS_LeftCtrl, KS_RightCtrl) ? MB_Ctrl : 0) |
                      (held(KS_LeftAlt, KS_RightAlt) ? MB_Alt : 0);
}
}
