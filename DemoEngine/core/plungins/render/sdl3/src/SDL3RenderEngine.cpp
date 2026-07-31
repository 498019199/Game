#include "SDL3RenderEngine.h"
#include "SDL3RenderWindow.h"
#include "SDL3RenderLayout.h"
#include "SDL3ShaderObject.h"
#include "SDL3RenderStateObject.h"
#include "SDL3GraphicsBuffer.h"
#include "SDL3Texture.h"
#include <base/ZEngine.h>
#include <render/RenderEffect.h>
#include <render/RenderDeviceCaps.h>
#include <render/ElementFormat.h>
#include <render/FrameBuffer.h>
#include <render/RenderView.h>
#include <common/Log.h>
#include <math/color.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <vector>

#if defined(ZENGINE_PLATFORM_WINDOWS)
#include <d3dcommon.h>
#endif

namespace RenderWorker
{

SDL3RenderEngine::SDL3RenderEngine() = default;

SDL3RenderEngine::~SDL3RenderEngine()
{
	this->Destroy();
}

void SDL3RenderEngine::Device(SDL_GPUDevice* device, SDL_Window* window)
{
	device_ = device;
	window_ = window;
	this->FillRenderDeviceCaps();
	EnsureDefaultSampleBinding();
}

void SDL3RenderEngine::FillRenderDeviceCaps()
{
	caps_.max_shader_model = ShaderModel(5, 0);
	caps_.max_texture_width = 16384;
	caps_.max_texture_height = 16384;
	caps_.max_texture_depth = 2048;
	caps_.max_texture_cube_size = 16384;
	caps_.max_texture_array_length = 2048;
	caps_.max_vertex_texture_units = 16;
	caps_.max_pixel_texture_units = 16;
	caps_.max_geometry_texture_units = 16;
	caps_.max_simultaneous_rts = 8;
	caps_.max_simultaneous_uavs = 8;
	caps_.max_vertex_streams = 16;
	caps_.max_texture_anisotropy = 16;

	caps_.primitive_restart_support = true;
	caps_.multithread_rendering_support = false;
	caps_.multithread_res_creating_support = false;
	caps_.arbitrary_multithread_rendering_support = false;
	caps_.mrt_independent_bit_depths_support = true;
	caps_.logic_op_support = false;
	caps_.independent_blend_support = true;
	caps_.depth_texture_support = true;
	caps_.fp_color_support = true;
	caps_.pack_to_rgba_required = false;
	caps_.draw_indirect_support = false;
	caps_.no_overwrite_support = true;
	caps_.full_npot_texture_support = true;
	caps_.render_to_texture_array_support = true;
	caps_.explicit_multi_sample_support = true;
	caps_.load_from_buffer_support = true;
	caps_.uavs_at_every_stage_support = false;
	caps_.rovs_support = false;
	caps_.flexible_srvs_support = true;
	caps_.vp_rt_index_at_every_stage_support = false;
	caps_.gs_support = false;
	caps_.cs_support = false;
	caps_.hs_support = false;
	caps_.ds_support = false;

	// TextureFormatSupport is consulted during SyncLoadTexture; without this list every format fails.
	std::vector<ElementFormat> texture_formats = {
		EF_R8,
		EF_GR8,
		EF_BGR8,
		EF_ARGB8,
		EF_ABGR8,
		EF_ARGB8_SRGB,
		EF_ABGR8_SRGB,
		EF_A2BGR10,
		EF_R16F,
		EF_GR16F,
		EF_ABGR16F,
		EF_R32F,
		EF_GR32F,
		EF_ABGR32F,
		EF_BC1,
		EF_BC1_SRGB,
		EF_BC2,
		EF_BC2_SRGB,
		EF_BC3,
		EF_BC3_SRGB,
		EF_BC4,
		EF_BC5,
		EF_D16,
		EF_D24S8,
		EF_D32F,
	};
	std::vector<ElementFormat> vertex_formats = {
		EF_R32F,
		EF_GR32F,
		EF_BGR32F,
		EF_ABGR32F,
		EF_ABGR16F,
		EF_ABGR8,
		EF_ARGB8,
		EF_R16F,
		EF_GR16F,
	};
	std::map<ElementFormat, std::vector<uint32_t>> render_target_formats;
	auto const sample1 = RenderDeviceCaps::EncodeSampleCountQuality(1, 1);
	for (ElementFormat fmt : {EF_ARGB8, EF_ABGR8, EF_ARGB8_SRGB, EF_ABGR8_SRGB, EF_ABGR16F, EF_R32F,
			EF_D16, EF_D24S8, EF_D32F})
	{
		render_target_formats[fmt] = {sample1};
	}

	caps_.AssignVertexFormats(std::move(vertex_formats));
	caps_.AssignTextureFormats(std::move(texture_formats));
	caps_.AssignRenderTargetFormats(std::move(render_target_formats));
}

void SDL3RenderEngine::BeginFrame()
{
	RenderEngine::BeginFrame();
	EndRenderPass();
	if (!device_ || !window_)
	{
		return;
	}

	cmd_ = SDL_AcquireGPUCommandBuffer(device_);
	if (!cmd_)
	{
		LogError() << "[SDL3] AcquireGPUCommandBuffer failed: " << SDL_GetError() << std::endl;
		return;
	}

	Uint32 sw = 0;
	Uint32 sh = 0;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_, window_, &swapchain_tex_, &sw, &sh))
	{
		LogError() << "[SDL3] WaitAndAcquireGPUSwapchainTexture failed: " << SDL_GetError() << std::endl;
		swapchain_tex_ = nullptr;
	}
}

