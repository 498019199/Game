#pragma once
#include <core/IContext.h>
#include "D3D11Adapter.h"


namespace CoreWorker
{
class D3D11RenderWindow
{
public:
	D3D11RenderWindow(D3D11Adapter* adapter, const std::string strName, const FWindowDesc& WindowDesc);
	~D3D11RenderWindow();

    void Destroy();

	void SwapBuffers();
	void WaitOnSwapBuffers();

private:
    void CreateSwapChain(ID3D11Device* d3d_device, bool try_hdr_display);
    
private:
    HWND hWnd_;
    bool full_screen_ = false;

    D3D11Adapter* adapter_;
    
    bool dxgi_stereo_support_{false};
    bool dxgi_allow_tearing_{false};
    bool dxgi_async_swap_chain_{false};

    DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
    IDXGISwapChain1* swap_chain_1_;

    DXGI_FORMAT back_buffer_format_;
    ElementFormat depth_stencil_fmt_;
};
}
