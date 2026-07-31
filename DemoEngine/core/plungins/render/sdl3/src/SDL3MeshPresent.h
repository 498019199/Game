#pragma once

#include "SDL3Util.h"
#include <math/matrix.h>
#include <math/vectorxd.h>

namespace RenderWorker
{

// Metal (MSL) present-path mesh draw for macOS where HLSL mesh shaders
// cannot compile. Decompresses SNORM positions via pos_center/pos_extent;
// optional albedo texture sampling when material + UV stream are present.
class SDL3MeshPresent final
{
public:
	SDL3MeshPresent() = default;
	~SDL3MeshPresent();

	SDL3MeshPresent(SDL3MeshPresent const&) = delete;
	SDL3MeshPresent& operator=(SDL3MeshPresent const&) = delete;

	bool EnsureResources(SDL_GPUDevice* device, SDL_Window* window, SDL_GPUTextureFormat depth_fmt);
	void DrawSceneMeshes(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex, SDL_GPUTexture* depth_tex,
		float4x4 const& view_proj);
	void Release(SDL_GPUDevice* device);

private:
	bool CreatePipelines(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt, SDL_GPUTextureFormat depth_fmt);

	SDL_GPUShader* vs_{nullptr};
	SDL_GPUShader* ps_{nullptr};
	SDL_GPUGraphicsPipeline* pipeline_{nullptr};
	SDL_GPUShader* vs_tex_{nullptr};
	SDL_GPUShader* ps_tex_{nullptr};
	SDL_GPUGraphicsPipeline* pipeline_tex_{nullptr};
	SDL_GPUSampler* sampler_{nullptr};
	SDL_GPUTextureFormat pipeline_color_fmt_{SDL_GPU_TEXTUREFORMAT_INVALID};
	SDL_GPUTextureFormat pipeline_depth_fmt_{SDL_GPU_TEXTUREFORMAT_INVALID};
	bool ready_{false};
};

} // namespace RenderWorker
