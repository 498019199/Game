#include "SDL3MeshPresent.h"
#include "SDL3GraphicsBuffer.h"
#include "SDL3RenderLayout.h"
#include <base/App3D.h>
#include <base/Context.h>
#include <common/Log.h>
#include <render/FrameBuffer.h>
#include <render/Mesh.h>
#include <render/Renderable.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <world/SceneNode.h>
#include <world/World.h>
#include <math/math.h>
#include <cstring>
#include <mutex>
#include <vector>

namespace RenderWorker
{
namespace
{

char const kMeshVsMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct Uniforms
{
	float4x4 wvp;
	float4 pos_center; // xyz
	float4 pos_extent; // xyz
	float4 color;      // xyz
	float4 light_dir;  // xyz
};

struct VSIn
{
	float4 pos [[attribute(0)]];
};

struct VSOut
{
	float4 position [[position]];
	float3 world_pos;
	float3 color;
	float3 light_dir;
};

vertex VSOut mesh_vs(VSIn in [[stage_in]], constant Uniforms& u [[buffer(0)]])
{
	float4 local = float4(in.pos.xyz * u.pos_extent.xyz + u.pos_center.xyz, 1.0);
	VSOut out;
	// Engine stores row-major M; memcpy into Metal float4x4 reads as M^T.
	// So (u.wvp * local) == HLSL mul(local, M) with M = model * viewproj.
	out.position = u.wvp * local;
	out.world_pos = local.xyz;
	out.color = u.color.xyz;
	out.light_dir = u.light_dir.xyz;
	return out;
}
)MSL";

char const kMeshPsMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct VSOut
{
	float4 position [[position]];
	float3 world_pos;
	float3 color;
	float3 light_dir;
};

fragment float4 mesh_ps(VSOut in [[stage_in]])
{
	float3 dx = dfdx(in.world_pos);
	float3 dy = dfdy(in.world_pos);
	float3 n = normalize(cross(dx, dy));
	float ndotl = abs(dot(n, normalize(in.light_dir))) * 0.55 + 0.45;
	return float4(in.color * ndotl, 1.0);
}
)MSL";

} // namespace

SDL3MeshPresent::~SDL3MeshPresent() = default;

void SDL3MeshPresent::Release(SDL_GPUDevice* device)
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
	pipeline_color_fmt_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	pipeline_depth_fmt_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	ready_ = false;
}

bool SDL3MeshPresent::CreatePipeline(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt,
	SDL_GPUTextureFormat depth_fmt)
{
	SDL_GPUShaderFormat const supported = SDL_GetGPUShaderFormats(device);
	if (!(supported & SDL_GPU_SHADERFORMAT_MSL))
	{
		LogError() << "[SDL3] Mesh present: no MSL shader format" << std::endl;
		return false;
	}

	SDL_GPUShaderCreateInfo vs_info{};
	vs_info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
	vs_info.format = SDL_GPU_SHADERFORMAT_MSL;
	vs_info.code = reinterpret_cast<Uint8 const*>(kMeshVsMsl);
	vs_info.code_size = std::strlen(kMeshVsMsl);
	vs_info.entrypoint = "mesh_vs";
	vs_info.num_uniform_buffers = 1;

	SDL_GPUShaderCreateInfo ps_info{};
	ps_info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	ps_info.format = SDL_GPU_SHADERFORMAT_MSL;
	ps_info.code = reinterpret_cast<Uint8 const*>(kMeshPsMsl);
	ps_info.code_size = std::strlen(kMeshPsMsl);
	ps_info.entrypoint = "mesh_ps";

	vs_ = SDL_CreateGPUShader(device, &vs_info);
	if (!vs_)
	{
		LogError() << "[SDL3] Mesh VS create failed: " << SDL_GetError() << std::endl;
		return false;
	}
	ps_ = SDL_CreateGPUShader(device, &ps_info);
	if (!ps_)
	{
		LogError() << "[SDL3] Mesh PS create failed: " << SDL_GetError() << std::endl;
		SDL_ReleaseGPUShader(device, vs_);
		vs_ = nullptr;
		return false;
	}

	SDL_GPUVertexBufferDescription vb_desc{};
	vb_desc.slot = 0;
	vb_desc.pitch = 8; // default SIGNED_ABGR16; overridden per-draw via... (fixed in CreatePipeline)
	vb_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vb_desc.instance_step_rate = 0;

	SDL_GPUVertexAttribute attr{};
	attr.location = 0;
	attr.buffer_slot = 0;
	attr.format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM;
	attr.offset = 0;

	SDL_GPUVertexInputState vis{};
	vis.num_vertex_buffers = 1;
	vis.vertex_buffer_descriptions = &vb_desc;
	vis.num_vertex_attributes = 1;
	vis.vertex_attributes = &attr;

	SDL_GPUColorTargetDescription color_desc{};
	color_desc.format = swapchain_fmt;
	color_desc.blend_state.enable_blend = false;
	color_desc.blend_state.color_write_mask = 0xF;

	SDL_GPUGraphicsPipelineCreateInfo pipe{};
	pipe.vertex_shader = vs_;
	pipe.fragment_shader = ps_;
	pipe.vertex_input_state = vis;
	pipe.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipe.target_info.num_color_targets = 1;
	pipe.target_info.color_target_descriptions = &color_desc;
	pipe.target_info.has_depth_stencil_target = true;
	pipe.target_info.depth_stencil_format = depth_fmt;
	pipe.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	pipe.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
	pipe.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	pipe.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
	pipe.depth_stencil_state.enable_depth_test = true;
	pipe.depth_stencil_state.enable_depth_write = true;
	pipe.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
	pipe.depth_stencil_state.enable_stencil_test = false;

	pipeline_ = SDL_CreateGPUGraphicsPipeline(device, &pipe);
	if (!pipeline_)
	{
		LogError() << "[SDL3] Mesh pipeline create failed: " << SDL_GetError() << std::endl;
		Release(device);
		return false;
	}

	pipeline_color_fmt_ = swapchain_fmt;
	pipeline_depth_fmt_ = depth_fmt;
	return true;
}

