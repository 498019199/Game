#pragma once

#include "D3D11RenderEngine.h"
#include "D3D11AdapterList.h"
#include "D3D11FrameBuffer.h"

namespace RenderWorker
{
class D3D11RenderWindow:public D3D11FrameBuffer
{
public:
    D3D11RenderWindow(D3D11Adapter* adapter, const std::string& name, RenderSettings const& settings);
    ~D3D11RenderWindow();
	
	void SwapBuffers() override;
	void WaitOnSwapBuffers() override;

	// 获取是否是全屏状态
	bool FullScreen() const;
	// 设置是否是全屏状态
	void FullScreen(bool fs);

private:
	void CreateSwapChain(ID3D11Device* d3d_device, bool try_hdr_display);

private:
	std::string name_;
	HWND wnd_; // Win32 Window handle

	bool is_full_screen_ {false};
	uint32_t sync_interval_ {0};

	D3D11Adapter* adapter_;

    DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
	DWORD stereo_cookie_;
#endif

	bool dxgi_stereo_support_ {false};
	bool dxgi_allow_tearing_ {false};
	bool dxgi_async_swap_chain_ {false};

	IDXGISwapChain1Ptr swap_chain_1_;
	bool main_wnd_  {false};
	Win32UniqueHandle frame_latency_waitable_obj_;

	ID3D11Texture2DPtr depth_stencil_buff_;
    ID3D11DepthStencilViewPtr depth_stencil_view_;
    ID3D11RenderTargetViewPtr render_target_view_;

	DXGI_FORMAT back_buffer_format_;
	ElementFormat depth_stencil_fmt_;

	std::wstring description_;
};
using D3D11RenderWindowPtr = std::shared_ptr<D3D11RenderWindow>;
}