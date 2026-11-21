#include "DesktopInput.hpp"
#import <GameController/GameController.h>
#import <CoreHaptics/CoreHaptics.h>
#include <algorithm>

namespace RenderWorker
{
struct DesktopInputJoystick::Controller
{
    GCController *pad;
    CHHapticEngine *engines[2]{};
    id<CHHapticPatternPlayer> players[2]{};
};
void DesktopInputJoystick::Enumerate(std::vector<InputDevicePtr> &devices)
{
    for (GCController *pad in GCController.controllers)
    {
        if (pad.extendedGamepad)
        {
            auto state = std::make_unique<Controller>();
            state->pad = pad;
            devices.push_back(std::make_shared<DesktopInputJoystick>(std::move(state)));
        }
    }
}
DesktopInputJoystick::DesktopInputJoystick(std::unique_ptr<Controller> pad) : controller_(std::move(pad))
{
    NSString *label = controller_->pad.vendorName ?: @"Gamepad";
    name_.resize(label.length);
    for (NSUInteger i = 0; i < label.length; ++i)
        name_[i] = [label characterAtIndex:i];
    num_buttons_ = 12;
    auto haptics = controller_->pad.haptics;
    NSArray *sides = @[ GCHapticsLocalityLeftHandle, GCHapticsLocalityRightHandle ];
    for (unsigned i = 0; i < 2; ++i)
    {
        if ([haptics.supportedLocalities containsObject:sides[i]])
            controller_->engines[i] = [haptics createEngineWithLocality:sides[i]];
    }
    if (!controller_->engines[0] && !controller_->engines[1])
        controller_->engines[0] = [haptics createEngineWithLocality:GCHapticsLocalityDefault];
    num_vibration_motors_ = controller_->engines[1] ? 2 : controller_->engines[0] ? 1 : 0;
}
DesktopInputJoystick::~DesktopInputJoystick()
{
    for (auto engine : controller_->engines)
        [engine stopWithCompletionHandler:nil];
}
std::wstring const &DesktopInputJoystick::Name() const
{
    return name_;
}
void DesktopInputJoystick::VibrationMotorSpeed(uint32_t n, float speed)
{
    if (n >= num_vibration_motors_)
        return;
    auto engine = controller_->engines[n];
    [controller_->players[n] stopAtTime:0 error:nil];
    controller_->players[n] = nil;
    if (speed <= 0 || !engine)
        return;
    CHHapticEventParameter *intensity =
        [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity
                                                      value:std::clamp(speed, 0.f, 1.f)];
    CHHapticEvent *event = [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous
                                                         parameters:@[ intensity ]
                                                       relativeTime:0
                                                           duration:GCHapticDurationInfinite];
    NSError *error = nil;
    auto pattern = [[CHHapticPattern alloc] initWithEvents:@[ event ] parameters:@[] error:&error];
    if (pattern && [engine startAndReturnError:&error])
    {
        controller_->players[n] = [engine createPlayerWithPattern:pattern error:&error];
        [controller_->players[n] startAtTime:0 error:&error];
    }
    if (error)
        LogWarn() << "Gamepad haptics: " << error.localizedDescription.UTF8String << std::endl;
}
void DesktopInputJoystick::UpdateInputs()
{
    index_ = !index_;
    buttons_[index_].fill(false);
    thumbs_.fill(float3(0, 0, 0));
    triggers_.fill(0);
    if (![GCController.controllers containsObject:controller_->pad])
        return;
    auto g = controller_->pad.extendedGamepad;
    GCControllerButtonInput *buttons[] = {g.dpad.up,    g.dpad.down,     g.dpad.left,    g.dpad.right,
                                          g.buttonMenu, g.buttonOptions, g.leftShoulder, g.rightShoulder,
                                          g.buttonA,    g.buttonB,       g.buttonX,      g.buttonY};
    for (size_t i = 0; i < std::size(buttons); ++i)
        buttons_[index_][i] = buttons[i].isPressed;
    auto axis = [](float v, float dz) { return std::abs(v) > dz ? v : 0.f; };
    thumbs_[0] = float3(axis(g.leftThumbstick.xAxis.value, .2395f), axis(g.leftThumbstick.yAxis.value, .2395f),
                        g.leftThumbstickButton.isPressed);
    thumbs_[1] = float3(axis(g.rightThumbstick.xAxis.value, .2652f), axis(g.rightThumbstick.yAxis.value, .2652f),
                        g.rightThumbstickButton.isPressed);
    triggers_[0] = g.leftTrigger.value > 30.f / 255 ? g.leftTrigger.value : 0;
    triggers_[1] = g.rightTrigger.value > 30.f / 255 ? g.rightTrigger.value : 0;
}
}