void SDL3RenderEngine::EndFrame()
{
	EndRenderPass();
	if (cmd_)
	{
		SDL_SubmitGPUCommandBuffer(cmd_);
		cmd_ = nullptr;
		swapchain_tex_ = nullptr;
	}
	RenderEngine::EndFrame();
}

void SDL3RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	scissor_x_ = x;
	scissor_y_ = y;
	scissor_w_ = width;
	scissor_h_ = height;
	scissor_valid_ = true;
	if (render_pass_)
	{
		SDL_Rect r{static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)};
		SDL_SetGPUScissor(render_pass_, &r);
	}
}

void SDL3RenderEngine::ResolvePassTargets()
{
	pass_colors_.clear();
	pass_depth_ = nullptr;
	pass_depth_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	pass_has_depth_ = false;

	auto const& fb = this->CurFrameBuffer();
	if (!fb)
	{
		return;
	}

	uint32_t const max_rts = caps_.max_simultaneous_rts;
	for (uint32_t i = 0; i < max_rts; ++i)
	{
		auto const& rtv = fb->AttachedRtv(static_cast<FrameBuffer::Attachment>(i));
		if (!rtv)
		{
			break;
		}
		auto const& tex = rtv->TextureResource();
		if (!tex)
		{
			break;
		}
		auto* gpu = checked_cast<SDL3Texture&>(*tex).GpuTexture();
		if (!gpu)
		{
			LogError() << "[SDL3] RTV texture has no GPU resource" << std::endl;
			break;
		}
		ColorTarget ct{};
		ct.texture = gpu;
		ct.format = SDL3Mapping::MappingFormat(rtv->Format());
		ct.mip_level = rtv->Level();
		ct.layer = rtv->FirstArrayIndex();
		pass_colors_.push_back(ct);
	}

	// Screen / default FB has depth but no color RTV — color is the swapchain.
	if (pass_colors_.empty() && swapchain_tex_ && device_ && window_)
	{
		ColorTarget ct{};
		ct.texture = swapchain_tex_;
		ct.format = SDL_GetGPUSwapchainTextureFormat(device_, window_);
		pass_colors_.push_back(ct);
	}

	if (auto const& dsv = fb->AttachedDsv())
	{
		if (auto const& tex = dsv->TextureResource())
		{
			pass_depth_ = checked_cast<SDL3Texture&>(*tex).GpuTexture();
			if (pass_depth_)
			{
				pass_depth_format_ = SDL3Mapping::MappingFormat(dsv->Format());
				pass_has_depth_ = true;
			}
		}
	}
}

	void SDL3RenderEngine::ApplyViewportAndScissor()
	{
		if (!render_pass_)
		{
			return;
		}

		float vp_x = 0.0f;
		float vp_y = 0.0f;
		float vp_w = 0.0f;
		float vp_h = 0.0f;

		auto const& fb = this->CurFrameBuffer();
		if (fb && fb->Viewport())
		{
			auto const& vp = *fb->Viewport();
			vp_x = static_cast<float>(vp.Left());
			vp_y = static_cast<float>(vp.Top());
			vp_w = static_cast<float>(vp.Width());
			vp_h = static_cast<float>(vp.Height());
		}

		// Never overwrite SDL's full-target default with a zero viewport — that
		// yields draws with zero coverage while clears still fill the RT.
		if (vp_w <= 0.0f || vp_h <= 0.0f)
		{
			if (fb && fb->Width() > 0 && fb->Height() > 0)
			{
				vp_w = static_cast<float>(fb->Width());
				vp_h = static_cast<float>(fb->Height());
			}
			else
			{
				return;
			}
		}

		SDL_GPUViewport gpu_vp{};
		gpu_vp.x = vp_x;
		gpu_vp.y = vp_y;
		gpu_vp.w = vp_w;
		gpu_vp.h = vp_h;
		gpu_vp.min_depth = 0.0f;
		gpu_vp.max_depth = 1.0f;
		SDL_SetGPUViewport(render_pass_, &gpu_vp);

		// Always keep scissor in sync with the viewport. Stale RmlUi LTRB values
		// (passed as xywh) must not leave a zero/tiny scissor across passes.
		int sc_x = static_cast<int>(vp_x);
		int sc_y = static_cast<int>(vp_y);
		int sc_w = static_cast<int>(vp_w);
		int sc_h = static_cast<int>(vp_h);
		if (scissor_valid_ && scissor_w_ > 0 && scissor_h_ > 0)
		{
			sc_x = static_cast<int>(scissor_x_);
			sc_y = static_cast<int>(scissor_y_);
			sc_w = static_cast<int>(scissor_w_);
			sc_h = static_cast<int>(scissor_h_);
		}
		SDL_Rect r{sc_x, sc_y, sc_w, sc_h};
		SDL_SetGPUScissor(render_pass_, &r);

		static bool logged_vp = false;
		if (!logged_vp)
		{
			logged_vp = true;
			LogInfo() << "[SDL3] Viewport " << vp_w << "x" << vp_h << " @(" << vp_x << "," << vp_y
					  << ") scissor " << sc_w << "x" << sc_h << std::endl;
		}
	}

