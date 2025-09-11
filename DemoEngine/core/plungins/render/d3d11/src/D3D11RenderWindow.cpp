#include <base/ZEngine.h>
#include <base/App3D.h>
#include <base/Window.h>

#include "D3D11RenderWindow.h"
#include "D3D11RenderEngine.h"
#include "D3D11RenderFactory.h"
#include "D3D11Texture.h"

namespace RenderWorker
{

D3D11RenderWindow::D3D11RenderWindow(D3D11Adapter* adapter, const std::string& name, RenderSettings const& settings)
    :adapter_(adapter)
{
    // Store info
    name_				= name;
    is_full_screen_		= settings.full_screen;
    sync_interval_		= settings.sync_interval;

    auto const& main_wnd = Context::Instance().AppInstance().MainWnd().get();
	wnd_ = main_wnd->GetHWND();

    if (this->FullScreen())
    {
        float const dpi_scale = main_wnd->DPIScale();

        left_ = 0;
        top_ = 0;
        width_ = static_cast<uint32_t>(settings.width * dpi_scale + 0.5f);
        height_ = static_cast<uint32_t>(settings.height * dpi_scale + 0.5f);
    }
    else
    {
        left_ = main_wnd->Left();
        top_ = main_wnd->Top();
        width_ = main_wnd->Width();
        height_ = main_wnd->Height();
    }

    viewport_->Left(0);
    viewport_->Top(0);
    viewport_->Width(width_);
    viewport_->Height(height_);

    ElementFormat format = settings.color_fmt;
	back_buffer_format_ = D3D11Mapping::MappingFormat(format);

    RenderFactory& rf = Context::Instance().RenderFactoryInstance();
    auto& d3d11_re = checked_cast<D3D11RenderEngine&>(rf.RenderEngineInstance());
    ID3D11Device1Ptr d3d_device = d3d11_re.D3DDevice1();
    ID3D11DeviceContext1Ptr d3d_imm_ctx;

    dxgi_stereo_support_ = d3d11_re.DXGIFactory2()->IsWindowedStereoEnabled() ? true : false;
#ifdef ZENGINE_PLATFORM_WINDOWS_STORE
	// Async swap chain doesn't get along very well with desktop full screen
	dxgi_async_swap_chain_ = true;
#endif // 

    if (d3d11_re.DXGISubVer() >= 5)
    {
        BOOL allow_tearing = FALSE;
        // 检查是否支持关闭垂直同步
        if (SUCCEEDED(d3d11_re.DXGIFactory5()->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
            &allow_tearing, sizeof(allow_tearing))))
        {
            dxgi_allow_tearing_ = allow_tearing ? true : false;
        }
    }
    
    // create d3d_device, d3d_imm_ctx
    if (d3d_device)
    {
        d3d_imm_ctx = d3d11_re.D3DDeviceImmContext1();
        main_wnd_ = false;
    }
    else
    {
        UINT const create_device_flags = 0;
        static UINT const all_create_device_flags[] =
        {
            create_device_flags | D3D11_CREATE_DEVICE_DEBUG,
            create_device_flags
        };

        std::span<UINT const> available_create_device_flags = MakeSpan(all_create_device_flags);
#ifdef ZENGINE_DEBUG
        available_create_device_flags = MakeSpan(all_create_device_flags);
#else
        if (!settings.debug_context)
        {
            available_create_device_flags = available_create_device_flags.subspan(1);
        }
#endif

        static std::pair<D3D_DRIVER_TYPE, std::wstring_view> const dev_type_behaviors[] =
        {
            {D3D_DRIVER_TYPE_HARDWARE, L"HW"},
            {D3D_DRIVER_TYPE_WARP, L"WARP"},
        };

        static D3D_FEATURE_LEVEL constexpr all_feature_levels[] =
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0
        };

        std::span<D3D_FEATURE_LEVEL const> feature_levels;
        {
            static size_t constexpr feature_level_name_hashes[] =
            {
                CtHash("12_1"),
                CtHash("12_0"),
                CtHash("11_1"),
                CtHash("11_0")
            };
            static_assert(std::size(feature_level_name_hashes) == std::size(all_feature_levels));

            uint32_t feature_level_start_index = 0;
            if (d3d11_re.DXGISubVer() < 4)
            {
                feature_level_start_index = 2;

                if (d3d11_re.DXGISubVer() < 2)
                {
                    feature_level_start_index = 3;
                }
            }
            for (size_t index = 0; index < settings.options.size(); ++ index)
            {
                size_t const opt_name_hash = RtHash(settings.options[index].first.c_str());
                size_t const opt_val_hash = RtHash(settings.options[index].second.c_str());
                if (CtHash("level") == opt_name_hash)
                {
                    for (uint32_t i = feature_level_start_index; i < std::size(feature_level_name_hashes); ++ i)
                    {
                        if (feature_level_name_hashes[i] == opt_val_hash)
                        {
                            feature_level_start_index = i;
                            break;
                        }
                    }
                }
            }

            feature_levels = MakeSpan(all_feature_levels).subspan(feature_level_start_index);
        }

