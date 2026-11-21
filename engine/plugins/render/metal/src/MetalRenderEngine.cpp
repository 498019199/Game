#include "MetalRenderEngine.h"
#include "MetalRenderWindow.h"
#include "MetalRenderLayout.h"
#include "MetalShaderObject.h"
#include "MetalRenderStateObject.h"
#include "MetalGraphicsBuffer.h"
#include "MetalTexture.h"
#include <base/ZEngine.h>
#include <world/World.h>
#include <world/SceneNode.h>
#include <render/Renderable.h>
#include <render/RenderEffect.h>
#include <render/RenderFactory.h>
#include <render/RenderDeviceCaps.h>
#include <render/ElementFormat.h>
#include <render/FrameBuffer.h>
#include <render/RenderView.h>
#include <common/Log.h>
#include <math/color.h>
#include <math/math.h>
#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <vector>


namespace RenderWorker
{

MetalRenderEngine::MetalRenderEngine()
{
	native_shader_platform_name_ = "metal_spirv";
	native_shader_fourcc_ = MakeFourCC<'D', 'X', 'B', 'C'>::value;
	native_shader_version_ = 6;
}

MetalRenderEngine::~MetalRenderEngine()
{
	this->Destroy();
}

void MetalRenderEngine::Device(metal::Device* device, void* window)
{
	device_ = device;
	device_lifetime_->device = device;
	window_ = window;
	this->FillRenderDeviceCaps();
	EnsureDefaultSampleBinding();
}

void MetalRenderEngine::FillRenderDeviceCaps()
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
	// Include packed mesh formats used by model_bin / OBJ convert (Rabbit: pos SNORM16, uv SNORM16).
	std::vector<ElementFormat> vertex_formats = {
		EF_R32F,
		EF_GR32F,
		EF_BGR32F,
		EF_ABGR32F,
		EF_ABGR16F,
		EF_ABGR8,
		EF_ARGB8,
		EF_SIGNED_ABGR8,
		EF_R16F,
		EF_GR16F,
		EF_GR16,
		EF_SIGNED_GR16,
		EF_ABGR16,
		EF_SIGNED_ABGR16,
		EF_ABGR16UI,
		EF_ABGR8UI, // skinned models (ogre_magi blend indices)
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

void MetalRenderEngine::BeginFrame()
{
	RenderEngine::BeginFrame();
	EndRenderPass();
	if (!device_ || !window_)
	{
		return;
	}

	// If a previous frame aborted before EndFrame submit, the Metal drawable stays
	// checked out and the next WaitAndAcquire hangs forever in nextDrawable.
	if (cmd_)
	{
		if (swapchain_tex_)
		{
			metal::SubmitCommandBuffer(cmd_);
		}
		else
		{
			metal::CancelCommandBuffer(cmd_);
		}
		cmd_ = nullptr;
		swapchain_tex_ = nullptr;
	}

	cmd_ = metal::AcquireCommandBuffer(device_);
	if (!cmd_)
	{
		LogError() << "[Metal] AcquireGPUCommandBuffer failed: " << metal::LastError() << std::endl;
		return;
	}

	uint32_t sw = 0;
	uint32_t sh = 0;
	if (!metal::WaitAndAcquireSwapchainTexture(cmd_, window_, &swapchain_tex_, &sw, &sh))
	{
		LogError() << "[Metal] WaitAndAcquireGPUSwapchainTexture failed: " << metal::LastError() << std::endl;
		swapchain_tex_ = nullptr;
		metal::CancelCommandBuffer(cmd_);
		cmd_ = nullptr;
	}
}

void MetalRenderEngine::EndFrame()
{
	EndRenderPass();

	// Submit the scene and overlays as rendered; diagnostics must not clear the editor.

	if (cmd_)
	{
		if (swapchain_tex_)
		{
			metal::SubmitCommandBuffer(cmd_);
		}
		else
		{
			metal::CancelCommandBuffer(cmd_);
		}
		cmd_ = nullptr;
		swapchain_tex_ = nullptr;
	}
	RenderEngine::EndFrame();
}

void MetalRenderEngine::BeginOverlayPass()
{
	scissor_enabled_ = false;
	EnsureRenderPass(false, nullptr, false, 1.0f, false, 0);
}

bool MetalRenderEngine::CaptureCurrentColor(TexturePtr& snapshot)
{
	auto const& fb = this->CurFrameBuffer();
	if (!device_ || !cmd_ || !swapchain_tex_ || !fb || !fb->Viewport() ||
		!fb->Viewport()->Width() || !fb->Viewport()->Height()) return false;

	metal::TextureLocation source{};
	metal::TextureFormat native_format = metal::TEXTUREFORMAT_INVALID;
	uint32_t width = 0, height = 0;
	auto const& rtv = fb->AttachedRtv(FrameBuffer::Attachment::Color0);
	if (rtv)
	{
		auto const& texture = rtv->TextureResource();
		if (!texture || texture->Type() != Texture::TT_2D || texture->SampleCount() != 1 ||
			rtv->SampleCount() != 1 || rtv->ArraySize() != 1 || rtv->Level() >= texture->MipMapsNum() ||
			rtv->FirstArrayIndex() >= texture->ArraySize() || IsDepthFormat(texture->Format()) ||
			rtv->Format() != texture->Format()) return false;
		auto const& native_texture = checked_cast<MetalTexture const&>(*texture);
		source.texture = native_texture.GpuTexture();
		source.mip_level = rtv->Level();
		source.layer = rtv->FirstArrayIndex();
		native_format = native_texture.GpuFormat();
		width = texture->Width(source.mip_level);
		height = texture->Height(source.mip_level);
	}
	else
	{
		// Swapchain capture is unsupported. Scene/Game offscreen Color0
		// targets can be captured; a bare window requires an intermediate RT.
		return false;
	}
	if (!source.texture || !width || !height || native_format == metal::TEXTUREFORMAT_INVALID) return false;
	auto const format = MetalMapping::MappingFormat(native_format);
	if (MetalMapping::MappingFormat(format) != native_format || IsDepthFormat(format)) return false;

	auto copy = snapshot;
	if (!copy || !copy->HWResourceReady() || copy->Type() != Texture::TT_2D ||
		copy->Width(0) != width || copy->Height(0) != height || copy->Format() != format ||
		copy->ArraySize() != 1 || copy->MipMapsNum() != 1 || copy->SampleCount() != 1 ||
		!(copy->AccessHint() & EAH_GPU_Read) || checked_cast<MetalTexture const&>(*copy).GpuTexture() == source.texture)
	{
		copy = Context::Instance().RenderFactoryInstance().MakeTexture2D(width, height, 1, 1,
			format, 1, 0, EAH_GPU_Read);
	}
	if (!copy || !copy->HWResourceReady()) return false;
	auto& destination_texture = checked_cast<MetalTexture&>(*copy);
	if (destination_texture.GpuFormat() != native_format) return false;
	metal::TextureLocation destination{};
	destination.texture = destination_texture.GpuTexture();
	if (!destination.texture || destination.texture == source.texture) return false;

	// The copy shares this frame's command buffer, so earlier scene draws finish
	// before the snapshot is sampled. The next draw resumes the framebuffer with
	// LOAD for both color and depth; it never samples its own render target.
	this->EndRenderPass();
	auto* pass = metal::BeginCopyPass(cmd_);
	if (!pass) return false;
	metal::CopyTextureToTexture(pass, &source, &destination, width, height, 1, false);
	metal::EndCopyPass(pass);
	snapshot = std::move(copy);
	return true;
}

void MetalRenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	scissor_x_ = x;
	scissor_y_ = y;
	scissor_w_ = width;
	scissor_h_ = height;
	scissor_valid_ = true;
	if (render_pass_)
	{
		metal::Rect r{static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)};
		metal::SetScissor(render_pass_, &r);
	}
}

