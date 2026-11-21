#include <base/ZEngine.h>

#include "DesktopInput.hpp"

#include <base/App3D.h>
#include <base/Window.h>

#include <algorithm>


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
    auto const* window = ctx.AppValid() ? ctx.AppInstance().MainWnd().get() : nullptr;
    for (std::size_t key = 1; window && key < keys_state_.size(); ++key)
        keys_state_[key] = window->KeyDown(static_cast<int>(key));

    index_ = !index_;
    keys_[index_] = keys_state_;
}
}
