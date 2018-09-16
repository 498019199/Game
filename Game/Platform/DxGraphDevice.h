#ifndef DX_GRAPHDEVICE_H_
#define DX_GRAPHDEVICE_H_
#pragma once
#pragma comment(lib, "ddraw.lib")

#include <d3d9.h>
#include <ddraw.h> 
#include "../Math/math.h"
#include "../Core/predefine.h"
#include "../Render/Renderable.h"
#include "../Core/Entity/Entity.h"
class DxGraphDevice : public IEntityEx
{
public:
	STX_ENTITY(DxGraphDevice, IEntityEx);

	DxGraphDevice(Context* pContext);

	~DxGraphDevice();

	static DxGraphDevice* Instance();

	virtual bool OnInit() override;

	virtual bool OnShut() override;

	virtual void Update() override;

	const LPDIRECT3DDEVICE9& GetDevice() { return m_device; }

	// 获得显示宽
	int GetWidth() const { return m_nWidth;  }

	// 获得显示高
	int GetHeight() const { return m_nHeight;  }

	// 锁住表面数据
	UCHAR* DDrawBacklockSurface();

	// 解锁表面数据
	void DDrawBackUnlockSurface();

	// 交换缓存表面
	void DDrawFilp();

	// 清理表面
	void ClearSurface();
	//************************************
	// Qualifier:											屏幕输出
	// Parameter: char * szText					输出文字
	// Parameter: int x								文字起始坐标x
	// Parameter: int y								文字起始坐标y
	// Parameter: COLORREF color			文字颜色
	//************************************
	void DrawTextGDI(char* szText, int x, int y, COLORREF color);
	void DrawTextGDI(char* szText, int x, int y, int color);

	// 释放directX
	void ShutDown();

	//************************************
	// Qualifier:
	// Parameter: HWND hwnd				窗口句柄
	// Parameter: UINT nWidth				屏幕宽
	// Parameter: UINT nHeight			屏幕高
	//************************************
	bool InitDevice(HWND hwnd, UINT  nWidth, UINT  nHeight, UINT nClientX, UINT nClintY, bool bWindows);

	// 初始化directdraw
	bool InitDraw(HWND hwnd);

	void BeginRender();

	void EndRender();

	VertexBuffer MakeCreationVertexBuffer(const VertexVec& vb);

	// 剔除三角形
	int CullingPolys(zbVertex4D* ps, int* encodes);

	// 转换到屏幕坐标
	float4 ViewportTransform(const float4& vert);

	void DrawTriangle(const RenderCVarlistPtr& cvList, const zbVertex4D* vertices);
	void DrawTriangle2D(zbVertex4D* vertices);

	void DoRender(const RenderCVarlistPtr& cvList, const RenderLayoutPtr& layout);

	// 背面消除
	bool CullFace(int fType, const float4& p1, const float4& p2, const float4& p3);

	// 根据 render_state 绘制原始三角形
	void DeviceDrawPrimitive(const RenderCVarlistPtr& cvList, const zbVertex4D& v1,const zbVertex4D& v2, const zbVertex4D& v3);

	// 画线
	void DeviceDrawLine(int x1, int y1, int x2, int y2, UINT32 c);

	// 画点
	void DevicePixel(int x, int y, UCHAR color);
private:
	// 创建第二表面
	LPDIRECTDRAWSURFACE7  DDrawCreateSurface(UINT nWidth, UINT  nHeight, int nSurfaceType = 0, int nColorType = 0);

	// 设置减裁区
	LPDIRECTDRAWCLIPPER DDrawAttachClipper(LPDIRECTDRAWSURFACE7 lpdds, int nNumRect, LPRECT clip_list);

	// 获取表面数据
	bool DDrawFillSurface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client = NULL);

	// 加载颜色文件
	void LoadPattleFormFile(char* szFileName);
private:
	static std::unique_ptr<DxGraphDevice> m_InstanceDevice;
	std::vector<VertexBuffer> m_VertexBuffercs;
	int m_nWidth;
	int m_nHeight;
	int m_nClientX;
	int m_nClientY;
	bool m_bWindows;
	HWND	m_hWnd;				// Win32 Window handle
	LPDIRECT3D9       m_d3d;
	LPDIRECT3DDEVICE9 m_device;

	LPDIRECTDRAW7 m_draw;
	int m_pixel_format;					// 显示像素格式
	DDSURFACEDESC2       m_ddsd;
	DDSCAPS2             m_ddscaps;//显示表面的功能控制设置
	LPDIRECTDRAWPALETTE  m_lpddpal;              // 调色板的指针
	LPDIRECTDRAWSURFACE7  m_lpddsprimary;	// 显示表面
	LPDIRECTDRAWSURFACE7  m_lpddsback;			// 缓存表面
	LPDIRECTDRAWCLIPPER  m_lpddclipper;          // dd clipper for back surface
	LPDIRECTDRAWCLIPPER  m_lpddclipperwin;       // dd clipper for window
	UCHAR                *m_szPrimaryBuffer;      // 显示表面视频缓存
	PALETTEENTRY        m_palette[256];         // color palette
	UCHAR                *m_szBackBuffer;         // 缓存表面视频缓存
	int m_nPrimaryPatch;
	int m_nBackPatch;
};
#endif//DX_GRAPHDEVICE_H_