void MetalRenderEngine::ResolvePassTargets()
{
	pass_colors_.clear();
	pass_depth_ = nullptr;
	pass_depth_format_ = metal::TEXTUREFORMAT_INVALID;
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
		auto* gpu = checked_cast<MetalTexture&>(*tex).GpuTexture();
		if (!gpu)
		{
			LogError() << "[Metal] RTV texture has no GPU resource" << std::endl;
			break;
		}
		ColorTarget ct{};
		ct.texture = gpu;
		ct.format = MetalMapping::MappingFormat(rtv->Format());
		ct.mip_level = rtv->Level();
		ct.layer = rtv->FirstArrayIndex();
		pass_colors_.push_back(ct);
	}

	// Screen / default FB has depth but no color RTV — color is the swapchain.
	if (pass_colors_.empty() && swapchain_tex_ && device_ && window_)
	{
		ColorTarget ct{};
		ct.texture = swapchain_tex_;
		ct.format = metal::GetSwapchainTextureFormat(device_, window_);
		pass_colors_.push_back(ct);
	}

	if (auto const& dsv = fb->AttachedDsv())
	{
			if (auto const& tex = dsv->TextureResource())
			{
				auto& native_tex = checked_cast<MetalTexture&>(*tex);
				pass_depth_ = native_tex.GpuTexture();
				if (pass_depth_)
				{
					pass_depth_format_ = native_tex.GpuFormat();
					if (pass_depth_format_ == metal::TEXTUREFORMAT_INVALID)
					{
						pass_depth_format_ = MetalMapping::MappingFormat(dsv->Format());
					}
					pass_has_depth_ = true;
				}
			}
	}
}

	void MetalRenderEngine::ApplyViewportAndScissor()
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

		// Never overwrite the full-target default with a zero viewport — that
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

		metal::Viewport gpu_vp{};
		gpu_vp.x = vp_x;
		gpu_vp.y = vp_y;
		gpu_vp.w = vp_w;
		gpu_vp.h = vp_h;
		gpu_vp.min_depth = 0.0f;
		gpu_vp.max_depth = 1.0f;
		metal::SetViewport(render_pass_, &gpu_vp);

		// Only apply the saved rectangle to passes whose rasterizer enables scissoring.
		int sc_x = static_cast<int>(vp_x);
		int sc_y = static_cast<int>(vp_y);
		int sc_w = static_cast<int>(vp_w);
		int sc_h = static_cast<int>(vp_h);
		if (scissor_enabled_ && scissor_valid_)
		{
			sc_x = static_cast<int>(scissor_x_);
			sc_y = static_cast<int>(scissor_y_);
			sc_w = static_cast<int>(scissor_w_);
			sc_h = static_cast<int>(scissor_h_);
		}
		metal::Rect r{sc_x, sc_y, sc_w, sc_h};
		metal::SetScissor(render_pass_, &r);
	}

