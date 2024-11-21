#pragma once
#include <render/IRenderDevice.h>
#include <d3d9.h>
#include <ddraw.h> 

namespace CoreWorker
{
class D3D9RenderDevice: public IRenderDevice
{
public:
    D3D9RenderDevice(HWND hwnd, int width, int height);

    bool CreateDevice();
    void Destroy();
    void Render();
private:
    HWND 			                    Hwnd_ = 0;
    int                                 nWidth_ = 0;
    int                                 Height_ = 0;


    LPDIRECT3D9							pd3d_;
	LPDIRECT3DDEVICE9					pd3dDevice_;	
	D3DPRESENT_PARAMETERS				d3dpp_;
    DDSCAPS2                            ddscaps_;                   //显示表面的功能控制设置

    LPDIRECT3DSURFACE9                  lpddsprimary_ = nullptr;	// 显示表面
	LPDIRECT3DSURFACE9                  lpddsback_ = nullptr;	    // 缓存表面
	LPDIRECT3DSURFACE9                  lpddszback_ = nullptr;	    // z缓存表面
};
}