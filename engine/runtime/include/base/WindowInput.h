#pragma once

namespace RenderWorker
{
struct WindowInput
{
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
