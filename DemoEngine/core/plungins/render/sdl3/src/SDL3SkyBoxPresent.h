#pragma once

#include "SDL3Util.h"
#include <render/Texture.h>
#include <math/matrix.h>

namespace RenderWorker
{

// Metal (MSL) present-path skybox: samples a cube map with camera inv_mvp.
// Used on macOS where HLSL→DXIL SkyBox.shader is unavailable.
class SDL3SkyBoxPresent final
{
public:
	SDL3SkyBoxPresent() = default;
	~SDL3SkyBoxPresent();

	SDL3SkyBoxPresent(SDL3SkyBoxPresent const&) = delete;
	SDL3SkyBoxPresent& operator=(SDL3SkyBoxPresent const&) = delete;

	bool EnsureResources(SDL_GPUDevice* device, SDL_Window* window);
	void Draw(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex, SDL_GPUTexture* cube_tex,
		float4x4 const& inv_mvp);
	void Release(SDL_GPUDevice* device);

private:
	bool CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt);

	SDL_GPUShader* vs_{nullptr};
	SDL_GPUShader* ps_{nullptr};
	SDL_GPUGraphicsPipeline* pipeline_{nullptr};
	SDL_GPUSampler* sampler_{nullptr};
	SDL_GPUTextureFormat pipeline_fmt_{SDL_GPU_TEXTUREFORMAT_INVALID};
	bool ready_{false};
};

} // namespace RenderWorker
