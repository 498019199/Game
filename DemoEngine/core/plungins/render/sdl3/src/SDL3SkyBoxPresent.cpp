#include "SDL3SkyBoxPresent.h"
#include <common/Log.h>
#include <math/matrix.h>
#include <cstring>

namespace RenderWorker
{
namespace
{

char const kSkyVsMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct Uniforms
{
	float4 r0;
	float4 r1;
	float4 r2;
	float4 r3;
};

struct VSOut
{
	float4 position [[position]];
	float3 dir;
};

vertex VSOut sky_vs(uint vid [[vertex_id]], constant Uniforms& u [[buffer(0)]])
{
	float2 corners[4] = {
		float2( 1.0,  1.0),
		float2( 1.0, -1.0),
		float2(-1.0,  1.0),
		float2(-1.0, -1.0)
	};
	float4 pos = float4(corners[vid], 1.0, 1.0);
	VSOut out;
	out.position = pos;
	// HLSL mul(pos, inv_mvp) with row-major rows in Uniforms.
	out.dir = pos.x * u.r0.xyz + pos.y * u.r1.xyz + pos.z * u.r2.xyz + pos.w * u.r3.xyz;
	return out;
}
)MSL";

char const kSkyPsMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct VSOut
{
	float4 position [[position]];
	float3 dir;
};

fragment float4 sky_ps(VSOut in [[stage_in]],
	texturecube<float> sky [[texture(0)]],
	sampler smp [[sampler(0)]])
{
	return sky.sample(smp, normalize(in.dir));
}
)MSL";

} // namespace

SDL3SkyBoxPresent::~SDL3SkyBoxPresent() = default;

void SDL3SkyBoxPresent::Release(SDL_GPUDevice* device)
{
	if (!device)
	{
		vs_ = nullptr;
		ps_ = nullptr;
		pipeline_ = nullptr;
		sampler_ = nullptr;
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
	if (sampler_)
	{
		SDL_ReleaseGPUSampler(device, sampler_);
		sampler_ = nullptr;
	}
	pipeline_fmt_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	ready_ = false;
}

bool SDL3SkyBoxPresent::CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt)
{
	SDL_GPUShaderFormat const supported = SDL_GetGPUShaderFormats(device);
	if (!(supported & SDL_GPU_SHADERFORMAT_MSL))
	{
		LogError() << "[SDL3] SkyBox present: no MSL shader format" << std::endl;
		return false;
	}

	SDL_GPUShaderCreateInfo vs_info{};
	vs_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
	vs_info.format = SDL_GPU_SHADERFORMAT_MSL;
	vs_info.code = reinterpret_cast<Uint8 const*>(kSkyVsMsl);
	vs_info.code_size = std::strlen(kSkyVsMsl);
	vs_info.entrypoint = "sky_vs";
	vs_info.num_uniform_buffers = 1;

	SDL_GPUShaderCreateInfo ps_info{};
	ps_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	ps_info.format = SDL_GPU_SHADERFORMAT_MSL;
	ps_info.code = reinterpret_cast<Uint8 const*>(kSkyPsMsl);
	ps_info.code_size = std::strlen(kSkyPsMsl);
	ps_info.entrypoint = "sky_ps";
	ps_info.num_samplers = 1;

	vs_ = SDL_CreateGPUShader(device, &vs_info);
	if (!vs_)
	{
		LogError() << "[SDL3] SkyBox VS create failed: " << SDL_GetError() << std::endl;
		return false;
	}
	ps_ = SDL_CreateGPUShader(device, &ps_info);
	if (!ps_)
	{
		LogError() << "[SDL3] SkyBox PS create failed: " << SDL_GetError() << std::endl;
		SDL_ReleaseGPUShader(device, vs_);
		vs_ = nullptr;
		return false;
	}

	SDL_GPUSamplerCreateInfo si{};
	si.min_filter = SDL_GPU_FILTER_LINEAR;
	si.mag_filter = SDL_GPU_FILTER_LINEAR;
	si.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	si.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	si.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	si.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler_ = SDL_CreateGPUSampler(device, &si);
	if (!sampler_)
	{
		LogError() << "[SDL3] SkyBox sampler create failed: " << SDL_GetError() << std::endl;
		SDL_ReleaseGPUShader(device, vs_);
		SDL_ReleaseGPUShader(device, ps_);
		vs_ = nullptr;
		ps_ = nullptr;
		return false;
	}

	SDL_GPUColorTargetDescription color_desc{};
	color_desc.format = swapchain_fmt;
	color_desc.blend_state.enable_blend = false;
	color_desc.blend_state.color_write_mask = 0xF;

	SDL_GPUGraphicsPipelineCreateInfo pipe{};
	pipe.vertex_shader = vs_;
	pipe.fragment_shader = ps_;
	pipe.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
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
		LogError() << "[SDL3] SkyBox pipeline create failed: " << SDL_GetError() << std::endl;
		Release(device);
		return false;
	}

	pipeline_fmt_ = swapchain_fmt;
	return true;
}

bool SDL3SkyBoxPresent::EnsureResources(SDL_GPUDevice* device, SDL_Window* window)
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
		LogInfo() << "[SDL3] SkyBox MSL present path ready" << std::endl;
	}
	return ready_;
}

void SDL3SkyBoxPresent::Draw(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex, SDL_GPUTexture* cube_tex,
	float4x4 const& inv_mvp)
{
	if (!ready_ || !cmd || !swapchain_tex || !cube_tex || !pipeline_ || !sampler_)
	{
		return;
	}

	struct Uniforms
	{
		float rows[4][4];
	} u{};
	std::memcpy(&u, inv_mvp.data(), sizeof(u));

	SDL_PushGPUVertexUniformData(cmd, 0, &u, sizeof(u));

	SDL_GPUColorTargetInfo color{};
	color.texture = swapchain_tex;
	color.clear_color.r = 0.0f;
	color.clear_color.g = 0.0f;
	color.clear_color.b = 0.0f;
	color.clear_color.a = 1.0f;
	color.load_op = SDL_GPU_LOADOP_CLEAR;
	color.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color, 1, nullptr);
	if (!pass)
	{
		LogError() << "[SDL3] SkyBox BeginGPURenderPass failed: " << SDL_GetError() << std::endl;
		return;
	}

	SDL_BindGPUGraphicsPipeline(pass, pipeline_);
	SDL_GPUTextureSamplerBinding bind{};
	bind.texture = cube_tex;
	bind.sampler = sampler_;
	SDL_BindGPUFragmentSamplers(pass, 0, &bind, 1);
	SDL_DrawGPUPrimitives(pass, 4, 1, 0, 0);
	SDL_EndGPURenderPass(pass);
}

} // namespace RenderWorker
