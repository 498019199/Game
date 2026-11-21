#include <base/Window.h>
#include <base/App3D.h>
#include <base/UIManager.h>

namespace RenderWorker
{
void Window::UpdateDpiScale(float scale)
{
    dpi_scale_ = scale;
    float const limit = Context::Instance().Config().graphics_cfg.max_dpi_scale;
    effective_dpi_scale_ = limit > 0 ? std::min(limit, scale) : scale;
}

bool Window::KeyDown(int key) const
{
    return active_ && key > 0 && key < KeyCount && keys_[key];
}

bool Window::ButtonDown(int button) const
{
    return active_ && button >= 0 && button < ButtonCount && buttons_[button];
}

void Window::Active(bool active)
{
    active_ = active;
    if (!active)
    {
        keys_.fill(false);
        buttons_.fill(false);
        pending_surrogate_ = 0;
        wheel_remainder_ = 0;
    }
}

bool Window::RequestClose()
{
    auto& ctx = Context::Instance();
    if (closed_ || (ctx.AppValid() && !ctx.AppInstance().OnCloseRequested()))
        return false;

    Active(false);
    closed_ = true;
    OnClose()(*this);
    return true;
}

void Window::DispatchInput(WindowInput const& event)
{
    if (!ready_ || closed_)
        return;
    if (event.type == WindowInput::Key)
    {
        if (event.key <= 0 || event.key >= KeyCount)
            return;
        keys_[event.key] = event.action != WindowInput::Released;
    }
    else if (event.type == WindowInput::Button)
    {
        if (event.key < 0 || event.key >= ButtonCount)
            return;
        buttons_[event.key] = event.action == WindowInput::Pressed;
    }

    auto& ctx = Context::Instance();
    if (ctx.AppValid() && ctx.AppInstance().MainWnd().get() == this)
        ctx.UIManagerInstance().QueueInput(event);

    if (event.type == WindowInput::Motion || event.type == WindowInput::Button || event.type == WindowInput::Scroll)
        pointer_position_ = int2(static_cast<int>(event.x), static_cast<int>(event.y));

    switch (event.type)
    {
    case WindowInput::Motion:
        OnPointerUpdate()(*this, pointer_position_, 0, ButtonDown(0) || ButtonDown(1));
        break;
    case WindowInput::Button:
    {
        // Pointer signals number left, middle and right as 1, 2 and 3.
        unsigned const id = event.key == 1 ? 3 : event.key == 2 ? 2 : event.key + 1;
        if (event.action == WindowInput::Pressed)
            OnPointerDown()(*this, pointer_position_, id);
        else
            OnPointerUp()(*this, pointer_position_, id);
        break;
    }
    case WindowInput::Scroll:
    {
        wheel_remainder_ += event.vertical;
        int const steps = static_cast<int>(wheel_remainder_);
        wheel_remainder_ -= steps;
        if (steps)
            OnPointerWheel()(*this, pointer_position_, 0, steps);
        break;
    }
    default:
        break;
    }
}

void Window::DispatchText(char16_t unit)
{
    if (unit >= 0xd800 && unit <= 0xdbff)
    {
        pending_surrogate_ = unit;
        return;
    }

    unsigned value = unit;
    if (unit >= 0xdc00 && unit <= 0xdfff)
    {
        if (!pending_surrogate_)
            return;
        value = 0x10000u + ((pending_surrogate_ - 0xd800u) << 10u) + (unit - 0xdc00u);
    }
    pending_surrogate_ = 0;
    // RmlUi handles control characters through the key event path.
    if (value >= 0x20 && value != 0x7f)
        DispatchInput({WindowInput::Character, 0, 0, 0, value});
}
}
