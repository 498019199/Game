#include <base/ZEngine.h>
#include <render/RenderEffect.h>
#include <render/RenderFactory.h>

#include "D3D11RenderEngine.h"
#include "D3D11Util.h"
#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderLayout.h"
#include "D3D11RenderWindow.h"
#include "D3D11RenderStateObject.h"

#include <functional>

namespace RenderWorker
{
using namespace CommonWorker;

static const std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11ShaderResourceView * const *)> ShaderSetShaderResources[] =
{
	std::mem_fn(&ID3D11DeviceContext::VSSetShaderResources),
	std::mem_fn(&ID3D11DeviceContext::PSSetShaderResources),
	std::mem_fn(&ID3D11DeviceContext::GSSetShaderResources),
	std::mem_fn(&ID3D11DeviceContext::CSSetShaderResources),
	std::mem_fn(&ID3D11DeviceContext::HSSetShaderResources),
	std::mem_fn(&ID3D11DeviceContext::DSSetShaderResources)
};
static_assert(std::size(ShaderSetShaderResources) == ShaderStageNum);

static const std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11SamplerState * const *)>  ShaderSetSamplers[] =
{
	std::mem_fn(&ID3D11DeviceContext::VSSetSamplers),
	std::mem_fn(&ID3D11DeviceContext::PSSetSamplers),
	std::mem_fn(&ID3D11DeviceContext::GSSetSamplers),
	std::mem_fn(&ID3D11DeviceContext::CSSetSamplers),
	std::mem_fn(&ID3D11DeviceContext::HSSetSamplers),
	std::mem_fn(&ID3D11DeviceContext::DSSetSamplers)
};
static_assert(std::size(ShaderSetSamplers) == ShaderStageNum);

static const std::function<void(ID3D11DeviceContext*, UINT, UINT, ID3D11Buffer * const *)> ShaderSetConstantBuffers[] =
{
	std::mem_fn(&ID3D11DeviceContext::VSSetConstantBuffers),
	std::mem_fn(&ID3D11DeviceContext::PSSetConstantBuffers),
	std::mem_fn(&ID3D11DeviceContext::GSSetConstantBuffers),
	std::mem_fn(&ID3D11DeviceContext::CSSetConstantBuffers),
	std::mem_fn(&ID3D11DeviceContext::HSSetConstantBuffers),
	std::mem_fn(&ID3D11DeviceContext::DSSetConstantBuffers)
};
static_assert(std::size(ShaderSetConstantBuffers) == ShaderStageNum);

D3D11RenderEngine::D3D11RenderEngine()
{
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
	// Dynamic loading because these dlls can't be loaded on WinXP
	if (!mod_dxgi_.Load("dxgi.dll"))
	{
		//LogError() << "COULDN'T load dxgi.dll" << std::endl;
		Verify(false);
	}

	if (!mod_d3d11_.Load("d3d11.dll"))
	{
		//LogError() << "COULDN'T load d3d11.dll" << std::endl;
		Verify(false);
	}

	DynamicCreateDXGIFactory1_ = reinterpret_cast<CreateDXGIFactory1Func>(mod_dxgi_.GetProcAddress("CreateDXGIFactory1"));
	DynamicCreateDXGIFactory2_ = reinterpret_cast<CreateDXGIFactory2Func>(mod_dxgi_.GetProcAddress("CreateDXGIFactory2"));
	DynamicD3D11CreateDevice_ = reinterpret_cast<D3D11CreateDeviceFunc>(mod_d3d11_.GetProcAddress("D3D11CreateDevice"));
#else
	DynamicCreateDXGIFactory1_ = ::CreateDXGIFactory1;
	DynamicCreateDXGIFactory2_ = ::CreateDXGIFactory2;
	DynamicD3D11CreateDevice_ = ::D3D11CreateDevice;
#endif //ZENGINE_PLATFORM_WINDOWS_DESKTOP

	if (DynamicCreateDXGIFactory2_)
	{
		UINT const dxgi_factory_flags = 0;
		static UINT const available_dxgi_factory_flags[] =
		{
#if defined(ZENGINE_DEBUG)
			dxgi_factory_flags | DXGI_CREATE_FACTORY_DEBUG,
#endif
			dxgi_factory_flags
		};

		HRESULT hr = E_FAIL;
		for (auto const& flags : available_dxgi_factory_flags)
		{
			hr = DynamicCreateDXGIFactory2_(flags, UuidOf<IDXGIFactory2>(), gi_factory_2_.put_void());
			if (SUCCEEDED(hr))
			{
				break;
			}
		}
	}
	else
	{
		TIFHR(DynamicCreateDXGIFactory1_(UuidOf<IDXGIFactory2>(), gi_factory_2_.put_void()));
	}

	dxgi_sub_ver_ = 2;
	if (gi_factory_2_.try_as(gi_factory_3_))
	{
		dxgi_sub_ver_ = 3;
		if (gi_factory_2_.try_as(gi_factory_4_))
		{
			dxgi_sub_ver_ = 4;
			if (gi_factory_2_.try_as(gi_factory_5_))
			{
				dxgi_sub_ver_ = 5;
				if (gi_factory_2_.try_as(gi_factory_6_))
				{
					dxgi_sub_ver_ = 6;
				}
			}
		}
	}

	if (gi_factory_6_)
	{
		adapterList_.Enumerate(gi_factory_6_.get());
	}
	else
	{
		adapterList_.Enumerate(gi_factory_2_.get());
	}
}

