#include "DesktopInput.hpp"
#include <Xinput.h>
#include <algorithm>

namespace RenderWorker
{
struct DesktopInputJoystick::Controller
{
    DWORD index;
    XINPUT_VIBRATION motors{};
};
void DesktopInputJoystick::Enumerate(std::vector<InputDevicePtr> &devices)
{
    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        XINPUT_STATE state{};
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
            devices.push_back(std::make_shared<DesktopInputJoystick>(std::make_unique<Controller>(Controller{i})));
    }
}
DesktopInputJoystick::DesktopInputJoystick(std::unique_ptr<Controller> pad)
    : controller_(std::move(pad)), name_(L"XInput Gamepad")
{
    num_buttons_ = 12;
    num_vibration_motors_ = 2;
}
DesktopInputJoystick::~DesktopInputJoystick()
{
    controller_->motors = {};
    XInputSetState(controller_->index, &controller_->motors);
}
std::wstring const &DesktopInputJoystick::Name() const
{
    return name_;
}
void DesktopInputJoystick::VibrationMotorSpeed(uint32_t n, float speed)
{
    if (n >= 2)
        return;
    WORD value = static_cast<WORD>(std::clamp(speed, 0.f, 1.f) * 65535.f);
    if (n == 0)
        controller_->motors.wLeftMotorSpeed = value;
    else
        controller_->motors.wRightMotorSpeed = value;
    XInputSetState(controller_->index, &controller_->motors);
}
void DesktopInputJoystick::UpdateInputs()
{
    index_ = !index_;
    buttons_[index_].fill(false);
    thumbs_.fill(float3(0, 0, 0));
    triggers_.fill(0);
    XINPUT_STATE state{};
    if (XInputGetState(controller_->index, &state) != ERROR_SUCCESS)
        return;
    auto const &g = state.Gamepad;
    constexpr WORD masks[] = {XINPUT_GAMEPAD_DPAD_UP,
                              XINPUT_GAMEPAD_DPAD_DOWN,
                              XINPUT_GAMEPAD_DPAD_LEFT,
                              XINPUT_GAMEPAD_DPAD_RIGHT,
                              XINPUT_GAMEPAD_START,
                              XINPUT_GAMEPAD_BACK,
                              XINPUT_GAMEPAD_LEFT_SHOULDER,
                              XINPUT_GAMEPAD_RIGHT_SHOULDER,
                              XINPUT_GAMEPAD_A,
                              XINPUT_GAMEPAD_B,
                              XINPUT_GAMEPAD_X,
                              XINPUT_GAMEPAD_Y};
    for (size_t i = 0; i < std::size(masks); ++i)
        buttons_[index_][i] = (g.wButtons & masks[i]) != 0;
    auto axis = [](SHORT value, int dz) { return std::abs(int(value)) > dz ? value / 32768.f : 0.f; };
    thumbs_[0] =
        float3(axis(g.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
               axis(g.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE), bool(g.wButtons & XINPUT_GAMEPAD_LEFT_THUMB));
    thumbs_[1] =
        float3(axis(g.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
               axis(g.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE), bool(g.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB));
    triggers_[0] = g.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? g.bLeftTrigger / 255.f : 0.f;
    triggers_[1] = g.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? g.bRightTrigger / 255.f : 0.f;
}
}
