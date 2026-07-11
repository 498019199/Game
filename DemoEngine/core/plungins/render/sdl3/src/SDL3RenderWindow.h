#pragma once

#include "SDL3FrameBuffer.h"
#include "SDL3Util.h"

namespace RenderWorker
{

class SDL3RenderWindow : public SDL3FrameBuffer
{
public:
	SDL3RenderWindow(std::string const& name, RenderSettings const& settings);
	~SDL3RenderWindow() override;

	void Destroy();
	void SwapBuffers() override;

	bool FullScreen() const { return is_full_screen_; }
	SDL_Window* SDLWindow() const noexcept { return window_; }

private:
	std::string name_;
	SDL_Window* window_{nullptr};
	bool owns_window_{false};
	bool is_full_screen_{false};
	uint32_t sync_interval_{0};
	ElementFormat depth_stencil_fmt_{EF_Unknown};
};

using SDL3RenderWindowPtr = std::shared_ptr<SDL3RenderWindow>;

} // namespace RenderWorker