D3D11RenderEngine::~D3D11RenderEngine()
{
	this->Destroy();
}

#if ZENGINE_IS_DEV_PLATFORM
void* D3D11RenderEngine::GetD3DDevice()
{
	return d3d_device_1_.get();
}

void* D3D11RenderEngine::GetD3DDeviceImmContext()
{
	return d3d_imm_ctx_1_.get();
}
#endif//ZENGINE_IS_DEV_PLATFORM


void D3D11RenderEngine::DoRender(const RenderEffect& effect, const RenderTechnique& tech, const RenderLayout& rl)
{
	uint32_t vertex_stream_num = rl.VertexStreamNum();

	const auto& d3d_rl = checked_cast<const D3D11RenderLayout&>(rl);
	d3d_rl.Active();

	// 绑定顶点
	const auto& vbs = d3d_rl.VBs();
	const auto& strides = d3d_rl.Strides();
	const auto& offsets = d3d_rl.Offsets();
	if(0 != vertex_stream_num)
	{
		if ((vb_cache_.size() != vertex_stream_num) || (vb_cache_ != vbs)
			|| (vb_stride_cache_ != strides) || (vb_offset_cache_ != offsets))
		{
			d3d_imm_ctx_1_->IASetVertexBuffers(0, vertex_stream_num, &vbs[0], &strides[0], &offsets[0]);
			vb_cache_ = vbs;
			vb_stride_cache_ = strides;
			vb_offset_cache_ = offsets;
		}

		auto layout = d3d_rl.InputLayout(effect.ShaderObjectByIndex(0).get());
		if (layout != input_layout_cache_)
		{
			d3d_imm_ctx_1_->IASetInputLayout(layout);
			input_layout_cache_ = layout;
		}
	}
	else
	{
		if (!vb_cache_.empty())
		{
			vb_cache_.assign(vb_cache_.size(), nullptr);
			vb_stride_cache_.assign(vb_stride_cache_.size(), 0);
			vb_offset_cache_.assign(vb_offset_cache_.size(), 0);
			d3d_imm_ctx_1_->IASetVertexBuffers(0, static_cast<UINT>(vb_cache_.size()),
				&vb_cache_[0], &vb_stride_cache_[0], &vb_offset_cache_[0]);
			vb_cache_.clear();
			vb_stride_cache_.clear();
			vb_offset_cache_.clear();
		}
		input_layout_cache_ = nullptr;
		d3d_imm_ctx_1_->IASetInputLayout(input_layout_cache_);
	}

    // 设置图元类型，设定输入布局
	uint32_t const vertex_count = static_cast<uint32_t>(rl.UseIndices() ? rl.IndicesNum() : rl.NumVertices());
	D3D11RenderLayout::topology_type tt = rl.TopologyType();
	if (topology_type_cache_ != tt)
	{
		d3d_imm_ctx_1_->IASetPrimitiveTopology(D3D11Mapping::Mapping(tt));
		topology_type_cache_ = tt;
	}

	uint32_t prim_count;
	switch (tt)
	{
	case D3D11RenderLayout::TT_PointList:
		prim_count = vertex_count;
		break;

	case D3D11RenderLayout::TT_LineList:
	case D3D11RenderLayout::TT_LineList_Adj:
		prim_count = vertex_count / 2;
		break;

	case D3D11RenderLayout::TT_LineStrip:
	case D3D11RenderLayout::TT_LineStrip_Adj:
		prim_count = vertex_count - 1;
		break;

	case D3D11RenderLayout::TT_TriangleList:
	case D3D11RenderLayout::TT_TriangleList_Adj:
		prim_count = vertex_count / 3;
		break;

	case D3D11RenderLayout::TT_TriangleStrip:
	case D3D11RenderLayout::TT_TriangleStrip_Adj:
		prim_count = vertex_count - 2;
		break;

	default:
		if ((tt >= D3D11RenderLayout::TT_1_Ctrl_Pt_PatchList)
			&& (tt <= D3D11RenderLayout::TT_32_Ctrl_Pt_PatchList))
		{
			prim_count = vertex_count / (tt - D3D11RenderLayout::TT_1_Ctrl_Pt_PatchList + 1);
		}
		else
		{
			ZENGINE_UNREACHABLE("Invalid topology type");
		}
		break;
	}
	num_primitives_just_rendered_ = prim_count;
	num_vertices_just_rendered_ = vertex_count;

    // 绑定索引资源
	if(rl.UseIndices())
	{
		ID3D11Buffer* d3dib = checked_cast<D3D11GraphicsBuffer&>(*rl.GetIndexStream()).D3DBuffer();
		if (ib_cache_ != d3dib)
		{
			ib_cache_ = d3dib;
			d3d_imm_ctx_1_->IASetIndexBuffer(d3dib, D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);
		}
	}
	else
	{
		if (ib_cache_)
		{
			d3d_imm_ctx_1_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
			ib_cache_ = nullptr;
		}
	}

	// 将更新好的常量缓冲区绑定到顶点着色器和像素着色器
	uint32_t const num_passes = tech.NumPasses();
	for (uint32_t i = 0; i < num_passes; ++ i)
	{
		auto& pass = tech.Pass(i);
		pass.Bind(effect);
		if(3 == num_vertices_just_rendered_)
			d3d_imm_ctx_1_->DrawIndexed(num_vertices_just_rendered_, 0, 0);
		else
			d3d_imm_ctx_1_->DrawAuto();
		pass.Unbind(effect);
	}
}