void MetalRenderEngine::EnsureRenderPass(bool clear_color, Color const* clear_clr, bool clear_depth, float depth,
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

	std::vector<metal::ColorTargetInfo> color_infos(pass_colors_.size());
	for (size_t i = 0; i < pass_colors_.size(); ++i)
	{
		auto& color_info = color_infos[i];
		color_info.texture = pass_colors_[i].texture;
		color_info.mip_level = pass_colors_[i].mip_level;
		color_info.layer_or_depth_plane = pass_colors_[i].layer;
		color_info.load_op = clear_color ? metal::LOADOP_CLEAR : metal::LOADOP_LOAD;
		color_info.store_op = metal::STOREOP_STORE;
		if (clear_clr)
		{
			color_info.clear_color.r = clear_clr->r();
			color_info.clear_color.g = clear_clr->g();
			color_info.clear_color.b = clear_clr->b();
			color_info.clear_color.a = clear_clr->a();
		}
	}

	metal::DepthStencilTargetInfo ds_info{};
	metal::DepthStencilTargetInfo* ds_ptr = nullptr;
	if (pass_has_depth_ && pass_depth_)
	{
		ds_info.texture = pass_depth_;
		ds_info.clear_depth = depth;
		ds_info.clear_stencil = static_cast<uint8_t>(stencil);
		ds_info.load_op = clear_depth ? metal::LOADOP_CLEAR : metal::LOADOP_LOAD;
		ds_info.store_op = metal::STOREOP_STORE;
		ds_info.stencil_load_op = clear_stencil ? metal::LOADOP_CLEAR : metal::LOADOP_LOAD;
		ds_info.stencil_store_op = metal::STOREOP_STORE;
		ds_ptr = &ds_info;
	}

	render_pass_ = metal::BeginRenderPass(cmd_, color_infos.data(), static_cast<uint32_t>(color_infos.size()), ds_ptr);
	if (!render_pass_)
	{
		LogError() << "[Metal] metal::BeginRenderPass failed: " << metal::LastError() << std::endl;
		return;
	}
	ApplyViewportAndScissor();
}