        for (auto const & dev_type_beh : dev_type_behaviors)
        {
            d3d_device.reset();
            d3d_imm_ctx.reset();
            IDXGIAdapter* dx_adapter = nullptr;
            D3D_DRIVER_TYPE dev_type = dev_type_beh.first;
            if (D3D_DRIVER_TYPE_HARDWARE == dev_type)
            {
                dx_adapter = adapter_->DXGIAdapter();
                dev_type = D3D_DRIVER_TYPE_UNKNOWN;
            }

            D3D_FEATURE_LEVEL out_feature_level = D3D_FEATURE_LEVEL_11_0;
            HRESULT hr = E_FAIL;
            for (auto const & flags : available_create_device_flags)
            {
                com_ptr<ID3D11Device> this_device;
                com_ptr<ID3D11DeviceContext> this_imm_ctx;
                D3D_FEATURE_LEVEL this_out_feature_level;
                hr = d3d11_re.D3D11CreateDevice(dx_adapter, dev_type, nullptr, flags,
                    &feature_levels[0], static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, this_device.put(),
                    &this_out_feature_level, this_imm_ctx.put());
                if (SUCCEEDED(hr))
                {
                    d3d_device = this_device.as<ID3D11Device1>();
                    d3d_imm_ctx = this_imm_ctx.as<ID3D11DeviceContext1>();
                    out_feature_level = this_out_feature_level;
                    break;
                }
            }

            // 给D3D11RenderEngine赋值
            if (SUCCEEDED(hr))
            {
                d3d11_re.D3DDevice(d3d_device.get(), d3d_imm_ctx.get(), out_feature_level);

                // if (Context::Instance().AppInstance().OnConfirmDevice()())
                // {
                //     if (dev_type != D3D_DRIVER_TYPE_HARDWARE)
                //     {
                //         if (auto dxgi_device = d3d_device.try_as<IDXGIDevice2>())
                //         {
                //             com_ptr<IDXGIAdapter> ada;
                //             dxgi_device->GetAdapter(ada.put());
                //             adapter_->ResetAdapter(ada.as<IDXGIAdapter2>().get());
                //             adapter_->Enumerate();
                //         }
                //     }
                // }

                
                description_ = adapter_->Description() + L" " + dev_type_beh.second.data() + L" FL ";
                std::wstring_view fl_str;
                switch (out_feature_level)
                {
                case D3D_FEATURE_LEVEL_12_1:
                    fl_str = L"12.1";
                    break;

                case D3D_FEATURE_LEVEL_12_0:
                    fl_str = L"12.0";
                    break;

                case D3D_FEATURE_LEVEL_11_1:
                    fl_str = L"11.1";
                    break;

                case D3D_FEATURE_LEVEL_11_0:
                    fl_str = L"11.0";
                    break;

                default:
                    fl_str = L"Unknown";
                    break;
                }
                description_ += fl_str.data();
                if (settings.sample_count > 1)
                {
                    description_ += L" (" + std::to_wstring(settings.sample_count) + L"x AA)";
                }
                break;
            }
            else
            {
                d3d_device.reset();
                d3d_imm_ctx.reset();
            }
        }