void D3D11RenderEngine::DoBindFrameBuffer([[maybe_unused]] FrameBufferPtr const & fb)
{
	COMMON_ASSERT(d3d_device_1_);
	COMMON_ASSERT(fb);
}

void D3D11RenderEngine::DoBindSOBuffers(const RenderLayoutPtr& rl)
{
	uint32_t num_buffs = rl ? rl->VertexStreamNum() : 0;
	if (num_buffs > 0)
	{
		std::vector<void*> so_src(num_buffs, nullptr);
		std::vector<ID3D11Buffer*> d3d11_buffs(num_buffs);
		std::vector<UINT> d3d11_buff_offsets(num_buffs, 0);
		for (uint32_t i = 0; i < num_buffs; ++ i)
		{
			auto& d3d11_buf = checked_cast<D3D11GraphicsBuffer&>(*rl->GetVertexStream(i));

			so_src[i] = &d3d11_buf;
			d3d11_buffs[i] = d3d11_buf.D3DBuffer();
		}

		for (uint32_t i = 0; i < num_buffs; ++ i)
		{
			if (so_src[i] != nullptr)
			{
				DetachSRV(so_src[i], 0, 1);
			}
		}

		d3d_imm_ctx_1_->SOSetTargets(static_cast<UINT>(num_buffs), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

		num_so_buffs_ = num_buffs;
	}
	else if (num_so_buffs_ > 0)
	{
		std::vector<ID3D11Buffer*> d3d11_buffs(num_so_buffs_, nullptr);
		std::vector<UINT> d3d11_buff_offsets(num_so_buffs_, 0);
		d3d_imm_ctx_1_->SOSetTargets(static_cast<UINT>(num_so_buffs_), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

		num_so_buffs_ = num_buffs;
	}
}

void D3D11RenderEngine::RSSetState(ID3D11RasterizerState* ras)
{
	if (rasterizer_state_cache_ != ras)
	{
		d3d_imm_ctx_1_->RSSetState(ras);
		rasterizer_state_cache_ = ras;
	}
}

void D3D11RenderEngine::OMSetBlendState(ID3D11BlendState* bs, const Color& blend_factor, uint32_t sample_mask)
{
	if ((blend_state_cache_ != bs) || (blend_factor_cache_ != blend_factor) || (sample_mask_cache_ != sample_mask))
	{
		d3d_imm_ctx_1_->OMSetBlendState(bs, &blend_factor.r(), sample_mask);
		blend_state_cache_ = bs;
		blend_factor_cache_ = blend_factor;
		sample_mask_cache_ = sample_mask;
	}
}

void D3D11RenderEngine::OMSetDepthStencilState(ID3D11DepthStencilState* ds, uint16_t stencil_ref)
{
	if ((depth_stencil_state_cache_ != ds) || (stencil_ref_cache_ != stencil_ref))
	{
		d3d_imm_ctx_1_->OMSetDepthStencilState(ds, stencil_ref);
		depth_stencil_state_cache_ = ds;
		stencil_ref_cache_ = stencil_ref;
	}
}

void D3D11RenderEngine::VSSetShader(ID3D11VertexShader* shader)
{
	if (vertex_shader_cache_ != shader)
	{
		d3d_imm_ctx_1_->VSSetShader(shader, nullptr, 0);
		vertex_shader_cache_ = shader;
	}
}

void D3D11RenderEngine::PSSetShader(ID3D11PixelShader* shader)
{
	if (pixel_shader_cache_ != shader)
	{
		d3d_imm_ctx_1_->PSSetShader(shader, nullptr, 0);
		pixel_shader_cache_ = shader;
	}
}

void D3D11RenderEngine::GSSetShader(ID3D11GeometryShader* shader)
{
	if (geometry_shader_cache_ != shader)
	{
		d3d_imm_ctx_1_->GSSetShader(shader, nullptr, 0);
		geometry_shader_cache_ = shader;
	}
}

void D3D11RenderEngine::SetShaderResources(ShaderStage stage, 
	std::span<std::tuple<void*, uint32_t, uint32_t> const> srvsrcs, std::span<ID3D11ShaderResourceView* const> srvs)
{
	uint32_t const stage_index = std::to_underlying(stage);
	if (MakeSpan(shader_srv_ptr_cache_[stage_index]) != srvs)
	{
		size_t const old_size = shader_srv_ptr_cache_[stage_index].size();
		shader_srv_ptr_cache_[stage_index].assign(srvs.begin(), srvs.end());
		if (old_size > static_cast<size_t>(srvs.size()))
		{
			shader_srv_ptr_cache_[stage_index].resize(old_size, nullptr);
		}

		ShaderSetShaderResources[stage_index](d3d_imm_ctx_1_.get(), 0,
			static_cast<UINT>(shader_srv_ptr_cache_[stage_index].size()), &shader_srv_ptr_cache_[stage_index][0]);

		shader_srvsrc_cache_[stage_index].assign(srvsrcs.begin(), srvsrcs.end());
		shader_srv_ptr_cache_[stage_index].resize(srvs.size());
	}
}

void D3D11RenderEngine::SetSamplers(ShaderStage stage, std::span<ID3D11SamplerState* const> samplers)
{
	uint32_t const stage_index = std::to_underlying(stage);
	if (MakeSpan(shader_sampler_ptr_cache_[stage_index]) != samplers)
	{
		ShaderSetSamplers[stage_index](d3d_imm_ctx_1_.get(), 0, static_cast<UINT>(samplers.size()), &(samplers[0]));

		shader_sampler_ptr_cache_[stage_index].assign(samplers.begin(), samplers.end());
	}
}

void D3D11RenderEngine::SetConstantBuffers(ShaderStage stage, std::span<ID3D11Buffer* const> cbs)
{
	uint32_t const stage_index = std::to_underlying(stage);
	if (MakeSpan(shader_cb_ptr_cache_[stage_index]) != cbs)
	{
		ShaderSetConstantBuffers[stage_index](d3d_imm_ctx_1_.get(), 0, static_cast<UINT>(cbs.size()), &(cbs[0]));

		shader_cb_ptr_cache_[stage_index].assign(cbs.begin(), cbs.end());
	}
}

void D3D11RenderEngine::RSSetViewports(UINT NumViewports, D3D11_VIEWPORT const * pViewports)
{
	if (NumViewports > 1)
	{
		d3d_imm_ctx_1_->RSSetViewports(NumViewports, pViewports);
	}
	else
	{
		if (!(MathWorker::equal(pViewports->TopLeftX, viewport_cache_.TopLeftX)
			&& MathWorker::equal(pViewports->TopLeftY, viewport_cache_.TopLeftY)
			&& MathWorker::equal(pViewports->Width, viewport_cache_.Width)
			&& MathWorker::equal(pViewports->Height, viewport_cache_.Height)
			&& MathWorker::equal(pViewports->MinDepth, viewport_cache_.MinDepth)
			&& MathWorker::equal(pViewports->MaxDepth, viewport_cache_.MaxDepth)))
		{
			viewport_cache_ = *pViewports;
			d3d_imm_ctx_1_->RSSetViewports(NumViewports, pViewports);
		}
	}
}

void D3D11RenderEngine::DetachSRV(void* rtv_src, uint32_t rt_first_subres, uint32_t rt_num_subres)
{
	for (uint32_t stage = 0; stage < ShaderStageNum; ++stage)
	{
		bool cleared = false;
		for (uint32_t i = 0; i < shader_srvsrc_cache_[stage].size(); ++ i)
		{
			if (std::get<0>(shader_srvsrc_cache_[stage][i]))
			{
				if (std::get<0>(shader_srvsrc_cache_[stage][i]) == rtv_src)
				{
					uint32_t const first = std::get<1>(shader_srvsrc_cache_[stage][i]);
					uint32_t const last = first + std::get<2>(shader_srvsrc_cache_[stage][i]);
					uint32_t const rt_first = rt_first_subres;
					uint32_t const rt_last = rt_first_subres + rt_num_subres;
					if (((first >= rt_first) && (first < rt_last))
						|| ((last >= rt_first) && (last < rt_last))
						|| ((rt_first >= first) && (rt_first < last))
						|| ((rt_last >= first) && (rt_last < last)))
					{
						shader_srv_ptr_cache_[stage][i] = nullptr;
						cleared = true;
					}
				}
			}
		}

		if (cleared)
		{
			ShaderSetShaderResources[stage](
				d3d_imm_ctx_1_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[stage].size()), &shader_srv_ptr_cache_[stage][0]);
		}
	}
}

void D3D11RenderEngine::OMSetRenderTargets(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs, ID3D11DepthStencilView* dsv)
{
	if ((rtv_ptr_cache_.size() != num_rtvs) || (dsv_ptr_cache_ != dsv)
		|| (memcmp(&rtv_ptr_cache_[0], rtvs, num_rtvs * sizeof(rtvs[0])) != 0))
	{
		d3d_imm_ctx_1_->OMSetRenderTargets(num_rtvs, rtvs, dsv);

		rtv_ptr_cache_.assign(rtvs, rtvs + num_rtvs);
		dsv_ptr_cache_ = dsv;
	}
}

void D3D11RenderEngine::OMSetRenderTargetsAndUnorderedAccessViews(UINT num_rtvs, ID3D11RenderTargetView* const * rtvs,
	ID3D11DepthStencilView* dsv,
	UINT uav_start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs, UINT const * uav_init_counts)
{
	if ((rtv_ptr_cache_.size() != num_rtvs) || (dsv_ptr_cache_ != dsv)
		|| (render_uav_ptr_cache_.size() < uav_start_slot + num_uavs)
		|| (memcmp(&rtv_ptr_cache_[0], rtvs, num_rtvs * sizeof(rtvs[0])) != 0)
		|| (memcmp(&render_uav_ptr_cache_[uav_start_slot], uavs, num_uavs * sizeof(uavs[0])) != 0)
		|| (memcmp(&render_uav_init_count_cache_[uav_start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0])) != 0))
	{
		d3d_imm_ctx_1_->OMSetRenderTargetsAndUnorderedAccessViews(num_rtvs, rtvs, dsv,
			uav_start_slot, num_uavs, uavs, uav_init_counts);

		rtv_ptr_cache_.assign(rtvs, rtvs + num_rtvs);
		dsv_ptr_cache_ = dsv;

		if (render_uav_ptr_cache_.size() < uav_start_slot + num_uavs)
		{
			render_uav_ptr_cache_.resize(uav_start_slot + num_uavs);
			render_uav_init_count_cache_.resize(render_uav_ptr_cache_.size());
		}
		memcpy(&render_uav_ptr_cache_[uav_start_slot], uavs, num_uavs * sizeof(uavs[0]));
		memcpy(&render_uav_init_count_cache_[uav_start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0]));
	}
}

