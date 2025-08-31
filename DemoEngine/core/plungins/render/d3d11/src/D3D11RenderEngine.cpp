#include <base/Context.h>
#include <base/WinApp.h>
#include <render/RenderEffect.h>

#include "D3D11RenderEngine.h"
#include "D3D11Util.h"
#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderLayout.h"
#include "D3D11RenderWindow.h"
#include <functional>

namespace RenderWorker
{
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
#if defined(DEBUG) || defined(_DEBUG)  
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
	render_target_view_.reset();
	depth_stencil_view_.reset();
	depth_stencil_buff_.reset();

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

#if ZENGINE_IS_DEV_PLATFORM
void* D3D11RenderEngine::GetD3DDevice()
{
	return d3d_device_.get();
}

void* D3D11RenderEngine::GetD3DDeviceImmContext()
{
	return d3d_imm_ctx_1_.get();
}
#endif//ZENGINE_IS_DEV_PLATFORM

void D3D11RenderEngine::BeginRender() const 
{
	COMMON_ASSERT(d3d_imm_ctx_1_);
	COMMON_ASSERT(swap_chain_);

	Color blackColor(0.0, 0.0, 0.0f, 1.0f);
	d3d_imm_ctx_1_->ClearRenderTargetView(render_target_view_.get(), &blackColor.r());
	d3d_imm_ctx_1_->ClearDepthStencilView(depth_stencil_view_.get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void D3D11RenderEngine::EndRender() const
{

}

void D3D11RenderEngine::SwitchChain() const
{
    TIFHR(swap_chain_->Present(0, 0));
}

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
	return d3d_device_1_.get();
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

		d3d_imm_ctx_->SOSetTargets(static_cast<UINT>(num_buffs), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

		num_so_buffs_ = num_buffs;
	}
	else if (num_so_buffs_ > 0)
	{
		std::vector<ID3D11Buffer*> d3d11_buffs(num_so_buffs_, nullptr);
		std::vector<UINT> d3d11_buff_offsets(num_so_buffs_, 0);
		d3d_imm_ctx_->SOSetTargets(static_cast<UINT>(num_so_buffs_), &d3d11_buffs[0], &d3d11_buff_offsets[0]);

		num_so_buffs_ = num_buffs;
	}
}

void D3D11RenderEngine::FillRenderDeviceCaps()
{
	COMMON_ASSERT(d3d_device_);

	switch (d3d_feature_level_)
	{
	case D3D_FEATURE_LEVEL_12_1:
	case D3D_FEATURE_LEVEL_12_0:
	case D3D_FEATURE_LEVEL_11_1:
	case D3D_FEATURE_LEVEL_11_0:
		// D3D11 feature level 12.1+ supports objects in shader model 5.1, although it doesn't support shader model 5.1 bytecode
		caps_.cs_support = true;
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
}