void SDL3RenderEngine::EnsureRenderPass(bool clear_color, Color const* clear_clr, bool clear_depth, float depth,
	bool clear_stencil, int32_t stencil)
{
	if (render_pass_ || !cmd_)
	{
		return;
	}

	ResolvePassTargets();
	if (pass_colors_.empty())
	{
		return;
	}

	std::vector<SDL_GPUColorTargetInfo> color_infos(pass_colors_.size());
	for (size_t i = 0; i < pass_colors_.size(); ++i)
	{
		auto& color_info = color_infos[i];
		color_info.texture = pass_colors_[i].texture;
		color_info.mip_level = pass_colors_[i].mip_level;
		color_info.layer_or_depth_plane = pass_colors_[i].layer;
		color_info.load_op = clear_color ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		color_info.store_op = SDL_GPU_STOREOP_STORE;
		if (clear_clr)
		{
			color_info.clear_color.r = clear_clr->r();
			color_info.clear_color.g = clear_clr->g();
			color_info.clear_color.b = clear_clr->b();
			color_info.clear_color.a = clear_clr->a();
		}
	}

	SDL_GPUDepthStencilTargetInfo ds_info{};
	SDL_GPUDepthStencilTargetInfo* ds_ptr = nullptr;
	if (pass_has_depth_ && pass_depth_)
	{
		ds_info.texture = pass_depth_;
		ds_info.clear_depth = depth;
		ds_info.clear_stencil = static_cast<Uint8>(stencil);
		ds_info.load_op = clear_depth ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		ds_info.store_op = SDL_GPU_STOREOP_STORE;
		ds_info.stencil_load_op = clear_stencil ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
		ds_info.stencil_store_op = SDL_GPU_STOREOP_STORE;
		ds_ptr = &ds_info;
	}

	render_pass_ = SDL_BeginGPURenderPass(cmd_, color_infos.data(), static_cast<Uint32>(color_infos.size()), ds_ptr);
	if (!render_pass_)
	{
		LogError() << "[SDL3] SDL_BeginGPURenderPass failed: " << SDL_GetError() << std::endl;
		return;
	}
	ApplyViewportAndScissor();
}

