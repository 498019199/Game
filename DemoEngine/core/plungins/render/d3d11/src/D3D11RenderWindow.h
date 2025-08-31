#pragma once
#include <render/FrameBuffer.h>
#include "D3D11AdapterList.h"
#include "D3D11RenderEngine.h"

namespace RenderWorker
{
class D3D11RenderWindow:public FrameBuffer
{
public:
    D3D11RenderWindow(D3D11Adapter* adapter, const std::string& name, RenderSettings const& settings);
    ~D3D11RenderWindow();

private:
	D3D11Adapter* adapter_;

    DXGI_SWAP_CHAIN_DESC1 sc_desc1_;
#ifdef ZENGINE_PLATFORM_WINDOWS_DESKTOP
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sc_fs_desc_;
	DWORD stereo_cookie_;
#endif
	IDXGISwapChain1Ptr swap_chain_1_;
};
using D3D11RenderWindowPtr = std::shared_ptr<D3D11RenderWindow>;
}