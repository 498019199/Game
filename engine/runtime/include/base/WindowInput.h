#pragma once

namespace RenderWorker
{
struct WindowInput
{
    // Keys use KeyboardSemantic. Buttons use the MouseButtons order.
    enum Action { Released, Pressed, Repeated };
    enum Modifier { Shift = 1, Control = 2, Alt = 4, Meta = 8, CapsLock = 16, NumLock = 32 };
    enum Type
    {
        Key,
        Character,
        Motion,
        Button,
        Scroll
    } type;
    int key{}, action{}, modifiers{};
    unsigned codepoint{};
    double x{}, y{}, horizontal{}, vertical{};
};
}