void SDL3RenderEngine::EndRenderPass()
{
	if (render_pass_)
	{
		SDL_EndGPURenderPass(render_pass_);
		render_pass_ = nullptr;
	}
}

void SDL3RenderEngine::DoCreateRenderWindow(std::string const& name, RenderSettings const& settings)
{
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL3Check(SDL_InitSubSystem(SDL_INIT_VIDEO), "SDL_InitSubSystem(SDL_INIT_VIDEO)");
		owns_sdl_init_ = true;
	}

	auto win = MakeSharedPtr<SDL3RenderWindow>(name, settings);
	native_shader_platform_name_ = "d3d_12";
	this->BindFrameBuffer(win);
}

SDL_GPUGraphicsPipeline* SDL3RenderEngine::GetOrCreatePipeline(const RenderEffect& effect, const RenderPass& pass,
	const RenderLayout& rl)
{
	auto so = pass.GetShaderObject(effect);
	if (!so)
	{
		return nullptr;
	}
	auto& sdl_so = checked_cast<SDL3ShaderObject&>(*so);
	sdl_so.Bind(effect);

	auto* vs = sdl_so.VertexShader();
	auto* ps = sdl_so.PixelShader();
	if (!vs || !ps)
	{
		return nullptr;
	}

	if (pass_colors_.empty())
	{
		ResolvePassTargets();
	}
	if (pass_colors_.empty() || pass_colors_.size() > kMaxPipelineColorTargets)
	{
		return nullptr;
	}

	auto const& state = pass.GetRenderStateObject();
	PipelineKey key{};
	key.vs = vs;
	key.ps = ps;
	key.layout = &rl;
	key.state = state.get();
	key.topo = rl.TopologyType();
	key.num_colors = static_cast<uint32_t>(pass_colors_.size());
	for (uint32_t i = 0; i < key.num_colors; ++i)
	{
		key.color_fmts[i] = pass_colors_[i].format;
	}
	key.has_depth = pass_has_depth_;
	key.depth_fmt = pass_depth_format_;

	auto it = pipelines_.find(key);
	if (it != pipelines_.end())
	{
		return it->second;
	}

	auto const& sdl_rl = checked_cast<SDL3RenderLayout const&>(rl);
		sdl_rl.Active();

		if (sdl_rl.VertexAttributes().empty() && (rl.VertexStreamNum() > 0))
		{
			LogError() << "[SDL3] Refusing pipeline create: layout has streams but no vertex attributes"
					   << std::endl;
			return nullptr;
		}

		{
			auto const& attrs = sdl_rl.VertexAttributes();
			auto const& descs = sdl_rl.VertexBufferDescs();
			LogInfo() << "[SDL3] CreatePSO streams=" << descs.size() << " attrs=" << attrs.size() << std::endl;
			for (size_t ai = 0; ai < attrs.size(); ++ai)
			{
				LogInfo() << "[SDL3]  attr[" << ai << "] loc=" << attrs[ai].location
						  << " slot=" << attrs[ai].buffer_slot << " fmt=" << static_cast<int>(attrs[ai].format)
						  << " off=" << attrs[ai].offset << std::endl;
			}
			for (size_t di = 0; di < descs.size(); ++di)
			{
				LogInfo() << "[SDL3]  vb[" << di << "] slot=" << descs[di].slot
						  << " pitch=" << descs[di].pitch << std::endl;
			}
		}

		SDL_GPUGraphicsPipelineCreateInfo info{};
		info.vertex_shader = vs;
		info.fragment_shader = ps;
		info.vertex_input_state.vertex_buffer_descriptions = sdl_rl.VertexBufferDescs().data();
		info.vertex_input_state.num_vertex_buffers = static_cast<Uint32>(sdl_rl.VertexBufferDescs().size());
		info.vertex_input_state.vertex_attributes = sdl_rl.VertexAttributes().data();
		info.vertex_input_state.num_vertex_attributes = static_cast<Uint32>(sdl_rl.VertexAttributes().size());
		info.primitive_type = SDL3Mapping::Mapping(rl.TopologyType());

	auto const& rs = state->GetRasterizerStateDesc();
	info.rasterizer_state.fill_mode = SDL3Mapping::Mapping(rs.polygon_mode);
	info.rasterizer_state.cull_mode = SDL3Mapping::Mapping(rs.cull_mode);
	info.rasterizer_state.front_face =
		rs.front_face_ccw ? SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE : SDL_GPU_FRONTFACE_CLOCKWISE;
	info.rasterizer_state.enable_depth_clip = rs.depth_clip_enable;

	auto const& dss = state->GetDepthStencilStateDesc();
	info.depth_stencil_state.enable_depth_test = dss.depth_enable;
	info.depth_stencil_state.enable_depth_write = dss.depth_write_mask;
	info.depth_stencil_state.compare_op = SDL3Mapping::Mapping(dss.depth_func);
	info.depth_stencil_state.enable_stencil_test = dss.front_stencil_enable || dss.back_stencil_enable;

	auto const& bs = state->GetBlendStateDesc();
	std::vector<SDL_GPUColorTargetDescription> color_descs(key.num_colors);
	for (uint32_t i = 0; i < key.num_colors; ++i)
	{
		auto& color_desc = color_descs[i];
		color_desc.format = key.color_fmts[i];
		uint32_t const bi = (i < 8) ? i : 0;
		color_desc.blend_state.enable_blend = bs.blend_enable[bi];
		color_desc.blend_state.src_color_blendfactor = SDL3Mapping::Mapping(bs.src_blend[bi]);
		color_desc.blend_state.dst_color_blendfactor = SDL3Mapping::Mapping(bs.dest_blend[bi]);
		color_desc.blend_state.color_blend_op = SDL3Mapping::Mapping(bs.blend_op[bi]);
		color_desc.blend_state.src_alpha_blendfactor = SDL3Mapping::Mapping(bs.src_blend_alpha[bi]);
		color_desc.blend_state.dst_alpha_blendfactor = SDL3Mapping::Mapping(bs.dest_blend_alpha[bi]);
		color_desc.blend_state.alpha_blend_op = SDL3Mapping::Mapping(bs.blend_op_alpha[bi]);
		color_desc.blend_state.enable_color_write_mask = true;
		color_desc.blend_state.color_write_mask = static_cast<SDL_GPUColorComponentFlags>(bs.color_write_mask[bi]);
	}
	info.target_info.color_target_descriptions = color_descs.data();
	info.target_info.num_color_targets = key.num_colors;
	if (key.has_depth)
	{
		info.target_info.has_depth_stencil_target = true;
		info.target_info.depth_stencil_format = key.depth_fmt;
	}

	SDL_GPUGraphicsPipeline* pipe = SDL_CreateGPUGraphicsPipeline(device_, &info);
	if (!pipe)
	{
		LogError() << "[SDL3] SDL_CreateGPUGraphicsPipeline failed: " << SDL_GetError() << std::endl;
		return nullptr;
	}
	pipelines_.emplace(key, pipe);
	return pipe;
}

