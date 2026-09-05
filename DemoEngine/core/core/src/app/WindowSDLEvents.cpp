#include <base/Window.h>
#include <base/App3D.h>
#include <base/UIManager.h>
#include <SDL3/SDL.h>

namespace RenderWorker
{
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

		wnd->OnSDLEvent()(event);
		if (auto* event_window = SDL_GetWindowFromEvent(&event); event_window && event_window != wnd->GetSDLWindow())
			continue;
		Context::Instance().UIManagerInstance().QueueInput(event);

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
}
