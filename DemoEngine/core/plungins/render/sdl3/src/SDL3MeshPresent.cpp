#include "SDL3MeshPresent.h"
#include "SDL3GraphicsBuffer.h"
#include "SDL3RenderLayout.h"
#include "SDL3Texture.h"
#include <base/App3D.h>
#include <base/Context.h>
#include <common/Log.h>
#include <render/FrameBuffer.h>
#include <render/Mesh.h>
#include <render/Renderable.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/RenderMaterial.h>
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

char const kMeshVsTexMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct Uniforms
{
	float4x4 wvp;
	float4 pos_center; // xyz
	float4 pos_extent; // xyz
	float4 color;      // xyz albedo tint
	float4 light_dir;  // xyz
	float4 tc_center;  // xy
	float4 tc_extent;  // xy
};

struct VSIn
{
	float4 pos [[attribute(0)]];
	float2 uv [[attribute(1)]];
};

struct VSOut
{
	float4 position [[position]];
	float3 world_pos;
	float3 color;
	float3 light_dir;
	float2 uv;
};

vertex VSOut mesh_vs_tex(VSIn in [[stage_in]], constant Uniforms& u [[buffer(0)]])
{
	float4 local = float4(in.pos.xyz * u.pos_extent.xyz + u.pos_center.xyz, 1.0);
	VSOut out;
	out.position = u.wvp * local;
	out.world_pos = local.xyz;
	out.color = u.color.xyz;
	out.light_dir = u.light_dir.xyz;
	out.uv = in.uv * u.tc_extent.xy + u.tc_center.xy;
	return out;
}
)MSL";

char const kMeshPsTexMsl[] = R"MSL(
#include <metal_stdlib>
using namespace metal;

struct VSOut
{
	float4 position [[position]];
	float3 world_pos;
	float3 color;
	float3 light_dir;
	float2 uv;
};

fragment float4 mesh_ps_tex(VSOut in [[stage_in]],
	texture2d<float> albedo [[texture(0)]],
	sampler smp [[sampler(0)]])
{
	float3 albedo_rgb = albedo.sample(smp, in.uv).rgb * in.color;
	float3 dx = dfdx(in.world_pos);
	float3 dy = dfdy(in.world_pos);
	float3 n = normalize(cross(dx, dy));
	float ndotl = abs(dot(n, normalize(in.light_dir))) * 0.55 + 0.45;
	return float4(albedo_rgb * ndotl, 1.0);
}
)MSL";