void SDL3RenderEngine::DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl)
{
	if (!cmd_)
	{
		return;
	}

		EnsureRenderPass(false, nullptr, false, 1.0f, false, 0);
		if (!render_pass_)
		{
			return;
		}
		// Re-assert viewport each draw: Clear() ends the pass, and a later
		// EnsureRenderPass must not keep a stale/zero scissor from UI.
		ApplyViewportAndScissor();

		auto const& sdl_rl = checked_cast<SDL3RenderLayout const&>(rl);
		sdl_rl.Active();

		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++i)
		{
			auto const& pass = tech.Pass(i);
			pass.Bind(effect);

			SDL_GPUGraphicsPipeline* pipe = GetOrCreatePipeline(effect, pass, rl);
			if (!pipe)
			{
				pass.Unbind(effect);
				continue;
			}

				// Upload/EndRenderPass during sampler bind can kill the pass; reopen.
				if (!render_pass_)
				{
					EnsureRenderPass(false, nullptr, false, 1.0f, false, 0);
					if (!render_pass_)
					{
						pass.Unbind(effect);
						continue;
					}
					ApplyViewportAndScissor();
				}

				SDL_BindGPUGraphicsPipeline(render_pass_, pipe);

			{
				auto so = pass.GetShaderObject(effect);
				for (auto stage : {ShaderStage::Vertex, ShaderStage::Pixel})
				{
					auto const& stage_obj = so->Stage(stage);
					if (!stage_obj || !cmd_)
					{
						continue;
					}
					auto const& sdl_stage = checked_cast<SDL3ShaderStageObject const&>(*stage_obj);
					auto const& indices = sdl_stage.CBufferIndices();
					auto const& sdesc = sdl_stage.GetShaderDesc();
					for (size_t ci = 0; ci < indices.size(); ++ci)
					{
						if (indices[ci] == 0xFF || ci >= sdesc.cb_desc.size())
						{
							continue;
						}
						auto* cb = effect.CBufferByIndex(indices[ci]);
						if (!cb || !cb->HWBuff())
						{
							continue;
						}
							cb->Update();
							auto const* hw = checked_cast<SDL3GraphicsBuffer const*>(cb->HWBuff().get());
							Uint32 const slot = sdesc.cb_desc[ci].bind_point;
							Uint32 const bytes = cb->Size();
							static int cb_log_left = 8;
							if (cb_log_left > 0 && stage == ShaderStage::Vertex && bytes >= 16)
							{
								--cb_log_left;
								auto const* f = reinterpret_cast<float const*>(hw->CpuData());
								LogInfo() << "[SDL3] PushVSUB slot=" << slot << " bytes=" << bytes
										  << " f0..3=[" << f[0] << "," << f[1] << "," << f[2] << "," << f[3] << "]"
										  << " cb=" << sdesc.cb_desc[ci].name << std::endl;
							}
							if (stage == ShaderStage::Pixel)
							{
								SDL_PushGPUFragmentUniformData(cmd_, slot, hw->CpuData(), bytes);
							}
							else
							{
								SDL_PushGPUVertexUniformData(cmd_, slot, hw->CpuData(), bytes);
							}
					}
				}

				// Bind real textures/samplers from effect reflection (fallback: default 1x1).
				auto const& ps_stage = so->Stage(ShaderStage::Pixel);
				if (ps_stage)
				{
					BindStageSamplers(effect, checked_cast<SDL3ShaderStageObject const&>(*ps_stage), true);
				}
					auto const& vs_stage = so->Stage(ShaderStage::Vertex);
					if (vs_stage)
					{
						BindStageSamplers(effect, checked_cast<SDL3ShaderStageObject const&>(*vs_stage), false);
					}
				}

				if (!render_pass_)
				{
					EnsureRenderPass(false, nullptr, false, 1.0f, false, 0);
					if (!render_pass_)
					{
						pass.Unbind(effect);
						continue;
					}
					ApplyViewportAndScissor();
					SDL_BindGPUGraphicsPipeline(render_pass_, pipe);
				}

				auto const& vbs = sdl_rl.VertexBuffers();
			if (!vbs.empty())
			{
				bool missing_vb = false;
				std::vector<SDL_GPUBufferBinding> bindings(vbs.size());
				for (size_t s = 0; s < vbs.size(); ++s)
				{
					bindings[s].buffer = vbs[s];
					bindings[s].offset = sdl_rl.VertexOffsets()[s];
					if (!vbs[s])
					{
						missing_vb = true;
					}
				}
				if (missing_vb)
				{
					LogError() << "[SDL3] Draw skipped: vertex buffer GPU resource is null "
								  "(delay-create before Active?)"
							   << std::endl;
					pass.Unbind(effect);
					continue;
				}
				SDL_BindGPUVertexBuffers(render_pass_, 0, bindings.data(), static_cast<Uint32>(bindings.size()));
			}

			if (rl.UseIndices() && sdl_rl.IndexBuffer())
			{
				SDL_GPUBufferBinding ib{};
				ib.buffer = sdl_rl.IndexBuffer();
				ib.offset = 0;
				SDL_BindGPUIndexBuffer(render_pass_, &ib, sdl_rl.IndexElementSize());
				SDL_DrawGPUIndexedPrimitives(render_pass_, rl.NumIndices(), std::max(1u, rl.NumInstances()),
					rl.StartIndexLocation(), static_cast<Sint32>(rl.StartVertexLocation()), rl.StartInstanceLocation());
			}
			else
			{
				SDL_DrawGPUPrimitives(render_pass_, rl.NumVertices(), std::max(1u, rl.NumInstances()),
					rl.StartVertexLocation(), rl.StartInstanceLocation());
			}

			pass.Unbind(effect);
		}
	}

	void SDL3RenderEngine::BindStageSamplers(RenderEffect const& effect, SDL3ShaderStageObject const& stage, bool fragment)
	{
		auto const& sdesc = stage.GetShaderDesc();
		Uint32 const n = sdesc.num_samplers;
		if (n == 0 || !render_pass_)
		{
			return;
		}

		EnsureDefaultSampleBinding();
		std::vector<SDL_GPUTextureSamplerBinding> binds(n);
		for (Uint32 s = 0; s < n; ++s)
		{
			binds[s].texture = default_sample_tex_;
			binds[s].sampler = default_sampler_;
		}

		SDL_GPUSampler* fallback_sampler = default_sampler_;
		for (auto const& res : sdesc.res_desc)
		{
			if (res.type != static_cast<uint8_t>(D3D_SIT_SAMPLER))
			{
				continue;
			}
			if (res.bind_point >= n)
			{
				continue;
			}
			auto* param = effect.ParameterByName(res.name);
			if (!param)
			{
				continue;
			}
			SamplerStateObjectPtr samp;
			param->Value(samp);
			if (samp)
			{
				auto* gpu = checked_cast<SDL3SamplerStateObject&>(*samp).GpuSampler();
				if (gpu)
				{
					binds[res.bind_point].sampler = gpu;
					if (!fallback_sampler || fallback_sampler == default_sampler_)
					{
						fallback_sampler = gpu;
					}
				}
			}
		}

		for (auto const& res : sdesc.res_desc)
		{
			if (res.type != static_cast<uint8_t>(D3D_SIT_TEXTURE))
			{
				continue;
			}
			if (res.bind_point >= n)
			{
				continue;
			}
			auto* param = effect.ParameterByName(res.name);
			if (!param)
			{
				continue;
			}
			ShaderResourceViewPtr srv;
			param->Value(srv);
			if (!srv || !srv->TextureResource())
			{
				continue;
			}
			auto* tex = dynamic_cast<SDL3Texture*>(srv->TextureResource().get());
			if (!tex || !tex->GpuTexture())
			{
				// Cube/3D still VirtualTexture in MVP — leave default slot.
				continue;
			}
			binds[res.bind_point].texture = tex->GpuTexture();
			if (!binds[res.bind_point].sampler)
			{
				binds[res.bind_point].sampler = fallback_sampler;
			}
		}

		// Fill any missing sampler with a real one so every slot is valid.
		for (Uint32 s = 0; s < n; ++s)
		{
			if (!binds[s].sampler)
			{
				binds[s].sampler = fallback_sampler ? fallback_sampler : default_sampler_;
			}
			if (!binds[s].texture)
			{
				binds[s].texture = default_sample_tex_;
			}
		}

		if (fragment)
		{
			SDL_BindGPUFragmentSamplers(render_pass_, 0, binds.data(), n);
		}
		else
		{
			SDL_BindGPUVertexSamplers(render_pass_, 0, binds.data(), n);
		}
	}

	void SDL3RenderEngine::EnsureDefaultSampleBinding()
	{
		if (!device_ || (default_sampler_ && default_sample_tex_))
		{
			return;
		}
		if (!default_sampler_)
		{
			SDL_GPUSamplerCreateInfo si{};
			si.min_filter = SDL_GPU_FILTER_LINEAR;
			si.mag_filter = SDL_GPU_FILTER_LINEAR;
			si.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
			si.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
			si.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
			si.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
			si.max_anisotropy = 1;
			si.enable_anisotropy = false;
			default_sampler_ = SDL_CreateGPUSampler(device_, &si);
		}
		if (!default_sample_tex_)
		{
			SDL_GPUTextureCreateInfo ti{};
			ti.type = SDL_GPU_TEXTURETYPE_2D;
			ti.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
			ti.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
			ti.width = 1;
			ti.height = 1;
			ti.layer_count_or_depth = 1;
			ti.num_levels = 1;
			ti.sample_count = SDL_GPU_SAMPLECOUNT_1;
			default_sample_tex_ = SDL_CreateGPUTexture(device_, &ti);
			if (default_sample_tex_)
			{
				// Upload opaque magenta so missing binds are obvious in RenderDoc.
				uint8_t const pixel[4] = {255, 0, 255, 255};
				SDL_GPUTransferBufferCreateInfo tb_info{};
				tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
				tb_info.size = sizeof(pixel);
				SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device_, &tb_info);
				if (transfer)
				{
					void* mapped = SDL_MapGPUTransferBuffer(device_, transfer, false);
					if (mapped)
					{
						std::memcpy(mapped, pixel, sizeof(pixel));
						SDL_UnmapGPUTransferBuffer(device_, transfer);
						SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device_);
						if (cmd)
						{
							SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
							SDL_GPUTextureTransferInfo src{};
							src.transfer_buffer = transfer;
							src.pixels_per_row = 1;
							src.rows_per_layer = 1;
							SDL_GPUTextureRegion dst{};
							dst.texture = default_sample_tex_;
							dst.w = 1;
							dst.h = 1;
							dst.d = 1;
							SDL_UploadToGPUTexture(copy, &src, &dst, false);
							SDL_EndGPUCopyPass(copy);
							SDL_SubmitGPUCommandBuffer(cmd);
						}
					}
					SDL_ReleaseGPUTransferBuffer(device_, transfer);
				}
			}
		}
	}

	void SDL3RenderEngine::DoBindFrameBuffer([[maybe_unused]] FrameBufferPtr const& fb)
{
	EndRenderPass();
	ResolvePassTargets();
}

