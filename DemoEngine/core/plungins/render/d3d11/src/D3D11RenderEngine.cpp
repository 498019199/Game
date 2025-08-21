#include <base/Context.h>
#include <base/WinApp.h>
#include <render/RenderEffect.h>

#include "D3D11RenderEngine.h"
#include "D3D11Util.h"
#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderLayout.h"

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
	const auto& Cfg = Context::Instance().Config();
	HWND hwnd = Context::Instance().AppInstance().GetHWND();

    // Create the device and device context.
    D3D_DRIVER_TYPE dev_type = D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE;
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDevice(
			0,                 // default adapter
			dev_type,
			0,                 // no software device
			createDeviceFlags, 
			0, 0,              // default feature level array
			D3D11_SDK_VERSION,
			d3d_device_.put(),
			&d3d_feature_level_,
			d3d_imm_ctx_.put());

	if( FAILED(hr) )
	{
		::MessageBoxW(0, L"D3D11CreateDevice Failed.", 0, 0);
		return ;
	}

	if( d3d_feature_level_ != D3D_FEATURE_LEVEL_11_0 )
	{
		::MessageBoxW(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return ;
	}

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

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	// TIFHR(d3d_device_->CheckMultisampleQualityLevels(
	// 	DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
	// COMMON_ASSERT( m4xMsaaQuality > 0 );

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = Cfg.graphics_cfg.width;

	sd.BufferDesc.Height = Cfg.graphics_cfg.height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    sd.SampleDesc.Count   = Cfg.graphics_cfg.sample_count;
    sd.SampleDesc.Quality = Cfg.graphics_cfg.sample_quality;

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed     = true;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	TIFHR(d3d_device_->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	      
	IDXGIAdapter* dxgiAdapter = 0;
	TIFHR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	TIFHR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	TIFHR(dxgiFactory->CreateSwapChain(d3d_device_.get(), &sd, swap_chain_.put()));
	
	if(dxgiDevice)
    	dxgiDevice->Release();
	if(dxgiAdapter)
		dxgiAdapter->Release();
	if(dxgiFactory)
		dxgiFactory->Release();
	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	
	weight_ = Cfg.graphics_cfg.width;
    height_ = Cfg.graphics_cfg.height;
    sample_count_ = Cfg.graphics_cfg.sample_count;
    sample_quality_ = Cfg.graphics_cfg.sample_quality;
	OnResize();

	FillRenderDeviceCaps();
	return ;
}

D3D11RenderEngine::~D3D11RenderEngine()
{
	render_target_view_.reset();
	depth_stencil_view_.reset();
	swap_chain_.reset();
	depth_stencil_buff_.reset();

	if( d3d_imm_ctx_ )
		d3d_imm_ctx_->ClearState();

	d3d_imm_ctx_.reset();
	d3d_device_.reset();
}

void D3D11RenderEngine::OnResize()
{
	COMMON_ASSERT(d3d_imm_ctx_);
	COMMON_ASSERT(d3d_device_);
	COMMON_ASSERT(swap_chain_);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	render_target_view_.reset();
	depth_stencil_view_.reset();
	depth_stencil_buff_.reset();

	// Resize the swap chain and recreate the render target view.
	TIFHR(swap_chain_->ResizeBuffers(1, weight_, height_, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	TIFHR(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	TIFHR(d3d_device_->CreateRenderTargetView(backBuffer, 0, render_target_view_.put()));
	if(backBuffer)
		backBuffer->Release();

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width     = weight_;
	depthStencilDesc.Height    = height_;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// Use 4X MSAA? --must match swap chain MSAA values.
	depthStencilDesc.SampleDesc.Count   = sample_count_;
	depthStencilDesc.SampleDesc.Quality = sample_quality_;

	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	TIFHR(d3d_device_->CreateTexture2D(&depthStencilDesc, 0, depth_stencil_buff_.put()));
	TIFHR(d3d_device_->CreateDepthStencilView(depth_stencil_buff_.get(), 0, depth_stencil_view_.put()));

	// Bind the render target view and depth/stencil view to the pipeline.
	d3d_imm_ctx_->OMSetRenderTargets(1, render_target_view_.put(), depth_stencil_view_.get());

	// Set the viewport transform.
	screen_viewport_.TopLeftX = 0;
	screen_viewport_.TopLeftY = 0;
	screen_viewport_.Width    = static_cast<float>(weight_);
	screen_viewport_.Height   = static_cast<float>(height_);
	screen_viewport_.MinDepth = 0.0f;
	screen_viewport_.MaxDepth = 1.0f;
	d3d_imm_ctx_->RSSetViewports(1, &screen_viewport_);
}

#if ZENGINE_IS_DEV_PLATFORM
void* D3D11RenderEngine::GetD3DDevice()
{
	return d3d_device_.get();
}

void* D3D11RenderEngine::GetD3DDeviceImmContext()
{
	return d3d_imm_ctx_.get();
}
#endif//ZENGINE_IS_DEV_PLATFORM

void D3D11RenderEngine::BeginRender() const 
{
	COMMON_ASSERT(d3d_imm_ctx_);
	COMMON_ASSERT(swap_chain_);

	Color blackColor(0.0, 0.0, 0.0f, 1.0f);
	d3d_imm_ctx_->ClearRenderTargetView(render_target_view_.get(), &blackColor.r());
	d3d_imm_ctx_->ClearDepthStencilView(depth_stencil_view_.get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void D3D11RenderEngine::EndRender() const
{

}

void D3D11RenderEngine::SwitchChain() const
{
    TIFHR(swap_chain_->Present(0, 0));
}

ID3D11Device* D3D11RenderEngine::D3DDevice() const
{
	return d3d_device_.get();
}

ID3D11DeviceContext* D3D11RenderEngine::D3DDeviceImmContext() const
{
    return d3d_imm_ctx_.get();
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
			d3d_imm_ctx_->IASetVertexBuffers(0, vertex_stream_num, &vbs[0], &strides[0], &offsets[0]);
			vb_cache_ = vbs;
			vb_stride_cache_ = strides;
			vb_offset_cache_ = offsets;
		}

		auto layout = d3d_rl.InputLayout(effect.ShaderObjectByIndex(0).get());
		if (layout != input_layout_cache_)
		{
			d3d_imm_ctx_->IASetInputLayout(layout);
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
			d3d_imm_ctx_->IASetVertexBuffers(0, static_cast<UINT>(vb_cache_.size()),
				&vb_cache_[0], &vb_stride_cache_[0], &vb_offset_cache_[0]);
			vb_cache_.clear();
			vb_stride_cache_.clear();
			vb_offset_cache_.clear();
		}
		input_layout_cache_ = nullptr;
		d3d_imm_ctx_->IASetInputLayout(input_layout_cache_);
	}

    // 设置图元类型，设定输入布局
	uint32_t const vertex_count = static_cast<uint32_t>(rl.UseIndices() ? rl.IndicesNum() : rl.NumVertices());
	D3D11RenderLayout::topology_type tt = rl.TopologyType();
	if (topology_type_cache_ != tt)
	{
		d3d_imm_ctx_->IASetPrimitiveTopology(D3D11Mapping::Mapping(tt));
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
			d3d_imm_ctx_->IASetIndexBuffer(d3dib, D3D11Mapping::MappingFormat(rl.IndexStreamFormat()), 0);
		}
	}
	else
	{
		if (ib_cache_)
		{
			d3d_imm_ctx_->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
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
			d3d_imm_ctx_->DrawIndexed(num_vertices_just_rendered_, 0, 0);
		else
			d3d_imm_ctx_->DrawAuto();
		pass.Unbind(effect);
	}
}

void D3D11RenderEngine::RSSetState(ID3D11RasterizerState* ras)
{
	if (rasterizer_state_cache_ != ras)
	{
		d3d_imm_ctx_->RSSetState(ras);
		rasterizer_state_cache_ = ras;
	}
}

void D3D11RenderEngine::OMSetBlendState(ID3D11BlendState* bs, const Color& blend_factor, uint32_t sample_mask)
{
	if ((blend_state_cache_ != bs) || (blend_factor_cache_ != blend_factor) || (sample_mask_cache_ != sample_mask))
	{
		d3d_imm_ctx_->OMSetBlendState(bs, &blend_factor.r(), sample_mask);
		blend_state_cache_ = bs;
		blend_factor_cache_ = blend_factor;
		sample_mask_cache_ = sample_mask;
	}
}

void D3D11RenderEngine::OMSetDepthStencilState(ID3D11DepthStencilState* ds, uint16_t stencil_ref)
{
	if ((depth_stencil_state_cache_ != ds) || (stencil_ref_cache_ != stencil_ref))
	{
		d3d_imm_ctx_->OMSetDepthStencilState(ds, stencil_ref);
		depth_stencil_state_cache_ = ds;
		stencil_ref_cache_ = stencil_ref;
	}
}

void D3D11RenderEngine::VSSetShader(ID3D11VertexShader* shader)
{
	if (vertex_shader_cache_ != shader)
	{
		d3d_imm_ctx_->VSSetShader(shader, nullptr, 0);
		vertex_shader_cache_ = shader;
	}
}

void D3D11RenderEngine::PSSetShader(ID3D11PixelShader* shader)
{
	if (pixel_shader_cache_ != shader)
	{
		d3d_imm_ctx_->PSSetShader(shader, nullptr, 0);
		pixel_shader_cache_ = shader;
	}
}

void D3D11RenderEngine::GSSetShader(ID3D11GeometryShader* shader)
{
	if (geometry_shader_cache_ != shader)
	{
		d3d_imm_ctx_->GSSetShader(shader, nullptr, 0);
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

		ShaderSetShaderResources[stage_index](d3d_imm_ctx_.get(), 0,
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
		ShaderSetSamplers[stage_index](d3d_imm_ctx_.get(), 0, static_cast<UINT>(samplers.size()), &(samplers[0]));

		shader_sampler_ptr_cache_[stage_index].assign(samplers.begin(), samplers.end());
	}
}

void D3D11RenderEngine::SetConstantBuffers(ShaderStage stage, std::span<ID3D11Buffer* const> cbs)
{
	uint32_t const stage_index = std::to_underlying(stage);
	if (MakeSpan(shader_cb_ptr_cache_[stage_index]) != cbs)
	{
		ShaderSetConstantBuffers[stage_index](d3d_imm_ctx_.get(), 0, static_cast<UINT>(cbs.size()), &(cbs[0]));

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
				d3d_imm_ctx_.get(), 0, static_cast<UINT>(shader_srv_ptr_cache_[stage].size()), &shader_srv_ptr_cache_[stage][0]);
		}
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