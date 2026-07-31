#include "SDL3MvpTriangle.h"
#include <common/Log.h>
#include <cstring>

namespace RenderWorker
{
namespace
{
// Hardcoded NDC triangle via vertex_id; solid red fragment. MSL for Metal (macOS).
char const kMvpVsMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct VSOut
{
	float4 position [[position]];
};

vertex VSOut mvp_vs(uint vid [[vertex_id]])
{
	float2 positions[3] = {
		float2( 0.0,  0.75),
		float2(-0.75, -0.75),
		float2( 0.75, -0.75)
	};
	VSOut out;
	out.position = float4(positions[vid], 0.0, 1.0);
	return out;
}
)MSL";

char const kMvpPsMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

fragment float4 mvp_ps()
{
	return float4(1.0, 0.0, 0.0, 1.0);
}
)MSL";

// SPIR-V path placeholder: Metal is the Mac gate; Windows DXIL lands later.
} // namespace

SDL3MvpTriangle::~SDL3MvpTriangle() = default;

void SDL3MvpTriangle::Release(SDL_GPUDevice* device)
{
	if (!device)
	{
		vs_ = nullptr;
		ps_ = nullptr;
		pipeline_ = nullptr;
		ready_ = false;
		return;
	}
	if (pipeline_)
	{
		SDL_ReleaseGPUGraphicsPipeline(device, pipeline_);
		pipeline_ = nullptr;
	}
	if (vs_)
	{
		SDL_ReleaseGPUShader(device, vs_);
		vs_ = nullptr;
	}
	if (ps_)
	{
		SDL_ReleaseGPUShader(device, ps_);
		ps_ = nullptr;
	}
	pipeline_fmt_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	ready_ = false;
}

bool SDL3MvpTriangle::CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt)
{
	SDL_GPUShaderFormat const supported = SDL_GetGPUShaderFormats(device);

	SDL_GPUShaderCreateInfo vs_info{};
	SDL_GPUShaderCreateInfo ps_info{};
	vs_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
	ps_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	vs_info.entrypoint = "mvp_vs";
	ps_info.entrypoint = "mvp_ps";

	if (supported & SDL_GPU_SHADERFORMAT_MSL)
	{
		vs_info.format = SDL_GPU_SHADERFORMAT_MSL;
		vs_info.code = reinterpret_cast<Uint8 const*>(kMvpVsMsl);
		vs_info.code_size = std::strlen(kMvpVsMsl);
		ps_info.format = SDL_GPU_SHADERFORMAT_MSL;
		ps_info.code = reinterpret_cast<Uint8 const*>(kMvpPsMsl);
		ps_info.code_size = std::strlen(kMvpPsMsl);
	}
	else
	{
		LogError() << "[SDL3] MVP triangle: no MSL shader format on this GPU device" << std::endl;
		return false;
	}

	vs_ = SDL_CreateGPUShader(device, &vs_info);
	if (!vs_)
	{
		LogError() << "[SDL3] MVP VS create failed: " << SDL_GetError() << std::endl;
		return false;
	}
	ps_ = SDL_CreateGPUShader(device, &ps_info);
	if (!ps_)
	{
		LogError() << "[SDL3] MVP PS create failed: " << SDL_GetError() << std::endl;
		SDL_ReleaseGPUShader(device, vs_);
		vs_ = nullptr;
		return false;
	}

	SDL_GPUColorTargetDescription color_desc{};
	color_desc.format = swapchain_fmt;
	color_desc.blend_state.enable_blend = false;
	color_desc.blend_state.color_write_mask = 0xF;

	SDL_GPUGraphicsPipelineCreateInfo pipe{};
	pipe.vertex_shader = vs_;
	pipe.fragment_shader = ps_;
	pipe.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipe.target_info.num_color_targets = 1;
	pipe.target_info.color_target_descriptions = &color_desc;
	pipe.target_info.has_depth_stencil_target = false;
	pipe.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	pipe.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
	pipe.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	pipe.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;

	pipeline_ = SDL_CreateGPUGraphicsPipeline(device, &pipe);
	if (!pipeline_)
	{
		LogError() << "[SDL3] MVP pipeline create failed: " << SDL_GetError() << std::endl;
		SDL_ReleaseGPUShader(device, vs_);
		SDL_ReleaseGPUShader(device, ps_);
		vs_ = nullptr;
		ps_ = nullptr;
		return false;
	}

	pipeline_fmt_ = swapchain_fmt;
	return true;
}

bool SDL3MvpTriangle::EnsureResources(SDL_GPUDevice* device, SDL_Window* window)
{
	if (!device || !window)
	{
		return false;
	}

	SDL_GPUTextureFormat const fmt = SDL_GetGPUSwapchainTextureFormat(device, window);
	if (ready_ && pipeline_ && pipeline_fmt_ == fmt)
	{
		return true;
	}

	Release(device);
	ready_ = CreatePipeline(device, fmt);
	if (ready_)
	{
		LogInfo() << "[SDL3] MVP blue+red-triangle path ready" << std::endl;
	}
	return ready_;
}

void SDL3MvpTriangle::Draw(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex,
	SDL_GPUTextureFormat /*swapchain_fmt*/)
{
	if (!ready_ || !cmd || !swapchain_tex || !pipeline_)
	{
		return;
	}

	SDL_GPUColorTargetInfo color{};
	color.texture = swapchain_tex;
	color.clear_color.r = 0.0f;
	color.clear_color.g = 0.0f;
	color.clear_color.b = 1.0f;
	color.clear_color.a = 1.0f;
	color.load_op = SDL_GPU_LOADOP_CLEAR;
	color.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color, 1, nullptr);
	if (!pass)
	{
		LogError() << "[SDL3] MVP BeginGPURenderPass failed: " << SDL_GetError() << std::endl;
		return;
	}

	SDL_BindGPUGraphicsPipeline(pass, pipeline_);
	SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
	SDL_EndGPURenderPass(pass);
}

} // namespace RenderWorker
