#include "D3D11RenderWindow.h"
#include "D3D11RenderDevice.h"
#include "D3D11Util.h"
#include "../../app/windows/WindowApp.h"
#include <algorithm>

namespace CoreWorker
{
D3D11RenderWindow::D3D11RenderWindow(D3D11Adapter* adapter, const std::string strName, const FWindowDesc& WindowDesc)
    :adapter_(adapter)
{
    auto app = CommonWorker::checked_cast<WindowApp*>(Context::Instance()->GetMainApp());
    auto d3d11_re = CommonWorker::checked_cast<D3D11RenderDevice*>(Context::Instance()->GetRenderDevice());
    auto d3d11_device = d3d11_re->D3DDevice1();
    dxgi_stereo_support_ = d3d11_re->DXGIFactory2()->IsWindowedStereoEnabled() ? true : false;
    hWnd_ = app->GetHWnd();
    full_screen_ = WindowDesc.full_screen;
    ElementFormat format = WindowDesc.color_fmt;

    if (d3d11_re->DXGISubVer() >= 5)
    {
        BOOL allow_tearing = FALSE;
        if (SUCCEEDED(d3d11_re->DXGIFactory5()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allow_tearing, sizeof(allow_tearing))))
        {
            dxgi_allow_tearing_ = allow_tearing ? true : false;
        }
    }

    ID3D11DeviceContext1* d3d_imm_ctx = nullptr;
    if (d3d11_re)
    {
        d3d_imm_ctx = d3d11_re->D3DDeviceImmContext1();
    }

    back_buffer_format_ = D3D11Mapping::MappingFormat(format);

    bool const stereo = false;
    sc_desc1_.Width = WindowDesc.width;
    sc_desc1_.Height = WindowDesc.height;
    sc_desc1_.Format = back_buffer_format_; // 后台缓冲区像素格式
    sc_desc1_.Stereo = stereo;
    sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc1_.BufferCount = 2;
    sc_desc1_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sc_desc1_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (stereo)
    {
        sc_desc1_.SampleDesc.Count = 1;
        sc_desc1_.SampleDesc.Quality = 0;
        sc_desc1_.Scaling = DXGI_SCALING_NONE;
        sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    }
    else
    {
        // 多重采样数量和质量级别
        sc_desc1_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), WindowDesc.sample_count);
        sc_desc1_.SampleDesc.Quality = WindowDesc.sample_quality;
        // display scaling mode
        sc_desc1_.Scaling = DXGI_SCALING_STRETCH;
        // 设为DXGI_SWAP_EFFECT_DISCARD，让显卡驱动程序选择最高效的显示模式
        if (dxgi_allow_tearing_)
        {
            sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        else
        {
            sc_desc1_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        }
    }

    sc_fs_desc_.RefreshRate.Numerator = 60;
	sc_fs_desc_.RefreshRate.Denominator = 1;
	sc_fs_desc_.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sc_fs_desc_.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sc_fs_desc_.Windowed = !full_screen_;

    if (dxgi_allow_tearing_)
    {
        sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }
    if (dxgi_async_swap_chain_)
    {
        sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    }

    // 创建交换链
    CreateSwapChain(d3d11_device, false);

    d3d11_re->DXGIFactory2()->MakeWindowAssociation(hWnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
	swap_chain_1_->SetFullscreenState(full_screen_, nullptr);
}

void D3D11RenderWindow::CreateSwapChain(ID3D11Device* d3d_device, bool try_hdr_display)
{
    auto d3d11_re = CommonWorker::checked_cast<D3D11RenderDevice*>(Context::Instance()->GetRenderDevice());
    HRESULT hr = d3d11_re->DXGIFactory2()->CreateSwapChainForHwnd(d3d_device, hWnd_, &sc_desc1_, &sc_fs_desc_, nullptr, &swap_chain_1_);
    if (!SUCCEEDED(hr))
    {
        LOGER_ERROR() << "Create swap chain failed!";
        return ;
    }

    if (try_hdr_display)
    {
        IDXGISwapChain4* sc4 = nullptr;
        if (SUCCEEDED(swap_chain_1_->QueryInterface(&sc4)))
        {
            UINT color_space_support;
            if (SUCCEEDED(sc4->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &color_space_support))
                && (color_space_support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
            {
                sc4->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
            }
        }
    }
}

D3D11RenderWindow::~D3D11RenderWindow()
{
    Destroy();
}

void D3D11RenderWindow::Destroy()
{}

void D3D11RenderWindow::SwapBuffers()
{}

void D3D11RenderWindow::WaitOnSwapBuffers()
{}
}