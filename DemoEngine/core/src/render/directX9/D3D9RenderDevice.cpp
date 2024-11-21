#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Winmm.lib")

#include <core/IContext.h>
#include "D3D9RenderDevice.h"
#include <dxdiag.h>
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }      //自定义一个SAFE_RELEASE()宏,便于COM资源的释放 

namespace CoreWorker
{
D3D9RenderDevice::D3D9RenderDevice(HWND hwnd, int width, int height)
	:Hwnd_(hwnd), nWidth_(width), Height_(height)
{
	
}

bool D3D9RenderDevice::CreateDevice()
{
	pd3d_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (nullptr == pd3d_)
	{
		// Failed to create D3D interface. Please check your video card.
		return false;
	}

	::ZeroMemory(&d3dpp_, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp_.BackBufferWidth = nWidth_;
	d3dpp_.BackBufferHeight = Height_;
	d3dpp_.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp_.BackBufferCount = 1;
	d3dpp_.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp_.MultiSampleQuality = 0;
	d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp_.hDeviceWindow = Hwnd_;
	d3dpp_.Windowed = false;
	d3dpp_.EnableAutoDepthStencil = true;
	d3dpp_.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp_.Flags = 0;
	d3dpp_.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = pd3d_->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		Hwnd_,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
		&d3dpp_,
		&pd3dDevice_);
	if (FAILED(hr))
	{
		//"Failed to create D3D device."
		return false;
	}

	pd3dDevice_->GetRenderTarget( 0, &lpddsback_ );
	pd3dDevice_->GetDepthStencilSurface( &lpddszback_ );
	return true;
}

void D3D9RenderDevice::Render()
{
	// 处理shader pass
	// 载入顶点，索引
	//pd3dDevice_->SetStreamSource();
	//pd3dDevice_->SetIndices(
}

// bool RenderDevice::InitDraw(HWND hwnd)
// {
//     if (FAILED(DirectDrawCreateEx(NULL, (void **)&m_draw, IID_IDirectDraw7, NULL)))
// 	{
// 		return false;
// 	}

// 	if (m_bWindows)
// 	{
// 		// 窗口模式
// 		if (FAILED(m_draw->SetCooperativeLevel(hwnd, DDSCL_NORMAL)))
// 		{
// 			return false;
// 		}
// 	} 
// 	else
// 	{
// 		// 全屏模式
// 		if (FAILED(m_draw->SetCooperativeLevel(hwnd, DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN |
// 			DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT | DDSCL_MULTITHREADED)))
// 		{
// 			return false;
// 		}

// 		if (FAILED(m_draw->SetDisplayMode(m_nWidth, m_nHeight, 16, 0, 0)))
// 		{
// 			return false;
// 		}
// 	}

// 	// 初始化表面描述体
// 	//INIT_DDRAW_STRUCT(m_ddsd);
// 	DDSURFACEDESC2 ddsd;
// 	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
// 	ddsd.dwSize = sizeof(ddsd);
// 	if (m_bWindows)
// 	{
// 		ddsd.dwFlags = DDSD_CAPS;
// 		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
// 		ddsd.dwBackBufferCount = 0;
// 	} 
// 	else
// 	{
// 		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
// 		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
// 		ddsd.dwBackBufferCount = 1;
// 	}

// 	// 创建窗口表面
// 	m_draw->CreateSurface(&ddsd, &m_lpddsprimary, NULL);
// 	BOOST_ASSERT_MSG(m_lpddsprimary, "create first surface fair!");
// 	// 获取像素格式
// 	DDPIXELFORMAT ddpf;
// 	INIT_DDRAW_STRUCT(ddpf);
// 	m_lpddsprimary->GetPixelFormat(&ddpf);
// 	m_pixel_format = ddpf.dwRGBBitCount;

// 	trace_log("pixel format = %d", m_pixel_format);

// 	if (m_bWindows)
// 	{
// 		m_lpddsback = DDrawCreateSurface(m_nWidth, m_nHeight, DDSCAPS_SYSTEMMEMORY);
// 	} 
// 	else
// 	{
// 		m_ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
// 		if (FAILED(m_lpddsprimary->GetAttachedSurface(&m_ddscaps, &m_lpddsback)))
// 		{
// 			return false;
// 		}
// 	}

// 	// 创建调色板
// 	//现在将调色板附加到主表面上。
// 	if (0/*== DD_PIXEL_FORMAT8*/)
// 	{
// #define MAX_COLORS_PALETTE  256
// 		memset(m_palette, 0, MAX_COLORS_PALETTE * sizeof(PALETTEENTRY));
// 		LoadPattleFormFile("PALDATA2.PAL");

// 		if (m_bWindows)
// 		{
// 			for (int i = 0; i < 10; ++i)
// 				m_palette[i].peFlags = m_palette[i + 246].peFlags = PC_EXPLICIT;

// 			if (FAILED(m_draw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE,
// 				m_palette, &m_lpddpal, NULL)))
// 			{
// 				return false;
// 			}
// 		} 
// 		else
// 		{
// 			if (FAILED(m_draw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256, 
// 				m_palette, &m_lpddpal, NULL)))
// 			{
// 				return false;
// 			}
// 		}

// 		m_lpddsprimary->SetPalette(m_lpddpal);
// 	} 

// 	// 置空表面
// 	if (m_bWindows)
// 	{
// 		DDrawFillSurface(m_lpddsback, 0);
// 	} 
// 	else
// 	{
// 		DDrawFillSurface(m_lpddsprimary, 0);
// 		DDrawFillSurface(m_lpddsback, 0);
// 	}

// 	// 设置窗口裁剪区域
// 	RECT screen_rect = { 0,0, (m_nWidth), (m_nHeight)};
// 	m_lpddclipper = DDrawAttachClipper(m_lpddsback, 1, &screen_rect);

// 	// 设置窗口裁剪模式
// 	if (m_bWindows)
// 	{
// 		if (FAILED(m_draw->CreateClipper(0, &m_lpddclipperwin, NULL)))
// 			return(0);

// 		if (FAILED(m_lpddclipperwin->SetHWnd(0, hwnd)))
// 			return(0);

// 		if (FAILED(m_lpddsprimary->SetClipper(m_lpddclipperwin)))
// 			return(0);
// 	}

// 	return true;
// }

// void DX9RenderDevice::BeginRender()
// {
// 	// 清理表面
// 	ClearSurface();
// 	// 锁住缓存表面
// 	DDrawBacklockSurface();
// }

// void DX9RenderDevice::EndRender()
// {
//     // 清理表面
// 	ClearSurface();
// 	// 锁住缓存表面
// 	DDrawBacklockSurface();   
// }

void D3D9RenderDevice::Destroy()
{
	SAFE_RELEASE(pd3d_);
	SAFE_RELEASE(pd3dDevice_);

	SAFE_RELEASE(lpddsprimary_);
	SAFE_RELEASE(lpddsback_);
	SAFE_RELEASE(lpddszback_);
}



}