void D3D11RenderEngine::CSSetUnorderedAccessViews(UINT start_slot, UINT num_uavs, ID3D11UnorderedAccessView* const * uavs,
	UINT const * uav_init_counts)
{
	if ((compute_uav_ptr_cache_.size() < start_slot + num_uavs)
		|| (memcmp(&compute_uav_ptr_cache_[start_slot], uavs, num_uavs * sizeof(uavs[0])) != 0)
		|| (memcmp(&compute_uav_init_count_cache_[start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0])) != 0))
	{
		d3d_imm_ctx_1_->CSSetUnorderedAccessViews(start_slot, num_uavs, uavs, uav_init_counts);

		if (compute_uav_ptr_cache_.size() < start_slot + num_uavs)
		{
			compute_uav_ptr_cache_.resize(start_slot + num_uavs);
			compute_uav_init_count_cache_.resize(compute_uav_ptr_cache_.size());
		}
		memcpy(&compute_uav_ptr_cache_[start_slot], uavs, num_uavs * sizeof(uavs[0]));
		memcpy(&compute_uav_init_count_cache_[start_slot], uav_init_counts, num_uavs * sizeof(uav_init_counts[0]));
	}
}

void D3D11RenderEngine::BeginFrame()
{
	RenderEngine::BeginFrame();
}