void MetalRenderEngine::EndRenderPass()
{
	if (render_pass_)
	{
		metal::EndRenderPass(render_pass_);
		render_pass_ = nullptr;
	}
}

void MetalRenderEngine::DoCreateRenderWindow(std::string const& name, RenderSettings const& settings)
{

	auto win = MakeSharedPtr<MetalRenderWindow>(name, settings);
	native_shader_platform_name_ = "metal_spirv";
	this->BindFrameBuffer(win);
}

metal::GraphicsPipeline* MetalRenderEngine::GetOrCreatePipeline(const RenderEffect& effect, const RenderPass& pass,
	const RenderLayout& rl)
{
	auto so = pass.GetShaderObject(effect);
	if (!so)
	{
		return nullptr;
	}
	auto& native_so = checked_cast<MetalShaderObject&>(*so);
	native_so.Bind(effect);

	auto* vs = native_so.VertexShader();
	auto* ps = native_so.PixelShader();
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

	auto const& native_rl = checked_cast<MetalRenderLayout const&>(rl);
		native_rl.Active();

	if (native_rl.VertexAttributes().empty() && (rl.VertexStreamNum() > 0))
	{
		LogError() << "[Metal] Refusing pipeline create: layout has streams but no vertex attributes"
					<< std::endl;
		return nullptr;
	}

	metal::GraphicsPipelineCreateInfo info{};
	info.vertex_shader = vs;
	info.fragment_shader = ps;
	info.vertex_input_state.vertex_buffer_descriptions = native_rl.VertexBufferDescs().data();
	info.vertex_input_state.num_vertex_buffers = static_cast<uint32_t>(native_rl.VertexBufferDescs().size());
	info.vertex_input_state.vertex_attributes = native_rl.VertexAttributes().data();
	info.vertex_input_state.num_vertex_attributes = static_cast<uint32_t>(native_rl.VertexAttributes().size());
	info.primitive_type = MetalMapping::Mapping(rl.TopologyType());

	auto const& rs = state->GetRasterizerStateDesc();
	info.rasterizer_state.fill_mode = MetalMapping::Mapping(rs.polygon_mode);
	info.rasterizer_state.cull_mode = MetalMapping::Mapping(rs.cull_mode);
	info.rasterizer_state.front_face =
		rs.front_face_ccw ? metal::FRONTFACE_COUNTER_CLOCKWISE : metal::FRONTFACE_CLOCKWISE;
	info.rasterizer_state.enable_depth_clip = rs.depth_clip_enable;

	auto const& dss = state->GetDepthStencilStateDesc();
	info.depth_stencil_state.enable_depth_test = dss.depth_enable;
	info.depth_stencil_state.enable_depth_write = dss.depth_write_mask;
	info.depth_stencil_state.compare_op = MetalMapping::Mapping(dss.depth_func);
	info.depth_stencil_state.enable_stencil_test = dss.front_stencil_enable || dss.back_stencil_enable;

	auto const& bs = state->GetBlendStateDesc();
	std::vector<metal::ColorTargetDescription> color_descs(key.num_colors);
	for (uint32_t i = 0; i < key.num_colors; ++i)
	{
		auto& color_desc = color_descs[i];
		color_desc.format = key.color_fmts[i];
		uint32_t const bi = (i < 8) ? i : 0;
		color_desc.blend_state.enable_blend = bs.blend_enable[bi];
		color_desc.blend_state.src_color_blendfactor = MetalMapping::Mapping(bs.src_blend[bi]);
		color_desc.blend_state.dst_color_blendfactor = MetalMapping::Mapping(bs.dest_blend[bi]);
		color_desc.blend_state.color_blend_op = MetalMapping::Mapping(bs.blend_op[bi]);
		color_desc.blend_state.src_alpha_blendfactor = MetalMapping::Mapping(bs.src_blend_alpha[bi]);
		color_desc.blend_state.dst_alpha_blendfactor = MetalMapping::Mapping(bs.dest_blend_alpha[bi]);
		color_desc.blend_state.alpha_blend_op = MetalMapping::Mapping(bs.blend_op_alpha[bi]);
		color_desc.blend_state.enable_color_write_mask = true;
		color_desc.blend_state.color_write_mask = static_cast<metal::ColorComponentFlags>(bs.color_write_mask[bi]);
	}
	info.target_info.color_target_descriptions = color_descs.data();
	info.target_info.num_color_targets = key.num_colors;
	if (key.has_depth)
	{
		info.target_info.has_depth_stencil_target = true;
		info.target_info.depth_stencil_format = key.depth_fmt;
	}

	metal::GraphicsPipeline* pipe = metal::CreateGraphicsPipeline(device_, &info);
	if (!pipe)
	{
		LogError() << "[Metal] metal::CreateGraphicsPipeline failed: " << metal::LastError() << std::endl;
		return nullptr;
	}
	pipelines_.emplace(key, pipe);
	return pipe;
}

