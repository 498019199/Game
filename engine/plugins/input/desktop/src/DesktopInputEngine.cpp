#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Context.h>
#include <base/Window.h>

#include "DesktopInput.hpp"

namespace RenderWorker
{
DesktopInputEngine::DesktopInputEngine()
{
    Window::InitializeDesktop();
}
DesktopInputEngine::~DesktopInputEngine()
{
    on_pointer_wheel_.Disconnect();
}

std::wstring const &DesktopInputEngine::Name() const
{
    static std::wstring const name(L"Desktop Input Engine");
    return name;
}

void DesktopInputEngine::EnumDevices()
{
    devices_.push_back(MakeSharedPtr<DesktopInputKeyboard>());
    devices_.push_back(MakeSharedPtr<DesktopInputMouse>());

    DesktopInputJoystick::Enumerate(devices_);

    if (Context::Instance().AppValid())
    {
        if (WindowPtr const &main_wnd = Context::Instance().AppInstance().MainWnd())
        {
            on_pointer_wheel_ = main_wnd->OnPointerWheel().Connect(
                [this]([[maybe_unused]] Window const &wnd, int2 const &pt, uint32_t id, int32_t wheel_delta) {
                    this->OnPointerWheel(pt, id, wheel_delta);
                });
        }
    }
}

void DesktopInputEngine::OnPointerWheel([[maybe_unused]] int2 const &pt, [[maybe_unused]] uint32_t id,
                                        int32_t wheel_delta)
{
    for (auto const &device : devices_)
    {
        if (InputEngine::IDT_Mouse == device->Type())
        {
            checked_pointer_cast<DesktopInputMouse>(device)->OnPointerWheel(wheel_delta);
        }
    }
}

void DesktopInputEngine::DoSuspend()
{
}

void DesktopInputEngine::DoResume()
{
}
}