void D3D11RenderEngine::EndFrame()
{
	RenderEngine::EndFrame();
}

IDXGIFactory2* D3D11RenderEngine::DXGIFactory2() const noexcept
{
	return gi_factory_2_.get();
}

IDXGIFactory3* D3D11RenderEngine::DXGIFactory3() const noexcept
{
	return gi_factory_3_.get();
}

IDXGIFactory4* D3D11RenderEngine::DXGIFactory4() const noexcept
{
	return gi_factory_4_.get();
}

IDXGIFactory5* D3D11RenderEngine::DXGIFactory5() const noexcept
{
	return gi_factory_5_.get();
}

IDXGIFactory6* D3D11RenderEngine::DXGIFactory6() const noexcept
{
	return gi_factory_6_.get();
}

uint8_t D3D11RenderEngine::DXGISubVer() const noexcept
{
	return dxgi_sub_ver_;
}

ID3D11Device1* D3D11RenderEngine::D3DDevice1() const noexcept
{
	return d3d_device_1_.get();
}

ID3D11Device2* D3D11RenderEngine::D3DDevice2() const noexcept
{
	return d3d_device_2_.get();
}

ID3D11Device3* D3D11RenderEngine::D3DDevice3() const noexcept
{
	return d3d_device_3_.get();
}