void MetalRenderEngine::DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl)
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

		auto const& native_rl = checked_cast<MetalRenderLayout const&>(rl);
		native_rl.Active();

		uint32_t const num_passes = tech.NumPasses();
		for (uint32_t i = 0; i < num_passes; ++i)
		{
			auto const& pass = tech.Pass(i);
			pass.Bind(effect);
			scissor_enabled_ = pass.GetRenderStateObject()->GetRasterizerStateDesc().scissor_enable;

			metal::GraphicsPipeline* pipe = GetOrCreatePipeline(effect, pass, rl);
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

				metal::BindGraphicsPipeline(render_pass_, pipe);
				ApplyViewportAndScissor();

			{
				auto so = pass.GetShaderObject(effect);
				for (auto stage : {ShaderStage::Vertex, ShaderStage::Pixel})
				{
					auto const& stage_obj = so->Stage(stage);
					if (!stage_obj || !cmd_)
					{
						continue;
					}
					auto const& native_stage = checked_cast<MetalShaderStageObject const&>(*stage_obj);
					auto const& indices = native_stage.CBufferIndices();
					auto const& sdesc = native_stage.GetShaderDesc();
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
							auto const* hw = checked_cast<MetalGraphicsBuffer const*>(cb->HWBuff().get());
							uint32_t const slot = sdesc.cb_desc[ci].bind_point;
							uint32_t const bytes = cb->Size();
							if (stage == ShaderStage::Pixel)
							{
								metal::PushFragmentUniformData(cmd_, slot, hw->CpuData(), bytes);
							}
							else
							{
								metal::PushVertexUniformData(cmd_, slot, hw->CpuData(), bytes);
							}
					}
				}

				// Bind real textures/samplers from effect reflection (fallback: default 1x1).
				auto const& ps_stage = so->Stage(ShaderStage::Pixel);
				if (ps_stage)
				{
					BindStageSamplers(effect, checked_cast<MetalShaderStageObject const&>(*ps_stage), true);
				}
					auto const& vs_stage = so->Stage(ShaderStage::Vertex);
					if (vs_stage)
					{
						BindStageSamplers(effect, checked_cast<MetalShaderStageObject const&>(*vs_stage), false);
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
					metal::BindGraphicsPipeline(render_pass_, pipe);
					ApplyViewportAndScissor();
				}

				auto const& vbs = native_rl.VertexBuffers();
			if (!vbs.empty())
			{
				bool missing_vb = false;
				std::vector<metal::BufferBinding> bindings(vbs.size());
				for (size_t s = 0; s < vbs.size(); ++s)
				{
					bindings[s].buffer = vbs[s];
					bindings[s].offset = native_rl.VertexOffsets()[s];
					if (!vbs[s])
					{
						missing_vb = true;
					}
				}
				if (missing_vb)
				{
					LogError() << "[Metal] Draw skipped: vertex buffer GPU resource is null "
								  "(delay-create before Active?)"
							   << std::endl;
					pass.Unbind(effect);
					continue;
				}
				metal::BindVertexBuffers(render_pass_, 0, bindings.data(), static_cast<uint32_t>(bindings.size()));
			}

			if (rl.UseIndices() && native_rl.IndexBuffer())
			{
				metal::BufferBinding ib{};
				ib.buffer = native_rl.IndexBuffer();
				ib.offset = 0;
				metal::BindIndexBuffer(render_pass_, &ib, native_rl.IndexElementSize());
				metal::DrawIndexedPrimitives(render_pass_, rl.NumIndices(), std::max(1u, rl.NumInstances()),
					rl.StartIndexLocation(), static_cast<int32_t>(rl.StartVertexLocation()), rl.StartInstanceLocation());
			}
			else
			{
				metal::DrawPrimitives(render_pass_, rl.NumVertices(), std::max(1u, rl.NumInstances()),
					rl.StartVertexLocation(), rl.StartInstanceLocation());
			}

			pass.Unbind(effect);
		}
	}

	void MetalRenderEngine::BindStageSamplers(RenderEffect const& effect, MetalShaderStageObject const& stage, bool fragment)
	{
		auto const& sdesc = stage.GetShaderDesc();
		uint32_t const n = sdesc.num_samplers;
		if (n == 0 || !render_pass_)
		{
			return;
		}

		EnsureDefaultSampleBinding();
		std::vector<metal::TextureSamplerBinding> binds(n);
		for (uint32_t s = 0; s < n; ++s)
		{
			binds[s].texture = DefaultTexture(metal::TEXTURETYPE_2D);
			binds[s].sampler = default_sampler_;
		}

		metal::Sampler* fallback_sampler = default_sampler_;
		for (auto const& res : sdesc.res_desc)
		{
			if (res.type != MetalShaderDesc::ResourceKind::Sampler)
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
				auto* gpu = checked_cast<MetalSamplerStateObject&>(*samp).GpuSampler();
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
			if (res.type != MetalShaderDesc::ResourceKind::Texture)
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
            auto tex = srv ? dynamic_cast<MetalTexture*>(srv->TextureResource().get()) : nullptr;
            if (tex && tex->GpuTexture()) {
                binds[res.bind_point].texture = tex->GpuTexture();
            } else {
                metal::TextureType shape = metal::TEXTURETYPE_2D;
                switch (param->Type()) {
                case REDT_textureCUBE: shape=metal::TEXTURETYPE_CUBE; break;
                case REDT_textureCUBEArray: shape=metal::TEXTURETYPE_CUBE_ARRAY; break;
                case REDT_texture3D: shape=metal::TEXTURETYPE_3D; break;
                case REDT_texture2DArray: shape=metal::TEXTURETYPE_2D_ARRAY; break;
                default: break;
                }
                binds[res.bind_point].texture = DefaultTexture(shape);
            }
			if (!binds[res.bind_point].sampler)
			{
				binds[res.bind_point].sampler = fallback_sampler;
			}
		}

		// Fill any missing sampler with a real one so every slot is valid.
		for (uint32_t s = 0; s < n; ++s)
		{
			if (!binds[s].sampler)
			{
				binds[s].sampler = fallback_sampler ? fallback_sampler : default_sampler_;
			}
			if (!binds[s].texture)
			{
				binds[s].texture = DefaultTexture(metal::TEXTURETYPE_2D);
			}
		}

		if (fragment)
		{
			metal::BindFragmentSamplers(render_pass_, 0, binds.data(), n);
		}
		else
		{
			metal::BindVertexSamplers(render_pass_, 0, binds.data(), n);
		}
	}

	void MetalRenderEngine::EnsureDefaultSampleBinding()
	{
		if (!default_sampler_)
		{
			metal::SamplerCreateInfo si{};
			si.min_filter = metal::FILTER_LINEAR;
			si.mag_filter = metal::FILTER_LINEAR;
			si.mipmap_mode = metal::SAMPLERMIPMAPMODE_LINEAR;
			si.address_mode_u = metal::SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
			si.address_mode_v = metal::SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
			si.address_mode_w = metal::SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
			si.max_anisotropy = 1;
			si.enable_anisotropy = false;
			default_sampler_ = metal::CreateSampler(device_, &si);
		}
    }