int FindVertexStream(RenderLayout const& rl, VertexElementUsage usage, ElementFormat format)
{
	for (uint32_t i = 0; i < rl.VertexStreamNum(); ++i)
	{
		auto const& ves = rl.VertexStreamFormat(i);
		if (ves.empty())
		{
			continue;
		}
		if (ves[0].usage == usage && ves[0].format == format)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

} // namespace

SDL3MeshPresent::~SDL3MeshPresent() = default;

void SDL3MeshPresent::Release(SDL_GPUDevice* device)
{
	if (!device)
	{
		vs_ = nullptr;
		ps_ = nullptr;
		pipeline_ = nullptr;
		vs_tex_ = nullptr;
		ps_tex_ = nullptr;
		pipeline_tex_ = nullptr;
		sampler_ = nullptr;
		ready_ = false;
		return;
	}
	if (pipeline_)
	{
		SDL_ReleaseGPUGraphicsPipeline(device, pipeline_);
		pipeline_ = nullptr;
	}
	if (pipeline_tex_)
	{
		SDL_ReleaseGPUGraphicsPipeline(device, pipeline_tex_);
		pipeline_tex_ = nullptr;
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
	if (vs_tex_)
	{
		SDL_ReleaseGPUShader(device, vs_tex_);
		vs_tex_ = nullptr;
	}
	if (ps_tex_)
	{
		SDL_ReleaseGPUShader(device, ps_tex_);
		ps_tex_ = nullptr;
	}
	if (sampler_)
	{
		SDL_ReleaseGPUSampler(device, sampler_);
		sampler_ = nullptr;
	}
	pipeline_color_fmt_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	pipeline_depth_fmt_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	ready_ = false;
}

bool SDL3MeshPresent::CreatePipelines(SDL_GPUDevice* device, SDL_GPUTextureFormat swapchain_fmt,
	SDL_GPUTextureFormat depth_fmt)
{
	SDL_GPUShaderFormat const supported = SDL_GetGPUShaderFormats(device);
	if (!(supported & SDL_GPU_SHADERFORMAT_MSL))
	{
		LogError() << "[SDL3] Mesh present: no MSL shader format" << std::endl;
		return false;
	}

	auto make_vs = [&](char const* code, char const* entry) -> SDL_GPUShader* {
		SDL_GPUShaderCreateInfo info{};
		info.stage = SDL_GPU_SHADERSTAGE_VERTEX;
		info.format = SDL_GPU_SHADERFORMAT_MSL;
		info.code = reinterpret_cast<Uint8 const*>(code);
		info.code_size = std::strlen(code);
		info.entrypoint = entry;
		info.num_uniform_buffers = 1;
		return SDL_CreateGPUShader(device, &info);
	};

	auto make_ps = [&](char const* code, char const* entry, Uint32 num_samplers) -> SDL_GPUShader* {
		SDL_GPUShaderCreateInfo info{};
		info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
		info.format = SDL_GPU_SHADERFORMAT_MSL;
		info.code = reinterpret_cast<Uint8 const*>(code);
		info.code_size = std::strlen(code);
		info.entrypoint = entry;
		info.num_samplers = num_samplers;
		return SDL_CreateGPUShader(device, &info);
	};

	vs_ = make_vs(kMeshVsMsl, "mesh_vs");
	if (!vs_)
	{
		LogError() << "[SDL3] Mesh VS create failed: " << SDL_GetError() << std::endl;
		return false;
	}
	ps_ = make_ps(kMeshPsMsl, "mesh_ps", 0);
	if (!ps_)
	{
		LogError() << "[SDL3] Mesh PS create failed: " << SDL_GetError() << std::endl;
		Release(device);
		return false;
	}
	vs_tex_ = make_vs(kMeshVsTexMsl, "mesh_vs_tex");
	if (!vs_tex_)
	{
		LogError() << "[SDL3] Mesh textured VS create failed: " << SDL_GetError() << std::endl;
		Release(device);
		return false;
	}
	ps_tex_ = make_ps(kMeshPsTexMsl, "mesh_ps_tex", 1);
	if (!ps_tex_)
	{
		LogError() << "[SDL3] Mesh textured PS create failed: " << SDL_GetError() << std::endl;
		Release(device);
		return false;
	}

	if (!sampler_)
	{
		SDL_GPUSamplerCreateInfo si{};
		si.min_filter = SDL_GPU_FILTER_LINEAR;
		si.mag_filter = SDL_GPU_FILTER_LINEAR;
		si.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
		si.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
		si.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
		si.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
		si.max_anisotropy = 1;
		si.enable_anisotropy = false;
		sampler_ = SDL_CreateGPUSampler(device, &si);
		if (!sampler_)
		{
			LogError() << "[SDL3] Mesh sampler create failed: " << SDL_GetError() << std::endl;
			Release(device);
			return false;
		}
	}

	SDL_GPUColorTargetDescription color_desc{};
	color_desc.format = swapchain_fmt;
	color_desc.blend_state.enable_blend = false;
	color_desc.blend_state.color_write_mask = 0xF;

	auto make_pipeline = [&](SDL_GPUShader* vs, SDL_GPUVertexBufferDescription const* vb_descs,
		Uint32 num_vbs, SDL_GPUVertexAttribute const* attrs, Uint32 num_attrs,
		SDL_GPUShader* ps) -> SDL_GPUGraphicsPipeline* {
		SDL_GPUVertexInputState vis{};
		vis.num_vertex_buffers = num_vbs;
		vis.vertex_buffer_descriptions = vb_descs;
		vis.num_vertex_attributes = num_attrs;
		vis.vertex_attributes = attrs;

		SDL_GPUGraphicsPipelineCreateInfo pipe{};
		pipe.vertex_shader = vs;
		pipe.fragment_shader = ps;
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
		return SDL_CreateGPUGraphicsPipeline(device, &pipe);
	};

	SDL_GPUVertexBufferDescription pos_vb_desc{};
	pos_vb_desc.slot = 0;
	pos_vb_desc.pitch = 8;
	pos_vb_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	pos_vb_desc.instance_step_rate = 0;

	SDL_GPUVertexAttribute pos_attr{};
	pos_attr.location = 0;
	pos_attr.buffer_slot = 0;
	pos_attr.format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM;
	pos_attr.offset = 0;

	pipeline_ = make_pipeline(vs_, &pos_vb_desc, 1, &pos_attr, 1, ps_);
	if (!pipeline_)
	{
		LogError() << "[SDL3] Mesh pipeline create failed: " << SDL_GetError() << std::endl;
		Release(device);
		return false;
	}

	SDL_GPUVertexBufferDescription tex_vb_descs[2]{pos_vb_desc, {}};
	tex_vb_descs[1].slot = 1;
	tex_vb_descs[1].pitch = 4;
	tex_vb_descs[1].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	tex_vb_descs[1].instance_step_rate = 0;

	SDL_GPUVertexAttribute tex_attrs[2]{pos_attr, {}};
	tex_attrs[1].location = 1;
	tex_attrs[1].buffer_slot = 1;
	tex_attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM;
	tex_attrs[1].offset = 0;

	pipeline_tex_ = make_pipeline(vs_tex_, tex_vb_descs, 2, tex_attrs, 2, ps_tex_);
	if (!pipeline_tex_)
	{
		LogError() << "[SDL3] Mesh textured pipeline create failed: " << SDL_GetError() << std::endl;
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
	if (ready_ && pipeline_ && pipeline_tex_ && pipeline_color_fmt_ == color_fmt && pipeline_depth_fmt_ == depth_fmt)
	{
		return true;
	}

	Release(device);
	ready_ = CreatePipelines(device, color_fmt, depth_fmt);
	if (ready_)
	{
		LogInfo() << "[SDL3] Mesh MSL present path ready (solid + textured)" << std::endl;
	}
	return ready_;
}

void SDL3MeshPresent::DrawSceneMeshes(SDL_GPUCommandBuffer* cmd, SDL_GPUTexture* swapchain_tex,
	SDL_GPUTexture* depth_tex, float4x4 const& view_proj)
{
	if (!ready_ || !cmd || !swapchain_tex || !depth_tex || !pipeline_ || !pipeline_tex_ || !sampler_)
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
		float tc_center[4];
		float tc_extent[4];
	};

	struct DrawItem
	{
		Uniforms uniforms{};
		SDL_GPUBuffer* vb_pos{nullptr};
		SDL_GPUBuffer* vb_uv{nullptr};
		Uint32 vb_pos_offset{0};
		Uint32 vb_uv_offset{0};
		SDL_GPUBuffer* ib{nullptr};
		SDL_GPUIndexElementSize index_size{SDL_GPU_INDEXELEMENTSIZE_16BIT};
		uint32_t num_indices{0};
		uint32_t start_index{0};
		Sint32 start_vertex{0};
		SDL_GPUTexture* albedo_tex{nullptr};
		bool textured{false};
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

				int const pos_stream = FindVertexStream(rl, VEU_Position, EF_SIGNED_ABGR16);
				if (pos_stream < 0 || rl.VertexSize(static_cast<uint32_t>(pos_stream)) != 8)
				{
					return;
				}

				int const uv_stream = FindVertexStream(rl, VEU_TextureCoord, EF_SIGNED_GR16);

				auto* sdl_rl = dynamic_cast<SDL3RenderLayout*>(&rl);
				if (!sdl_rl)
				{
					return;
				}
				sdl_rl->Active();
				auto const& vbs = sdl_rl->VertexBuffers();
				if (static_cast<size_t>(pos_stream) >= vbs.size() || !vbs[static_cast<size_t>(pos_stream)]
					|| !sdl_rl->IndexBuffer())
				{
					return;
				}

				auto const& offsets = sdl_rl->VertexOffsets();

				RenderMaterialPtr const& mtl = mesh->Material();
				SDL_GPUTexture* albedo_gpu = nullptr;
				float3 albedo_tint(0.86f, 0.80f, 0.72f);
				if (mtl)
				{
					float4 const& albedo = mtl->Albedo();
					albedo_tint = float3(albedo.x(), albedo.y(), albedo.z());
					auto const& srv = mtl->Texture(RenderMaterial::TS_Albedo);
					if (srv && srv->TextureResource())
					{
						if (auto* tex = dynamic_cast<SDL3Texture*>(srv->TextureResource().get()))
						{
							albedo_gpu = tex->GpuTexture();
						}
					}
				}

				bool const can_texture = albedo_gpu && uv_stream >= 0
					&& static_cast<size_t>(uv_stream) < vbs.size() && vbs[static_cast<size_t>(uv_stream)];

				float4x4 const model = node.TransformToWorld();
				float4x4 const wvp = model * view_proj;
				AABBox const& bb = mesh->PosBound();
				float3 const center = bb.Center();
				float3 const extent = bb.HalfSize();
				AABBox const& tc_bb = mesh->TexcoordBound();

				DrawItem item{};
				std::memcpy(item.uniforms.wvp, wvp.data(), sizeof(item.uniforms.wvp));
				item.uniforms.pos_center[0] = center.x();
				item.uniforms.pos_center[1] = center.y();
				item.uniforms.pos_center[2] = center.z();
				item.uniforms.pos_extent[0] = extent.x();
				item.uniforms.pos_extent[1] = extent.y();
				item.uniforms.pos_extent[2] = extent.z();
				item.uniforms.color[0] = albedo_tint.x();
				item.uniforms.color[1] = albedo_tint.y();
				item.uniforms.color[2] = albedo_tint.z();
				item.uniforms.light_dir[0] = 0.35f;
				item.uniforms.light_dir[1] = 0.85f;
				item.uniforms.light_dir[2] = 0.45f;
				item.uniforms.tc_center[0] = tc_bb.Center().x();
				item.uniforms.tc_center[1] = tc_bb.Center().y();
				item.uniforms.tc_extent[0] = tc_bb.HalfSize().x();
				item.uniforms.tc_extent[1] = tc_bb.HalfSize().y();
				item.vb_pos = vbs[static_cast<size_t>(pos_stream)];
				item.vb_pos_offset = offsets.empty() ? 0 : offsets[static_cast<size_t>(pos_stream)];
				if (can_texture)
				{
					item.vb_uv = vbs[static_cast<size_t>(uv_stream)];
					item.vb_uv_offset = offsets.empty() ? 0 : offsets[static_cast<size_t>(uv_stream)];
					item.albedo_tex = albedo_gpu;
					item.textured = true;
				}
				item.ib = sdl_rl->IndexBuffer();
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

	uint32_t textured_count = 0;
	for (DrawItem const& item : items)
	{
		if (item.textured)
		{
			SDL_BindGPUGraphicsPipeline(pass, pipeline_tex_);
			SDL_PushGPUVertexUniformData(cmd, 0, &item.uniforms, sizeof(item.uniforms));

			SDL_GPUBufferBinding vb_binds[2]{};
			vb_binds[0].buffer = item.vb_pos;
			vb_binds[0].offset = item.vb_pos_offset;
			vb_binds[1].buffer = item.vb_uv;
			vb_binds[1].offset = item.vb_uv_offset;
			SDL_BindGPUVertexBuffers(pass, 0, vb_binds, 2);

			SDL_GPUTextureSamplerBinding tex_bind{};
			tex_bind.texture = item.albedo_tex;
			tex_bind.sampler = sampler_;
			SDL_BindGPUFragmentSamplers(pass, 0, &tex_bind, 1);
			++textured_count;
		}
		else
		{
			SDL_BindGPUGraphicsPipeline(pass, pipeline_);
			Uniforms solid_uniforms{};
			std::memcpy(solid_uniforms.wvp, item.uniforms.wvp, sizeof(solid_uniforms.wvp));
			std::memcpy(solid_uniforms.pos_center, item.uniforms.pos_center, sizeof(solid_uniforms.pos_center));
			std::memcpy(solid_uniforms.pos_extent, item.uniforms.pos_extent, sizeof(solid_uniforms.pos_extent));
			std::memcpy(solid_uniforms.color, item.uniforms.color, sizeof(solid_uniforms.color));
			std::memcpy(solid_uniforms.light_dir, item.uniforms.light_dir, sizeof(solid_uniforms.light_dir));
			SDL_PushGPUVertexUniformData(cmd, 0, &solid_uniforms, sizeof(solid_uniforms));

			SDL_GPUBufferBinding vb_bind{};
			vb_bind.buffer = item.vb_pos;
			vb_bind.offset = item.vb_pos_offset;
			SDL_BindGPUVertexBuffers(pass, 0, &vb_bind, 1);
		}

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
		LogInfo() << "[SDL3] Mesh present drew " << items.size() << " mesh(es), " << textured_count
				  << " textured" << std::endl;
	}
}

} // namespace RenderWorker
