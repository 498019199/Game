#pragma once

#include <render/RenderEngine.h>
#include "SDL3Util.h"

namespace RenderWorker
{

class SDL3RenderEngine : public RenderEngine
{
public:
	SDL3RenderEngine();
	~SDL3RenderEngine() override;

	bool RequiresFlipping() const override
	{
		return true;
	}

	void BeginFrame() override;
	void EndFrame() override;

	SDL_GPUDevice* Device() const noexcept { return device_; }
	SDL_Window* Window() const noexcept { return window_; }
	SDL_GPUCommandBuffer* CurrentCommandBuffer() const noexcept { return cmd_; }

	void Device(SDL_GPUDevice* device, SDL_Window* window);
	void DetachWindow() noexcept { window_ = nullptr; }
	void FillRenderDeviceCaps();

	void CurrentCommandBuffer(SDL_GPUCommandBuffer* cmd) noexcept { cmd_ = cmd; }
	void CurrentSwapchainTexture(SDL_GPUTexture* tex) noexcept { swapchain_tex_ = tex; }
	SDL_GPUTexture* CurrentSwapchainTexture() const noexcept { return swapchain_tex_; }

private:
	void DoCreateRenderWindow(std::string const& name, RenderSettings const& settings) override;
	void DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl) override;
	void DoBindFrameBuffer(FrameBufferPtr const& fb) override;
	void DoBindSOBuffers(const RenderLayoutPtr& rl) override;
	void DoDestroy() override;

private:
	SDL_GPUDevice* device_{nullptr};
	SDL_Window* window_{nullptr};
	SDL_GPUCommandBuffer* cmd_{nullptr};
	SDL_GPUTexture* swapchain_tex_{nullptr};
	bool owns_sdl_init_{false};
};

} // namespace RenderWorker