metal::Texture* MetalRenderEngine::DefaultTexture(metal::TextureType type)
{
    auto& texture = default_textures_.at(type);
    if (texture) return texture;
    metal::TextureCreateInfo info{};
    info.type = type;
    info.format = metal::TEXTUREFORMAT_R8G8B8A8_UNORM;
    info.usage = metal::TEXTUREUSAGE_SAMPLER;
    info.width = info.height = info.num_levels = 1;
    info.layer_count_or_depth = type == metal::TEXTURETYPE_CUBE || type == metal::TEXTURETYPE_CUBE_ARRAY ? 6 : 1;
    texture = metal::CreateTexture(device_, &info);
    MetalCheck(texture, "Create default texture");
    metal::TransferBufferCreateInfo upload{metal::TRANSFERBUFFERUSAGE_UPLOAD, 4};
    auto* buffer = metal::CreateTransferBuffer(device_, &upload);
    MetalCheck(buffer, "Create default texture upload buffer");
    auto release = CommonWorker::Defer([&] { metal::ReleaseTransferBuffer(device_, buffer); });
    uint8_t const magenta[4] = {255, 0, 255, 255};
    auto mapped = metal::MapTransferBuffer(device_, buffer, false);
    MetalCheck(mapped, "Map default texture upload buffer");
    std::memcpy(mapped, magenta, sizeof(magenta));
    metal::UnmapTransferBuffer(device_, buffer);
    auto cmd = metal::AcquireCommandBuffer(device_);
    auto pass = metal::BeginCopyPass(cmd);
    metal::TextureTransferInfo src{buffer,0,1,1};
    metal::TextureRegion dst{};
    dst.texture = texture;
    dst.w = dst.h = dst.d = 1;
    for (dst.layer=0; dst.layer<info.layer_count_or_depth; ++dst.layer)
        metal::UploadToTexture(pass, &src, &dst, false);
    metal::EndCopyPass(pass);
    metal::SubmitCommandBuffer(cmd);
    return texture;
}

	void MetalRenderEngine::DoBindFrameBuffer([[maybe_unused]] FrameBufferPtr const& fb)
{
	EndRenderPass();
	ResolvePassTargets();
}

void MetalRenderEngine::DoBindSOBuffers([[maybe_unused]] const RenderLayoutPtr& rl)
{
}

void MetalRenderEngine::DoDestroy()
{
	EndRenderPass();

	for (auto& kv : pipelines_)
	{
		if (kv.second && device_)
		{
			metal::ReleaseGraphicsPipeline(device_, kv.second);
		}
	}
	pipelines_.clear();
		if (device_)
		{
		if (default_sampler_)
		{
			metal::ReleaseSampler(device_, default_sampler_);
			default_sampler_ = nullptr;
		}
        for (auto& texture : default_textures_) {
            if (texture) metal::ReleaseTexture(device_, texture);
            texture = nullptr;
        }
	}

	if (cmd_)
	{
		metal::CancelCommandBuffer(cmd_);
		cmd_ = nullptr;
	}
	swapchain_tex_ = nullptr;
	pass_colors_.clear();
	pass_depth_ = nullptr;
	pass_has_depth_ = false;
	window_ = nullptr;

	if (device_)
	{
		metal::Device* device = device_;
		device_lifetime_->device = nullptr;
		device_ = nullptr;
		metal::DestroyDevice(device);
	}
}

} // namespace RenderWorker
