#include <base/App3D.h>
#include <base/UIManager.h>
#include <base/Window.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

namespace RenderWorker
{
void Window::InitializeDesktop()
{
    static struct Runtime
    {
        Runtime()
        {
            glfwSetErrorCallback([](int, char const *message) { LogError() << "GLFW: " << message << std::endl; });
            if (!glfwInit())
                TMSG("Unable to initialize desktop window system");
        }
        ~Runtime()
        {
            glfwTerminate();
        }
    } runtime;
}

namespace
{
Window &Owner(GLFWwindow *w)
{
    return *static_cast<Window *>(glfwGetWindowUserPointer(w));
}
void Queue(WindowInput event)
{
    Context::Instance().UIManagerInstance().QueueInput(event);
}
}

Window::Window(std::string const &name, RenderSettings const &settings, void *native_wnd)
    : left_(settings.left), top_(settings.top), width_(settings.width), height_(settings.height), dpi_scale_(1),
      effective_dpi_scale_(1), win_rotation_(WR_Identity), active_(true), ready_(false), closed_(false),
      keep_screen_on_(settings.keep_screen_on)
{
    InitializeDesktop();
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, !settings.hide_win);
#ifdef _WIN32
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif
    window_ = glfwCreateWindow(settings.width, settings.height, name.c_str(),
                               settings.full_screen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
    if (!window_)
        TMSG("Unable to create desktop window");
    if (!settings.full_screen)
        glfwSetWindowPos(window_, settings.left, settings.top);
    glfwSetWindowUserPointer(window_, this);
    float sx, sy;
    glfwGetWindowContentScale(window_, &sx, &sy);
    UpdateDpiScale(sx);
    glfwSetWindowContentScaleCallback(window_, [](GLFWwindow *w, float x, float) { Owner(w).UpdateDpiScale(x); });
    glfwSetWindowSizeCallback(window_, [](GLFWwindow *w, int x, int y) {
        Owner(w).width_ = x;
        Owner(w).height_ = y;
    });
    glfwSetWindowPosCallback(window_, [](GLFWwindow *w, int x, int y) {
        Owner(w).left_ = x;
        Owner(w).top_ = y;
    });
    glfwSetWindowCloseCallback(window_, [](GLFWwindow *w) {
        auto &window = Owner(w);
        if (!Context::Instance().AppInstance().OnCloseRequested())
        {
            glfwSetWindowShouldClose(w, false);
            return;
        }
        window.Closed(true);
        window.OnClose()(window);
    });
    glfwSetWindowFocusCallback(window_, [](GLFWwindow *w, int active) { Owner(w).Active(active != 0); });
    glfwSetKeyCallback(window_, [](GLFWwindow *, int key, int, int action, int mods) {
        Queue({WindowInput::Key, key, action, mods});
    });
    glfwSetCharCallback(window_, [](GLFWwindow *, unsigned c) { Queue({WindowInput::Character, 0, 0, 0, c}); });
    glfwSetCursorPosCallback(window_, [](GLFWwindow *w, double x, double y) {
        Queue({WindowInput::Motion, 0, 0, 0, 0, x, y});
        Owner(w).OnPointerUpdate()(Owner(w), int2(int(x), int(y)), 0,
                                   glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) ||
                                       glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT));
    });
    glfwSetMouseButtonCallback(window_, [](GLFWwindow *w, int button, int action, int mods) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        Queue({WindowInput::Button, button, action, mods, 0, x, y});
        // Engine pointer IDs use left, middle, right numbering.
        unsigned id = button == 1 ? 3u : button == 2 ? 2u : unsigned(button + 1);
        auto &window = Owner(w);
        if (action == GLFW_PRESS)
            window.OnPointerDown()(window, int2(int(x), int(y)), id);
        else
            window.OnPointerUp()(window, int2(int(x), int(y)), id);
    });
    glfwSetScrollCallback(window_, [](GLFWwindow *w, double dx, double dy) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        Queue({WindowInput::Scroll, 0, 0, 0, 0, x, y, dx, dy});
        Owner(w).OnPointerWheel()(Owner(w), int2(int(x), int(y)), 0, int(dy));
    });
    if (native_wnd)
    {
#ifdef _WIN32
        SetParent(GetHWND(), static_cast<HWND>(native_wnd));
#else
        extern void AttachDesktopChild(void *, void *);
        AttachDesktopChild(native_wnd, NativeHandle());
#endif
    }
#ifdef _WIN32
    if (keep_screen_on_) SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
#endif
    ready_ = true;
}

Window::~Window()
{
#ifdef _WIN32
    if (keep_screen_on_) SetThreadExecutionState(ES_CONTINUOUS);
#endif
    glfwDestroyWindow(window_);
}
void Window::PumpEvents()
{
    glfwPollEvents();
}
#ifdef _WIN32
HWND Window::GetHWND() const
{
    return glfwGetWin32Window(window_);
}
void *Window::NativeHandle() const
{
    return GetHWND();
}
#else
void *Window::NativeHandle() const
{
    return glfwGetCocoaWindow(window_);
}
#endif
}