		main_wnd_ = true;
    }

    Verify(d3d_device != nullptr);
    Verify(d3d_imm_ctx != nullptr);
    //COMMON_ASSUME(d3d_device != nullptr);
    //COMMON_ASSUME(d3d_imm_ctx != nullptr);

    // 创建交换链
    depth_stencil_fmt_ = settings.depth_stencil_fmt;
    if (IsDepthFormat(depth_stencil_fmt_))
    {
        COMMON_ASSERT((EF_D32F == depth_stencil_fmt_) || (EF_D24S8 == depth_stencil_fmt_)
            || (EF_D16 == depth_stencil_fmt_));

        UINT format_support;
        if (EF_D32F == depth_stencil_fmt_)
        {
            // Try 32-bit zbuffer
            d3d_device->CheckFormatSupport(DXGI_FORMAT_D32_FLOAT, &format_support);
            if (!(format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
            {
                depth_stencil_fmt_ = EF_D24S8;
            }
            if (EF_D24S8 == depth_stencil_fmt_)
			{
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &format_support);
				if (!(format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
				{
					depth_stencil_fmt_ = EF_D16;
				}
			}
			if (EF_D16 == depth_stencil_fmt_)
			{
				d3d_device->CheckFormatSupport(DXGI_FORMAT_D16_UNORM, &format_support);
				if (!(format_support & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL))
				{
					depth_stencil_fmt_ = EF_Unknown;
				}
			}
        }
    }

    bool const stereo = (STM_LCDShutter == settings.stereo_method) && dxgi_stereo_support_;
    sc_desc1_.Width = this->Width();
    sc_desc1_.Height = this->Height();
    sc_desc1_.Format = back_buffer_format_;
    sc_desc1_.Stereo = stereo;
    sc_desc1_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc1_.BufferCount = 2;
    sc_desc1_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    // 为Stereo 3D注册一个窗口以接收立体显示状态变化的通知消息
    d3d11_re.DXGIFactory2()->RegisterStereoStatusWindow(wnd_, WM_SIZE, &stereo_cookie_);

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
        sc_desc1_.SampleDesc.Count = std::min(static_cast<uint32_t>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT), settings.sample_count);
        sc_desc1_.SampleDesc.Quality = settings.sample_quality;
        sc_desc1_.Scaling = DXGI_SCALING_STRETCH;
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
    sc_fs_desc_.Windowed = !this->FullScreen();
#else
#endif//

    if (dxgi_allow_tearing_)
    {
        sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }
    if (dxgi_async_swap_chain_)
    {
        sc_desc1_.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    }

    this->CreateSwapChain(d3d_device.get(), settings.display_output_method != DOM_sRGB);
    Verify(!!swap_chain_1_);

#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    //DXGI 中窗口关联和全屏状态设置的操作
	d3d11_re.DXGIFactory2()->MakeWindowAssociation(wnd_, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
	swap_chain_1_->SetFullscreenState(this->FullScreen(), nullptr);
#endif

    // 启用多线程保护模式，使 Direct3D 设备能够安全地接受来自多个线程的 API 调用
    if (auto d3d_multithread = d3d_device.try_as<ID3D10Multithread>())
    {
        d3d_multithread->SetMultithreadProtected(true);
    }

    // 创建渲染目标视图,深度/模板缓冲区及其视图
    this->UpdateSurfacesPtrs();

#ifdef ZENGINE_DEBUG
    // Direct3D 11 中用于调试的设置，主要功能是配置 Direct3D 信息队列（Info Queue），使其在遇到严重错误时触发调试断点。
    if (auto d3d_info_queue = d3d_device.try_as<ID3D11InfoQueue>())
    {
        // 当出现 "corruption" 级别的消息（通常是严重的内存损坏或数据一致性问题）时，触发调试断点
        d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
        // 当出现 "error" 级别的消息（Direct3D 函数调用失败等错误）时，触发调试断点
        d3d_info_queue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
    }
#endif
}

D3D11RenderWindow::~D3D11RenderWindow()
{
    
}

void D3D11RenderWindow::Destroy()
{
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    // 释放全屏独占资源
    if (swap_chain_1_)
    {
        swap_chain_1_->SetFullscreenState(false, nullptr);
    }

    RenderFactory& rf = Context::Instance().RenderFactoryInstance();
    auto const& d3d11_re = checked_cast<const D3D11RenderEngine&>(rf.RenderEngineInstance());
    d3d11_re.DXGIFactory2()->UnregisterStereoStatus(stereo_cookie_);
#else
#endif // ZENGINE_PLATFORM_WINDOWS_DESKTOP

    render_target_view_right_eye_.reset();
    depth_stencil_view_right_eye_.reset();
    render_target_view_.reset();
    depth_stencil_view_.reset();
    back_buffer_.reset();
    depth_stencil_.reset();
    swap_chain_1_.reset();
}

void D3D11RenderWindow::SwapBuffers()
{
    if (swap_chain_1_)
    {
        bool allow_tearing = dxgi_allow_tearing_ && (sync_interval_ == 0);
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
        allow_tearing &= !is_full_screen_;
#endif
        UINT const present_flags = allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
        TIFHR(swap_chain_1_->Present(sync_interval_, present_flags));

        if (DXGI_PRESENT_ALLOW_TEARING == present_flags)
        {
            views_dirty_ = true;
        }
    }
}

void D3D11RenderWindow::WaitOnSwapBuffers()
{
    if (swap_chain_1_ && dxgi_async_swap_chain_)
    {
        ::WaitForSingleObjectEx(frame_latency_waitable_obj_.get(), 1000, true);
    }
}

bool D3D11RenderWindow::FullScreen() const
{
    return is_full_screen_;
}

void D3D11RenderWindow::FullScreen(bool fs)
{
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    if (is_full_screen_ != fs)
    {
        left_ = 0;
        top_ = 0;

        uint32_t style;
        if (fs)
        {
            style = WS_POPUP;
        }
        else
        {
            style = WS_OVERLAPPEDWINDOW;
        }

        ::SetWindowLongPtrW(wnd_, GWL_STYLE, style);

        RECT rc = { 0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_) };
        ::AdjustWindowRect(&rc, style, false);
        width_ = rc.right - rc.left;
        height_ = rc.bottom - rc.top;
        ::SetWindowPos(wnd_, nullptr, left_, top_, width_, height_, SWP_NOZORDER);

        sc_desc1_.Width = width_;
        sc_desc1_.Height = height_;
        sc_fs_desc_.Windowed = !fs;

        is_full_screen_ = fs;

        swap_chain_1_->SetFullscreenState(is_full_screen_, nullptr);
        if (is_full_screen_)
        {
            DXGI_MODE_DESC desc;
            desc.Width = width_;
            desc.Height = height_;
            desc.RefreshRate.Numerator = 60;
            desc.RefreshRate.Denominator = 1;
            desc.Format = back_buffer_format_;
            desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            swap_chain_1_->ResizeTarget(&desc);
        }

        ::ShowWindow(wnd_, SW_SHOWNORMAL);
        ::UpdateWindow(wnd_);
    }
#else
#endif
}

void D3D11RenderWindow::UpdateSurfacesPtrs()
{
    RenderFactory& rf = Context::Instance().RenderFactoryInstance();
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    if (dxgi_allow_tearing_)
#endif
    {
    }

    // 创建渲染目标视图
    ID3D11Texture2DPtr back_buffer;
    TIFHR(swap_chain_1_->GetBuffer(0, UuidOf<ID3D11Texture2D>(), back_buffer.put_void()));
    back_buffer_ = MakeSharedPtr<D3D11Texture2D>(back_buffer);
	render_target_view_ = rf.Make2DRtv(back_buffer_, 0, 1, 0);

	bool stereo = (STM_LCDShutter == Context::Instance().Config().graphics_cfg.stereo_method) && dxgi_stereo_support_;
    if (stereo)
    {
        render_target_view_right_eye_ = rf.Make2DRtv(back_buffer_, 1, 1, 0);
    }

    // 创建深度/模板缓冲区及其视图
    if (depth_stencil_fmt_ != EF_Unknown)
    {
        depth_stencil_ = rf.MakeTexture2D(width_, height_, 1, stereo ? 2 : 1, depth_stencil_fmt_,
            back_buffer_->SampleCount(), back_buffer_->SampleQuality(),
            EAH_GPU_Read | EAH_GPU_Write);

		depth_stencil_view_ = rf.Make2DDsv(depth_stencil_, 0, 1, 0);
        
        if (stereo)
        {
            depth_stencil_view_right_eye_ = rf.Make2DDsv(depth_stencil_, 1, 1, 0);
        }
    }

    this->Attach(Attachment::Color0, render_target_view_);
    if (depth_stencil_view_)
    {
        this->Attach(depth_stencil_view_);
    }
}

void D3D11RenderWindow::CreateSwapChain(ID3D11Device* d3d_device, bool try_hdr_display)
{
	auto const& d3d11_re = checked_cast<D3D11RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
    // 传统 HWND 窗口（基于 Win32 API 创建的窗口）
    d3d11_re.DXGIFactory2()->CreateSwapChainForHwnd(d3d_device, wnd_, &sc_desc1_, &sc_fs_desc_, nullptr, swap_chain_1_.put());
#else
    // CoreWindow 窗口（基于 UWP/WinRT 框架的窗口）
    d3d11_re.DXGIFactory2()->CreateSwapChainForCoreWindow(
        d3d_device, static_cast<IUnknown*>(uwp::get_abi(wnd_)), &sc_desc1_, nullptr, swap_chain_1_.put());
#endif

    if (dxgi_async_swap_chain_)
    {
        IDXGISwapChain3Ptr sc3;
        if (swap_chain_1_.try_as(sc3))
        {
            frame_latency_waitable_obj_ = MakeWin32UniqueHandle(sc3->GetFrameLatencyWaitableObject());
        }
    }

    if (try_hdr_display)
    {
        IDXGISwapChain4Ptr sc4;
        if (swap_chain_1_.try_as(sc4))
        {
            UINT color_space_support;
            // 检查系统是否支持 HDR 颜色空间
            if (SUCCEEDED(sc4->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &color_space_support))
                && (color_space_support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
            {
                sc4->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
            }
        }
    }
}
}