bool SDL3MeshPresent::EnsureResources(SDL_GPUDevice* device, SDL_Window* window, SDL_GPUTextureFormat depth_fmt)
{
	if (!device || !window || depth_fmt == SDL_GPU_TEXTUREFORMAT_INVALID)
	{
		return false;
	}

	SDL_GPUTextureFormat const color_fmt = SDL_GetGPUSwapchainTextureFormat(device, window);
	if (ready_ && pipeline_ && pipeline_color_fmt_ == color_fmt && pipeline_depth_fmt_ == depth_fmt)
	{
		return true;
	}

	Release(device);
	ready_ = CreatePipeline(device, color_fmt, depth_fmt);
	if (ready_)
	{
		LogInfo() << "[SDL3] Mesh MSL present path ready" << std::endl;
	}
	return ready_;
}

void SDL3MeshPresent::DrawSceneMeshes(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex,
	SDL_GPUTexture* depth_tex, float4x4 const& view_proj)
{
	if (!ready_ || !cmd || !swapchain_tex || !depth_tex || !pipeline_)
	{
		return;
	}

	struct Uniforms
	{
		float wvp[16];
		float pos_center[4];
		float pos_extent[4];
		float color[4];
		float light_dir[4];
	};

	struct DrawItem
	{
		Uniforms uniforms{};
		SDL_GPUBuffer* vb{nullptr};
		SDL_GPUBuffer* ib{nullptr};
		Uint32 vb_offset{0};
		SDL_GPUIndexElementSize index_size{SDL_GPU_INDEXELEMENTSIZE_16BIT};
		uint32_t num_indices{0};
		uint32_t start_index{0};
		Sint32 start_vertex{0};
	};

	std::vector<DrawItem> items;
	uint32_t vp_w = 0;
	uint32_t vp_h = 0;
	{
		// Brief lock: snapshot only. Never GPU-wait while holding update_mutex_.
		std::lock_guard<std::mutex> scene_lock(Context::Instance().WorldInstance().MutexForUpdate());
		if (Context::Instance().AppValid())
		{
			FrameBufferPtr const& fb =
				Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
			if (fb)
			{
				vp_w = fb->Width();
				vp_h = fb->Height();
			}
		}

		Context::Instance().WorldInstance().SceneRootNode().Traverse([&](SceneNode& node) {
			if (node.Name() == L"SkyBox")
			{
				return true;
			}

			node.ForEachComponentOfType<RenderableComponent>([&](RenderableComponent& comp) {
				auto* mesh = dynamic_cast<StaticMesh*>(&comp.BoundRenderable());
				if (!mesh || !mesh->HWResourceReady())
				{
					return;
				}

				auto& rl = mesh->GetRenderLayout();
				if (!rl.UseIndices() || rl.NumIndices() == 0 || rl.VertexStreamNum() == 0)
				{
					return;
				}

				auto const& ves = rl.VertexStreamFormat(0);
				if (ves.empty() || ves[0].usage != VEU_Position || ves[0].format != EF_SIGNED_ABGR16
					|| rl.VertexSize(0) != 8)
				{
					return;
				}

				auto* sdl_rl = dynamic_cast<SDL3RenderLayout*>(&rl);
				if (!sdl_rl)
				{
					return;
				}
				sdl_rl->Active();
				auto const& vbs = sdl_rl->VertexBuffers();
				if (vbs.empty() || !vbs[0] || !sdl_rl->IndexBuffer())
				{
					return;
				}

				float4x4 const model = node.TransformToWorld();
				float4x4 const model = node.TransformToWorld();
				// Same as DetailedMesh: M = model * view_proj for HLSL mul(pos, M).
				// Do NOT transpose — row-major→Metal memcpy already supplies M^T for (U*v).
				float4x4 const wvp = model * view_proj;
				AABBox const& bb = mesh->PosBound();
				float3 const center = bb.Center();
				float3 const extent = bb.HalfSize();

				DrawItem item{};
				std::memcpy(item.uniforms.wvp, wvp.data(), sizeof(item.uniforms.wvp));
				item.uniforms.pos_center[0] = center.x();
				item.uniforms.pos_center[1] = center.y();
				item.uniforms.pos_center[2] = center.z();
				item.uniforms.pos_extent[0] = extent.x();
				item.uniforms.pos_extent[1] = extent.y();
				item.uniforms.pos_extent[2] = extent.z();
				item.uniforms.color[0] = 0.86f;
				item.uniforms.color[1] = 0.80f;
				item.uniforms.color[2] = 0.72f;
				item.uniforms.light_dir[0] = 0.35f;
				item.uniforms.light_dir[1] = 0.85f;
				item.uniforms.light_dir[2] = 0.45f;
				item.vb = vbs[0];
				item.ib = sdl_rl->IndexBuffer();
				item.vb_offset = sdl_rl->VertexOffsets().empty() ? 0 : sdl_rl->VertexOffsets()[0];
				item.index_size = sdl_rl->IndexElementSize();
				item.num_indices = rl.NumIndices();
				item.start_index = rl.StartIndexLocation();
				item.start_vertex = static_cast<Sint32>(rl.StartVertexLocation());
				items.push_back(item);
			});
			return true;
		});
	}

	if (items.empty())
	{
		return;
	}

	SDL_GPUColorTargetInfo color{};
	color.texture = swapchain_tex;
	color.load_op = SDL_GPU_LOADOP_LOAD;
	color.store_op = SDL_GPU_STOREOP_STORE;

	SDL_GPUDepthStencilTargetInfo depth{};
	depth.texture = depth_tex;
	depth.clear_depth = 1.0f;
	depth.load_op = SDL_GPU_LOADOP_CLEAR;
	depth.store_op = SDL_GPU_STOREOP_STORE;
	depth.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
	depth.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

	SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &color, 1, &depth);
	if (!pass)
	{
		LogError() << "[SDL3] Mesh BeginGPURenderPass failed: " << SDL_GetError() << std::endl;
		return;
	}

	SDL_BindGPUGraphicsPipeline(pass, pipeline_);
	if (vp_w > 0 && vp_h > 0)
	{
		SDL_GPUViewport vp{};
		vp.x = 0;
		vp.y = 0;
		vp.w = static_cast<float>(vp_w);
		vp.h = static_cast<float>(vp_h);
		vp.min_depth = 0.0f;
		vp.max_depth = 1.0f;
		SDL_SetGPUViewport(pass, &vp);
	}

	for (DrawItem const& item : items)
	{
		SDL_PushGPUVertexUniformData(cmd, 0, &item.uniforms, sizeof(item.uniforms));

		SDL_GPUBufferBinding vb_bind{};
		vb_bind.buffer = item.vb;
		vb_bind.offset = item.vb_offset;
		SDL_BindGPUVertexBuffers(pass, 0, &vb_bind, 1);

		SDL_GPUBufferBinding ib_bind{};
		ib_bind.buffer = item.ib;
		ib_bind.offset = 0;
		SDL_BindGPUIndexBuffer(pass, &ib_bind, item.index_size);

		SDL_DrawGPUIndexedPrimitives(pass, item.num_indices, 1, item.start_index, item.start_vertex, 0);
	}

	SDL_EndGPURenderPass(pass);

	static bool logged = false;
	if (!logged)
	{
		logged = true;
		LogInfo() << "[SDL3] Mesh present drew " << items.size() << " untextured mesh(es)" << std::endl;
	}
}

} // namespace RenderWorker
