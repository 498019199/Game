#pragma once

#include "SDL3Util.h"

namespace RenderWorker
{

// Self-contained blue-clear + red-triangle present path for the SDL3 MVP gate.
// Does not use RenderEffect / PredefinedCBuffers / scene meshes.
class SDL3MvpTriangle final
{
public:
	SDL3MvpTriangle() = default;
	~SDL3MvpTriangle();

	SDL3MvpTriangle(SDL3MvpTriangle const&) = delete;
	SDL3MvpTriangle& operator=(SDL3MvpTriangle const&) = delete;

	bool EnsureResources(SDL_GPUDevice* device, SDL_Window* window);
	void Draw(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex, SDL_GPUTextureFormat swapchain_fmt);
	void Release(SDL_GPUDevice* device);

private:
	bool CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt);

	SDL_GPUShader* vs_{nullptr};
	SDL_GPUShader* ps_{nullptr};
	SDL_GPUGraphicsPipeline* pipeline_{nullptr};
	SDL_GPUTextureFormat pipeline_fmt_{SDL_GPU_TEXTUREFORMAT_INVALID};
	bool ready_{false};
};

} // namespace RenderWorker