ID3D11Device4* D3D11RenderEngine::D3DDevice4() const noexcept
{
	return d3d_device_4_.get();
}

ID3D11Device5* D3D11RenderEngine::D3DDevice5() const noexcept
{
	return d3d_device_5_.get();
}

ID3D11DeviceContext1* D3D11RenderEngine::D3DDeviceImmContext1() const noexcept
{
	return d3d_imm_ctx_1_.get();
}

ID3D11DeviceContext2* D3D11RenderEngine::D3DDeviceImmContext2() const noexcept
{
	return d3d_imm_ctx_2_.get();
}

ID3D11DeviceContext3* D3D11RenderEngine::D3DDeviceImmContext3() const noexcept
{
	return d3d_imm_ctx_3_.get();
}

ID3D11DeviceContext4* D3D11RenderEngine::D3DDeviceImmContext4() const noexcept
{
	return d3d_imm_ctx_4_.get();
}

uint8_t D3D11RenderEngine::D3D11RuntimeSubVer() const noexcept
{
	return d3d_11_runtime_sub_ver_;
}

D3D_FEATURE_LEVEL D3D11RenderEngine::DeviceFeatureLevel() const noexcept
{
	return d3d_feature_level_;
}

const D3D11AdapterList& D3D11RenderEngine::D3DAdapters() const noexcept
{
	return adapterList_;
}

D3D11Adapter& D3D11RenderEngine::ActiveAdapter() const
{
	return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
}

void D3D11RenderEngine::DetectD3D11Runtime(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx)
{
	d3d_device_1_.reset(device);
	d3d_imm_ctx_1_.reset(imm_ctx);
	d3d_11_runtime_sub_ver_ = 1;

	if (d3d_device_1_.try_as(d3d_device_2_) && d3d_imm_ctx_1_.try_as(d3d_imm_ctx_2_))
	{
		d3d_11_runtime_sub_ver_ = 2;
		if (d3d_device_1_.try_as(d3d_device_3_) && d3d_imm_ctx_1_.try_as(d3d_imm_ctx_3_))
		{
			d3d_11_runtime_sub_ver_ = 3;
			if (d3d_device_1_.try_as(d3d_device_4_))
			{
				d3d_11_runtime_sub_ver_ = 4;
					
				d3d_device_1_.try_as(d3d_device_5_);
				d3d_imm_ctx_1_.try_as(d3d_imm_ctx_4_);
			}
		}
	}
}

void D3D11RenderEngine::D3DDevice(ID3D11Device1* device, ID3D11DeviceContext1* imm_ctx, D3D_FEATURE_LEVEL feature_level)
{
	this->DetectD3D11Runtime(device, imm_ctx);

	d3d_feature_level_ = feature_level;
	Verify(device != nullptr);

	if (d3d_11_runtime_sub_ver_ >= 4)
	{
		// 创建一个手动重置的事件对象（Event Object）
		device_lost_event_ = MakeWin32UniqueHandle(::CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS));
		if (device_lost_event_ != nullptr)
		{
			// 创建一个等待对象（Wait Object）
			thread_pool_wait_ = MakeWin32UniquTpWait(::CreateThreadpoolWait(D3D11RenderEngine::OnDeviceLost, this, nullptr));
			if (thread_pool_wait_ != nullptr)
			{
				::SetThreadpoolWait(thread_pool_wait_.get(), device_lost_event_.get(), nullptr);
				TIFHR(d3d_device_4_->RegisterDeviceRemovedEvent(device_lost_event_.get(), &device_lost_reg_cookie_));
			}
		}
	}
}

void D3D11RenderEngine::DoCreateRenderWindow(std::string const & name, RenderSettings const & settings)
{
	D3D11RenderWindowPtr win = MakeSharedPtr<D3D11RenderWindow>(&this->ActiveAdapter(),
			name, settings);
	
	switch (d3d_feature_level_)
	{
	case D3D_FEATURE_LEVEL_12_1:
	case D3D_FEATURE_LEVEL_12_0:
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
		shader_profiles_[std::to_underlying(ShaderStage::Vertex)] = "vs_5_0";
		shader_profiles_[std::to_underlying(ShaderStage::Pixel)] = "ps_5_0";
		shader_profiles_[std::to_underlying(ShaderStage::Geometry)] = "gs_5_0";
		shader_profiles_[std::to_underlying(ShaderStage::Compute)] = "cs_5_0";
		shader_profiles_[std::to_underlying(ShaderStage::Hull)] = "hs_5_0";
		shader_profiles_[std::to_underlying(ShaderStage::Domain)] = "ds_5_0";
		break;

	default:
		ZENGINE_UNREACHABLE("Invalid feature level");
	}

	this->ResetRenderStates();
	this->BindFrameBuffer(win);
}

