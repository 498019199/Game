#include <base/UIManager.h>
#include <base/App3D.h>
#include <base/Window.h>
#include <RmlUi/Core.h>
#include <RmlUi_Platform_GLFW.h>
#include <array>
#include <vector>
#include <set>

namespace RenderWorker
{
struct UIInputState
{

    std::vector<WindowInput> events;
    std::function<bool(int)> key_handler;
    bool mouse = false, keyboard = false;
    bool block_mouse = false, block_keyboard = false;
    bool consumed_mouse = false, consumed_keyboard = false;
    std::array<bool, GLFW_KEY_LAST + 1> held_keys{};
    unsigned held_buttons = 0;
    std::set<Rml::Input::KeyIdentifier> rml_keys;
    int modifiers = 0;
    float x = 0, y = 0, width = 1, height = 1;
    std::array<bool, 5> buttons{};
};

void UIManager::QueueInput(WindowInput const &event)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    input_state_->events.push_back(event);
}

void UIManager::SetInputViewport(bool mouse, bool keyboard, float x, float y, float width, float height)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    auto &s = *input_state_;
    s.mouse = mouse;
    s.keyboard = keyboard;
    s.x = x;
    s.y = y;
    s.width = width;
    s.height = height;
}

void UIManager::SetKeyHandler(std::function<bool(int)> handler)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    input_state_->key_handler = std::move(handler);
}

bool UIManager::GameKeyboardBlocked() const noexcept
{
    return input_state_ && (input_state_->block_keyboard || input_state_->consumed_keyboard);
}
bool UIManager::GamePointerBlocked() const noexcept
{
    return input_state_ && (input_state_->block_mouse || input_state_->consumed_mouse);
}
void UIManager::AcknowledgeGameInput() noexcept
{
    if (input_state_)
    {
        input_state_->consumed_mouse = false;
        input_state_->consumed_keyboard = false;
    }
}

void UIManager::RouteInput(bool editor_mode, bool imgui_exclusive)
{
    if (!input_state_)
        input_state_ = std::make_shared<UIInputState>();
    auto &s = *input_state_;
    auto *window = Context::Instance().AppValid() ? Context::Instance().AppInstance().MainWnd().get() : nullptr;
    bool const active = window && window->Active();
    bool const mouse = active && !imgui_exclusive && (!editor_mode || s.mouse);
    bool const keyboard = active && !imgui_exclusive && (!editor_mode || s.keyboard);
    s.block_mouse = !mouse;
    s.block_keyboard = !keyboard;
    if (!editor_mode && window)
    {
        int w = 1, h = 1;
        glfwGetWindowSize(window->DesktopWindow(), &w, &h);
        s.x = s.y = 0;
        s.width = float(w);
        s.height = float(h);
    }
    if (rml_context_)
    {
        int mods = RmlGLFW::ConvertKeyModifiers(s.modifiers);
        if (!keyboard)
        {
            for (auto key : s.rml_keys)
                rml_context_->ProcessKeyUp(key, mods);
            s.rml_keys.clear();
        }
        if (!mouse)
        {
            // Cancel a Rml drag before another UI takes ownership.
            rml_context_->ProcessMouseLeave();
            for (size_t i = 0; i < s.buttons.size(); ++i)
                if (s.buttons[i])
                {
                    rml_context_->ProcessMouseButtonUp(int(i), mods);
                    s.buttons[i] = false;
                }
        }
        bool suppress_console_text = false;
        for (auto const &e : s.events)
        {
            if (e.type == WindowInput::Key || e.type == WindowInput::Button)
            {
                s.modifiers = e.modifiers;
                mods = RmlGLFW::ConvertKeyModifiers(s.modifiers);
            }
            if (e.type == WindowInput::Key)
            {
                if (!keyboard)
                    continue;
                auto key = RmlGLFW::ConvertKey(e.key);
                if (e.action == GLFW_PRESS && s.key_handler && s.key_handler(int(key)))
                {
                    s.block_keyboard = true;
                    suppress_console_text = e.key == GLFW_KEY_GRAVE_ACCENT;
                    continue;
                }
                if (e.action == GLFW_RELEASE)
                    s.rml_keys.erase(key);
                else
                    s.rml_keys.insert(key);
                if (!RmlGLFW::ProcessKeyCallback(rml_context_, e.key, e.action, e.modifiers))
                    s.block_keyboard = true;
            }
            else if (e.type == WindowInput::Character)
            {
                if (keyboard && !(suppress_console_text && (e.codepoint == '`' || e.codepoint == '~')))
                    s.block_keyboard |= !RmlGLFW::ProcessCharCallback(rml_context_, e.codepoint);
            }
            else if (mouse)
            {
                bool propagated =
                    rml_context_->ProcessMouseMove(int((e.x - s.x) * width_ / std::max(s.width, 1.f)),
                                                   int((e.y - s.y) * height_ / std::max(s.height, 1.f)), mods);
                if (e.type == WindowInput::Button && e.key >= 0 && e.key < int(s.buttons.size()))
                {
                    s.buttons[e.key] = e.action == GLFW_PRESS;
                    propagated &= s.buttons[e.key] ? rml_context_->ProcessMouseButtonDown(e.key, mods)
                                                   : rml_context_->ProcessMouseButtonUp(e.key, mods);
                }
                else if (e.type == WindowInput::Scroll)
                {
                    propagated &= rml_context_->ProcessMouseWheel(Rml::Vector2f(-e.horizontal, -e.vertical), mods);
                }
                if (!propagated)
                    s.block_mouse = true;
            }
        }
        auto *hover = rml_context_->GetHoverElement();
        mouse_on_ui_ = mouse && hover && hover != rml_context_->GetRootElement();
        auto *focus = rml_context_->GetFocusElement();
        bool const focused_ui = keyboard && focus && focus != rml_context_->GetRootElement() && focus->IsVisible();
        s.block_mouse |= mouse_on_ui_;
        s.block_keyboard |= focused_ui;
    }
    auto *desktop = window ? window->DesktopWindow() : nullptr;
    for (int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i)
    {
        if (s.held_keys[i])
            s.consumed_keyboard = true;
        s.held_keys[i] = desktop && glfwGetKey(desktop, i) && (s.held_keys[i] || s.block_keyboard);
    }
    unsigned buttons = 0;
    for (int i = 0; desktop && i <= GLFW_MOUSE_BUTTON_LAST; ++i)
        if (glfwGetMouseButton(desktop, i))
            buttons |= 1u << i;
    if (s.held_buttons)
        s.consumed_mouse = true;
    s.held_buttons = buttons & (s.block_mouse ? ~0u : s.held_buttons);
    s.consumed_keyboard |= s.block_keyboard;
    s.consumed_mouse |= s.block_mouse;
    s.events.clear();
}
}