void SDL3RenderEngine::DoBindSOBuffers([[maybe_unused]] const RenderLayoutPtr& rl)
{
}

void SDL3RenderEngine::DoDestroy()
{
	EndRenderPass();

	for (auto& kv : pipelines_)
	{
		if (kv.second && device_)
		{
			SDL_ReleaseGPUGraphicsPipeline(device_, kv.second);
		}
	}
	pipelines_.clear();

	if (device_)
	{
		if (default_sampler_)
		{
			SDL_ReleaseGPUSampler(device_, default_sampler_);
			default_sampler_ = nullptr;
		}
		if (default_sample_tex_)
		{
			SDL_ReleaseGPUTexture(device_, default_sample_tex_);
			default_sample_tex_ = nullptr;
		}
	}

	if (cmd_)
	{
		SDL_CancelGPUCommandBuffer(cmd_);
		cmd_ = nullptr;
	}
	swapchain_tex_ = nullptr;
	pass_colors_.clear();
	pass_depth_ = nullptr;
	pass_has_depth_ = false;

	if (device_ && window_)
	{
		SDL_ReleaseWindowFromGPUDevice(device_, window_);
	}
	window_ = nullptr;

	if (device_)
	{
		SDL_DestroyGPUDevice(device_);
		device_ = nullptr;
	}

	if (owns_sdl_init_)
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		owns_sdl_init_ = false;
	}
}

} // namespace RenderWorker