void D3D11RenderEngine::ResetRenderStates()
{
	vertex_shader_cache_ = nullptr;
	pixel_shader_cache_ = nullptr;
	geometry_shader_cache_ = nullptr;
	// compute_shader_cache_ = nullptr;
	// hull_shader_cache_ = nullptr;
	// domain_shader_cache_ = nullptr;

	auto& rf = Context::Instance().RenderFactoryInstance();
	cur_rs_obj_ = rf.MakeRenderStateObject(RasterizerStateDesc(), DepthStencilStateDesc(), BlendStateDesc());
	auto& d3d_cur_rs_obj = checked_cast<D3D11RenderStateObject&>(*cur_rs_obj_);
	rasterizer_state_cache_ = d3d_cur_rs_obj.D3DRasterizerState();
	depth_stencil_state_cache_ = d3d_cur_rs_obj.D3DDepthStencilState();
	stencil_ref_cache_ = 0;
	blend_state_cache_ = d3d_cur_rs_obj.D3DBlendState();
	blend_factor_cache_ = Color(1, 1, 1, 1);
	sample_mask_cache_ = 0xFFFFFFFF;

	d3d_imm_ctx_1_->RSSetState(rasterizer_state_cache_);
	d3d_imm_ctx_1_->OMSetDepthStencilState(depth_stencil_state_cache_, stencil_ref_cache_);
	d3d_imm_ctx_1_->OMSetBlendState(blend_state_cache_, &blend_factor_cache_.r(), sample_mask_cache_);

	topology_type_cache_ = RenderLayout::TT_PointList;
	d3d_imm_ctx_1_->IASetPrimitiveTopology(D3D11Mapping::Mapping(topology_type_cache_));

	input_layout_cache_ = nullptr;
	d3d_imm_ctx_1_->IASetInputLayout(input_layout_cache_);

	viewport_cache_ = {};

	if (!vb_cache_.empty())
	{
		vb_cache_.assign(vb_cache_.size(), nullptr);
		vb_stride_cache_.assign(vb_stride_cache_.size(), 0);
		vb_offset_cache_.assign(vb_offset_cache_.size(), 0);
		d3d_imm_ctx_1_->IASetVertexBuffers(0, static_cast<UINT>(vb_cache_.size()),
			&vb_cache_[0], &vb_stride_cache_[0], &vb_offset_cache_[0]);
		vb_cache_.clear();
		vb_stride_cache_.clear();
		vb_offset_cache_.clear();
	}

	ib_cache_ = nullptr;
	d3d_imm_ctx_1_->IASetIndexBuffer(ib_cache_, DXGI_FORMAT_R16_UINT, 0);

	for (uint32_t i = 0; i < ShaderStageNum; ++i)
	{
		if (!shader_srv_ptr_cache_[i].empty())
		{
			std::fill(shader_srv_ptr_cache_[i].begin(), shader_srv_ptr_cache_[i].end(), static_cast<ID3D11ShaderResourceView*>(nullptr));
			ShaderSetShaderResources[i](d3d_imm_ctx_1_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[i].size()), &shader_srv_ptr_cache_[i][0]);
			shader_srvsrc_cache_[i].clear();
			shader_srv_ptr_cache_[i].clear();
		}

		if (!shader_sampler_ptr_cache_[i].empty())
		{
			std::fill(shader_sampler_ptr_cache_[i].begin(), shader_sampler_ptr_cache_[i].end(), static_cast<ID3D11SamplerState*>(nullptr));
			ShaderSetSamplers[i](d3d_imm_ctx_1_.get(), 0, static_cast<UINT>(shader_sampler_ptr_cache_[i].size()), &shader_sampler_ptr_cache_[i][0]);
			shader_sampler_ptr_cache_[i].clear();
		}

		if (!shader_cb_ptr_cache_[i].empty())
		{
			std::fill(shader_cb_ptr_cache_[i].begin(), shader_cb_ptr_cache_[i].end(), static_cast<ID3D11Buffer*>(nullptr));
			ShaderSetConstantBuffers[i](d3d_imm_ctx_1_.get(), 0, static_cast<UINT>(shader_cb_ptr_cache_[i].size()), &shader_cb_ptr_cache_[i][0]);
			shader_cb_ptr_cache_[i].clear();
		}
	}
}

void D3D11RenderEngine::InvalidRTVCache()
{
	rtv_ptr_cache_.clear();
}

