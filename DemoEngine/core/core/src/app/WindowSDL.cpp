#include <base/Window.h>
#include <base/ZEngine.h>
#include <SDL3/SDL.h>

namespace RenderWorker
{

Window::Window(const std::string& name, const RenderSettings& settings, void* native_wnd)
	: left_(0), top_(0), width_(0), height_(0), hide_(settings.hide_win), name_(name),
	  dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity),
	  active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
		{
			LogError() << "[SDL3] SDL_InitSubSystem(VIDEO) failed: " << SDL_GetError() << std::endl;
			TMSG("SDL_InitSubSystem(VIDEO)");
		}
	}

	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	if (hide_)
	{
		flags |= SDL_WINDOW_HIDDEN;
	}
	if (settings.full_screen)
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}

	if (native_wnd)
	{
		SDL_PropertiesID props = SDL_CreateProperties();
#if defined(ZENGINE_PLATFORM_LINUX)
		SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X11_WINDOW_NUMBER,
			static_cast<Sint64>(reinterpret_cast<uintptr_t>(native_wnd)));
#elif defined(ZENGINE_PLATFORM_DARWIN) || defined(ZENGINE_PLATFORM_IOS)
		SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, native_wnd);
#elif defined(ZENGINE_PLATFORM_ANDROID)
		SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_ANDROID_WINDOW_POINTER, native_wnd);
#endif
		sdl_wnd_ = SDL_CreateWindowWithProperties(props);
		SDL_DestroyProperties(props);
		if (!sdl_wnd_)
		{
			LogError() << "[SDL3] CreateWindow from native handle failed: " << SDL_GetError()
					   << ", creating a new window" << std::endl;
		}
	}

	if (!sdl_wnd_)
	{
		sdl_wnd_ = SDL_CreateWindow(name.c_str(), static_cast<int>(settings.width),
			static_cast<int>(settings.height), flags);
	}

	if (!sdl_wnd_)
	{
		LogError() << "[SDL3] SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
		TMSG("SDL_CreateWindow");
	}

	SDL_SetWindowPosition(sdl_wnd_, settings.left, settings.top);

	int w = 0;
	int h = 0;
	SDL_GetWindowSize(sdl_wnd_, &w, &h);
	left_ = settings.left;
	top_ = settings.top;
	width_ = static_cast<uint32_t>(w);
	height_ = static_cast<uint32_t>(h);

	float const scale = SDL_GetWindowDisplayScale(sdl_wnd_);
	this->UpdateDpiScale(scale > 0.0f ? scale : 1.0f);

	active_ = true;
	ready_ = true;
}

Window::~Window()
{
	if (sdl_wnd_)
	{
		SDL_DestroyWindow(sdl_wnd_);
		sdl_wnd_ = nullptr;
	}
}

void Window::PumpEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (!Context::Instance().AppValid())
		{
			continue;
		}
		auto& app = Context::Instance().AppInstance();
		Window* wnd = app.MainWnd().get();
		if (!wnd)
		{
			continue;
		}

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			wnd->Closed(true);
			wnd->OnClose()(*wnd);
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			if (SDL_GetWindowFromID(event.window.windowID) == wnd->GetSDLWindow())
			{
				wnd->Closed(true);
				wnd->OnClose()(*wnd);
			}
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			if (SDL_GetWindowFromID(event.window.windowID) == wnd->GetSDLWindow())
			{
				wnd->Active(true);
			}
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			if (SDL_GetWindowFromID(event.window.windowID) == wnd->GetSDLWindow())
			{
				wnd->Active(false);
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			wnd->OnPointerDown()(*wnd, int2(static_cast<int>(event.button.x), static_cast<int>(event.button.y)),
				event.button.button);
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			wnd->OnPointerUp()(*wnd, int2(static_cast<int>(event.button.x), static_cast<int>(event.button.y)),
				event.button.button);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			wnd->OnPointerUpdate()(*wnd, int2(static_cast<int>(event.motion.x), static_cast<int>(event.motion.y)), 0,
				(event.motion.state != 0));
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			wnd->OnPointerWheel()(*wnd, int2(0, 0), 0, static_cast<int32_t>(event.wheel.y));
			break;
		default:
			break;
		}
	}
}

} // namespace RenderWorker
