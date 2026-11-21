#ifndef _DesktopINPUT_HPP
#define _DesktopINPUT_HPP

#pragma once

#include <base/Input.h>
#include <base/Window.h>

#include <array>
#include <vector>

namespace RenderWorker
{
class DesktopInputEngine final : public InputEngine
{
  public:
    DesktopInputEngine();
    ~DesktopInputEngine() override;

    std::wstring const &Name() const override;
    void EnumDevices() override;

  private:
    void DoSuspend() override;
    void DoResume() override;

    void OnPointerWheel(int2 const &pt, uint32_t id, int32_t wheel_delta);

  private:
    Signal::Connection on_pointer_wheel_;
};

class DesktopInputKeyboard final : public InputKeyboard
{
  public:
    DesktopInputKeyboard();

    std::wstring const &Name() const override;

  private:
    void UpdateInputs() override;

  private:
    std::array<bool, 256> keys_state_{};
};

class DesktopInputMouse final : public InputMouse
{
  public:
    DesktopInputMouse();

    std::wstring const &Name() const override;

    void OnPointerWheel(int32_t wheel_delta);

  private:
    void UpdateInputs() override;

  private:
    int2 last_abs_state_{0, 0};
    bool has_last_abs_{false};
    int32_t wheel_state_{0};
};

class DesktopInputJoystick final : public InputJoystick
{
  public:
    struct Controller;
    static void Enumerate(std::vector<InputDevicePtr> &devices);
    explicit DesktopInputJoystick(std::unique_ptr<Controller> controller);
    ~DesktopInputJoystick() override;

    std::wstring const &Name() const override;

    void VibrationMotorSpeed(uint32_t n, float motor_speed) override;

  private:
    void UpdateInputs() override;

  private:
    std::unique_ptr<Controller> controller_;
    std::wstring name_;
};
}

#endif // _DesktopINPUT_HPP