void D3D11RenderEngine::DoDestroy()
{
	if ((device_lost_reg_cookie_ != 0) && d3d_device_4_)
	{
		d3d_device_4_->UnregisterDeviceRemoved(device_lost_reg_cookie_);
		device_lost_reg_cookie_ = 0;
	}
	device_lost_event_.reset();
	thread_pool_wait_.reset();

	adapterList_.Destroy();

	rasterizer_state_cache_ = nullptr;
	depth_stencil_state_cache_ = nullptr;
	blend_state_cache_ = nullptr;
	vertex_shader_cache_ = nullptr;
	pixel_shader_cache_ = nullptr;
	geometry_shader_cache_ = nullptr;
	// compute_shader_cache_ = nullptr;
	// hull_shader_cache_ = nullptr;
	// domain_shader_cache_ = nullptr;
	input_layout_cache_ = nullptr;
	vb_cache_.clear();
	ib_cache_ = nullptr;

	for (size_t i = 0; i < ShaderStageNum; ++ i)
	{
		shader_srvsrc_cache_[i].clear();
		shader_srv_ptr_cache_[i].clear();
		shader_sampler_ptr_cache_[i].clear();
		shader_cb_ptr_cache_[i].clear();
	}

	render_uav_ptr_cache_.clear();
	render_uav_init_count_cache_.clear();
	compute_uav_ptr_cache_.clear();
	compute_uav_init_count_cache_.clear();
	rtv_ptr_cache_.clear();
	dsv_ptr_cache_ = nullptr;

	if (d3d_imm_ctx_1_)
	{
		d3d_imm_ctx_1_->ClearState();
		d3d_imm_ctx_1_->Flush();
	}

	d3d_imm_ctx_1_.reset();
	d3d_imm_ctx_2_.reset();
	d3d_imm_ctx_3_.reset();
	d3d_imm_ctx_4_.reset();
	d3d_device_1_.reset();
	d3d_device_2_.reset();
	d3d_device_3_.reset();
	d3d_device_4_.reset();
	d3d_device_5_.reset();
	gi_factory_2_.reset();
	gi_factory_3_.reset();
	gi_factory_4_.reset();
	gi_factory_5_.reset();
	gi_factory_6_.reset();

	DynamicCreateDXGIFactory1_ = nullptr;
	DynamicCreateDXGIFactory2_ = nullptr;
	DynamicD3D11CreateDevice_ = nullptr;

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
	mod_d3d11_.Free();
	mod_dxgi_.Free();
#endif

}

void D3D11RenderEngine::FillRenderDeviceCaps()
{
	switch (d3d_feature_level_)
	{
	case D3D_FEATURE_LEVEL_12_1:
	case D3D_FEATURE_LEVEL_12_0:
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
		// D3D11 feature level 12.1+ supports objects in shader model 5.1, although it doesn't support shader model 5.1 bytecode
		// caps_.max_shader_model
		// 	= (d3d_feature_level_ > D3D_FEATURE_LEVEL_12_0) ? ShaderModel(5, 1) : ShaderModel(5, 0);
		caps_.max_texture_width = caps_.max_texture_height = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
		caps_.max_texture_depth = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
		caps_.max_texture_cube_size = D3D11_REQ_TEXTURECUBE_DIMENSION;
		caps_.max_texture_array_length = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
		caps_.max_vertex_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
		caps_.max_pixel_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
		caps_.max_geometry_texture_units = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
		caps_.max_simultaneous_rts = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
		caps_.max_simultaneous_uavs = D3D11_PS_CS_UAV_REGISTER_COUNT;
		caps_.cs_support = true;
		//caps_.tess_method = TM_Hardware;
		caps_.max_vertex_streams = D3D11_STANDARD_VERTEX_ELEMENT_COUNT;
		caps_.max_texture_anisotropy = D3D11_MAX_MAXANISOTROPY;
	default:
		ZENGINE_UNREACHABLE("Invalid feature level");
	}

	caps_.gs_support = true;
	caps_.hs_support = true;
	caps_.ds_support = true;
}

char const * D3D11RenderEngine::DefaultShaderProfile(ShaderStage stage) const
{
	return shader_profiles_[std::to_underlying(stage)];
}

HRESULT D3D11RenderEngine::D3D11CreateDevice(IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags,
	D3D_FEATURE_LEVEL const * pFeatureLevels, UINT FeatureLevels, UINT SDKVersion,
	ID3D11Device** ppDevice, D3D_FEATURE_LEVEL* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext) const
{
	return DynamicD3D11CreateDevice_(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion,
		ppDevice, pFeatureLevel, ppImmediateContext);
}

void D3D11RenderEngine::OnDeviceLost([[maybe_unused]] PTP_CALLBACK_INSTANCE instance, [[maybe_unused]] PVOID context,
	[[maybe_unused]] PTP_WAIT wait, [[maybe_unused]] TP_WAIT_RESULT wait_result) noexcept
{
	// TODO
}
}