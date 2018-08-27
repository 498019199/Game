// 2018年1月21日 19点14分从红龙那本总改成3D游戏编程技巧大师
#include "DxGraphDevice.h"
#include "../Util/UtilTool.h"
#include "../Container/macro.h"
#include "../Container/RenderVariable.h"
#include "../Render/RenderLayout.h"
#include "../Render/SceneManager.h"
#include "../Render/ICamera.h"
#include <boost/assert.hpp>
std::unique_ptr<DxGraphDevice> DxGraphDevice::m_InstanceDevice = nullptr;
extern HWND g_hwnd;
#define CLIP_FLOAT(v) static_cast<float>(v)
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }      //自定义一个SAFE_RELEASE()宏,便于COM资源的释放 
#define INIT_DDRAW_STRUCT(value) {memset(&value, 0, sizeof(value)); value.dwSize=sizeof(value);} 
enum TriType
{
	TRI_TYPE_FLAT_TOP = 1,
	TRI_TYPE_FLAT_BOTTOM = 2,
	TRI_TYPE_FLAT_MASK = 3,
	TRI_TYPE_FLAT_NORMAL = 4,
};

DxGraphDevice::DxGraphDevice(Context* pContext)
	:IEntityEx(pContext),m_pixel_format(0), m_szPrimaryBuffer(NULL), m_szBackBuffer(NULL),
	m_nPrimaryPatch(0), m_nBackPatch(0)
{
	m_InstanceDevice = std::unique_ptr<DxGraphDevice>(this);
}

DxGraphDevice::~DxGraphDevice()
{
	ShutDown();
}

DxGraphDevice* DxGraphDevice::Instance()
{
	if (nullptr != m_InstanceDevice)
	{
		return m_InstanceDevice.get();
	}

	return nullptr;
}

bool DxGraphDevice::OnInit()
{
	return true;
}

bool DxGraphDevice::OnShut()
{
	return true;
}

void DxGraphDevice::Update()
{

}

bool DxGraphDevice::InitDevice(HWND hwnd, UINT nWidth, UINT nHeight, UINT nClientX, UINT nClintY, bool bWindows)
{
	m_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (nullptr == m_d3d)
		return false;

	_D3DPRESENT_PARAMETERS_ d3dpp;
	::ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = nWidth;
	d3dpp.BackBufferHeight = nHeight;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = m_d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		&m_device);

	if (FAILED(hr))
	{
		return false;
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nClientX = nClientX;
	m_nClientY = nClintY;
	m_bWindows = bWindows;
	InitDraw(hwnd);
#ifdef _DEBUG
	D3DADAPTER_IDENTIFIER9 Adapter;  //定义一个D3DADAPTER_IDENTIFIER9结构体，用于存储显卡信息
	m_d3d->GetAdapterIdentifier(0, 0, &Adapter);
	trace_log("Current card model:%s", Adapter.Description);
	trace_log("Startup::%s", "DxGraphDevice");
#endif
	return true;
}

bool DxGraphDevice::InitDraw(HWND hwnd)
{
	if (FAILED(DirectDrawCreateEx(NULL, (void **)&m_draw, IID_IDirectDraw7, NULL)))
	{
		return false;
	}

	if (m_bWindows)
	{
		// 窗口模式
		if (FAILED(m_draw->SetCooperativeLevel(hwnd, DDSCL_NORMAL)))
		{
			return false;
		}
	} 
	else
	{
		// 全屏模式
		if (FAILED(m_draw->SetCooperativeLevel(hwnd, DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN |
			DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT | DDSCL_MULTITHREADED)))
		{
			return false;
		}

		if (FAILED(m_draw->SetDisplayMode(m_nWidth, m_nHeight, 16, 0, 0)))
		{
			return false;
		}
	}

	// 初始化表面描述体
	//INIT_DDRAW_STRUCT(m_ddsd);
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(ddsd);
	if (m_bWindows)
	{
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;
	} 
	else
	{
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
	}

	// 创建窗口表面
	m_draw->CreateSurface(&ddsd, &m_lpddsprimary, NULL);
	BOOST_ASSERT_MSG(m_lpddsprimary, "create first surface fair!");
	// 获取像素格式
	DDPIXELFORMAT ddpf;
	INIT_DDRAW_STRUCT(ddpf);
	m_lpddsprimary->GetPixelFormat(&ddpf);
	m_pixel_format = ddpf.dwRGBBitCount;

	trace_log("pixel format = %d", m_pixel_format);

	if (m_bWindows)
	{
		m_lpddsback = DDrawCreateSurface(m_nWidth, m_nHeight, DDSCAPS_SYSTEMMEMORY);
	} 
	else
	{
		m_ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(m_lpddsprimary->GetAttachedSurface(&m_ddscaps, &m_lpddsback)))
		{
			return false;
		}
	}

	// 创建调色板
	//现在将调色板附加到主表面上。
	if (0/*== DD_PIXEL_FORMAT8*/)
	{
#define MAX_COLORS_PALETTE  256
		memset(m_palette, 0, MAX_COLORS_PALETTE * sizeof(PALETTEENTRY));
		LoadPattleFormFile("PALDATA2.PAL");

		if (m_bWindows)
		{
			for (int i = 0; i < 10; ++i)
				m_palette[i].peFlags = m_palette[i + 246].peFlags = PC_EXPLICIT;

			if (FAILED(m_draw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE,
				m_palette, &m_lpddpal, NULL)))
			{
				return false;
			}
		} 
		else
		{
			if (FAILED(m_draw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256, 
				m_palette, &m_lpddpal, NULL)))
			{
				return false;
			}
		}

		m_lpddsprimary->SetPalette(m_lpddpal);
	} 

	// 置空表面
	if (m_bWindows)
	{
		DDrawFillSurface(m_lpddsback, 0);
	} 
	else
	{
		DDrawFillSurface(m_lpddsprimary, 0);
		DDrawFillSurface(m_lpddsback, 0);
	}

	// 设置窗口裁剪区域
	RECT screen_rect = { 0,0, (m_nWidth), (m_nHeight)};
	m_lpddclipper = DDrawAttachClipper(m_lpddsback, 1, &screen_rect);

	// 设置窗口裁剪模式
	if (m_bWindows)
	{
		if (FAILED(m_draw->CreateClipper(0, &m_lpddclipperwin, NULL)))
			return(0);

		if (FAILED(m_lpddclipperwin->SetHWnd(0, hwnd)))
			return(0);

		if (FAILED(m_lpddsprimary->SetClipper(m_lpddclipperwin)))
			return(0);
	}


	return true;
}

void DxGraphDevice::BeginRender()
{
	// 清理表面
	ClearSurface();
	// 锁住缓存表面
	DDrawBacklockSurface();
}

void DxGraphDevice::DoRender(const RenderCVarlistPtr& cvList, const RenderLayoutPtr& layout)
{
	auto ib = layout->GetIndexStream();
	auto vb = *(layout->GetVertexStream().get());
	for (uint32_t i = 0; i < ib.size(); ++i)
	{
		auto indiecs = ib[i];
		ClipPolys(cvList, vb[indiecs[0]], vb[indiecs[1]], vb[indiecs[2]]);
	}
}

int DxGraphDevice::CullingPolys(zbVertex4D* ps, int* encodes)
{
	auto camera = Context::Instance()->ActiveScene()->ActiveCamera();
	auto fZfactor = MathLib::Tan(camera->FOV() * 0.5f);
	auto fFar = camera->FarPlane();
	auto fNear = camera->NearPlane();
	int nInVertNum = 0;
	for (int i = 0; i < 3; ++i)
	{
		float ftest = ps[i].v.z() * fZfactor;
		if (ps[i].v.x() > ftest)
			encodes[i] = CLIP_X__MAX;
		else if (ps[i].v.x() < ftest)
			encodes[i] = CLIP_X__MIN;
		else
			encodes[i] = CLIP_X__MID;
	}
	if ((CLIP_X__MAX == encodes[0] && CLIP_X__MAX == encodes[1] && CLIP_X__MAX == encodes[2]) ||
		(CLIP_X__MIN == encodes[0] && CLIP_X__MIN == encodes[1] && CLIP_X__MIN == encodes[2]))
	{
		return 0;
	}

	for (int i = 0; i < 3; ++i)
	{
		if (ps[i].v.z() > fFar)
			encodes[i] |= CLIP_Z__MAX;
		else if (ps[i].v.z() < fNear)
			encodes[i] |= CLIP_Z__MIN;
		else
		{
			encodes[i] |= CLIP_Z__MID;
			nInVertNum++;
		}
	}

	return nInVertNum;
}

void DxGraphDevice::ClipPolys(const RenderCVarlistPtr& cvList, const zbVertex4D& v1, const zbVertex4D& v2, const zbVertex4D& v3)
{
	float4x4 mv;
	cvList->QueryByName("model_view")->Value(mv);
	zbVertex4D ps[4];
	ps[0] = v1, ps[1] = v2, ps[2] = v3;
	ps[0].v = MathLib::MatrixMulVector(v1.v, mv);
	ps[1].v = MathLib::MatrixMulVector(v2.v, mv);
	ps[2].v = MathLib::MatrixMulVector(v3.v, mv);

	// 剔除三角形
	int encodes[3] = {};
	auto nInVertNum = CullingPolys(ps, encodes);

	// 只对近裁剪面和远裁剪面裁剪
	if (2 == nInVertNum)
	{
	}
	else if (1 == nInVertNum)
	{
	}

	float4x4 mvp;
	cvList->QueryByName("mvp")->Value(mvp);
	ps[0].v = MathLib::MatrixMulVector(v1.v, mvp);
	ps[1].v = MathLib::MatrixMulVector(v2.v, mvp);
	ps[2].v = MathLib::MatrixMulVector(v3.v, mvp);
	if (1)
	{
		for (int i = 0; i < 3; ++i)
		{
			float rhw = 1.0f / ps[i].v.w();
			ps[i].v.x() = (ps[i].v.x() * rhw + 1.0f) * GetContext()->GetWidth() * 0.5f;
			ps[i].v.y() = (1.0f - ps[i].v.y() * rhw) *  GetContext()->GetHeight() * 0.5f;
			ps[i].v.z() = ps[i].v.z() * rhw;
			ps[i].v.w() = 1.0f;
		}
	}

	DeviceDrawPrimitive(cvList, ps[0], ps[1], ps[2]);
}

void DxGraphDevice::EndRender()
{
	// 解锁缓存表面
	DDrawBackUnlockSurface();
	// 主表面交换缓存表面
	DDrawFilp();
}

VertexBuffer DxGraphDevice::MakeCreationVertexBuffer(const VertexVec& vb)
{
	VertexBuffer buffer = MakeSharedPtr<VertexVec>();
	buffer->resize(vb.size());
	memcpy(&(buffer.get()->at(0)), &vb[0], sizeof(zbVertex4D) * vb.size());

	int nIndex = m_VertexBuffercs.size();
	m_VertexBuffercs.push_back(buffer);
 	return m_VertexBuffercs[nIndex];
}

void DxGraphDevice::ShutDown()
{
	SAFE_RELEASE(m_d3d);
	SAFE_RELEASE(m_device);

	SAFE_RELEASE(m_lpddpal);
	SAFE_RELEASE(m_lpddsprimary);
	SAFE_RELEASE(m_lpddsback);
}

LPDIRECTDRAWSURFACE7  DxGraphDevice::DDrawCreateSurface(UINT nWidth, UINT nHeight, int nSurfaceType /*= 0*/, int nColorType /*= 0*/)
{
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWSURFACE7 lpdds;

	INIT_DDRAW_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.dwWidth = nWidth;
	ddsd.dwHeight = nHeight;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | nSurfaceType;

	if (FAILED(m_draw->CreateSurface(&ddsd, &lpdds, NULL)))
	{
		return NULL;
	}

	// 设置颜色键
	DDCOLORKEY color_key; 
	color_key.dwColorSpaceLowValue = nColorType;
	color_key.dwColorSpaceHighValue = nColorType;
	lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);

	return lpdds;
}

LPDIRECTDRAWCLIPPER DxGraphDevice::DDrawAttachClipper(LPDIRECTDRAWSURFACE7 lpdds, int nNumRect, LPRECT clip_list)
{
	LPDIRECTDRAWCLIPPER lpddclipper;
	LPRGNDATA region_data;

	// 第一个创建direct draw减裁区
	if (FAILED(m_draw->CreateClipper(0, &lpddclipper, NULL)))
	{	
		return NULL;
	}

	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER) + nNumRect * sizeof(RECT));
	memcpy(region_data->Buffer, clip_list, sizeof(RECT)*nNumRect);
	region_data->rdh.dwSize = sizeof(RGNDATAHEADER);
	region_data->rdh.iType = RDH_RECTANGLES;
	region_data->rdh.nCount = nNumRect;
	region_data->rdh.nRgnSize = nNumRect * sizeof(RECT);
	region_data->rdh.rcBound.left = 64000;
	region_data->rdh.rcBound.top = 64000;
	region_data->rdh.rcBound.right = -64000;
	region_data->rdh.rcBound.bottom = -64000;

	// find bounds of all clipping regions
	for (int index = 0; index < nNumRect; index++)
	{
		// test if the next rectangle unioned with the current bound is larger
		if (clip_list[index].left < region_data->rdh.rcBound.left)
			region_data->rdh.rcBound.left = clip_list[index].left;

		if (clip_list[index].right > region_data->rdh.rcBound.right)
			region_data->rdh.rcBound.right = clip_list[index].right;

		if (clip_list[index].top < region_data->rdh.rcBound.top)
			region_data->rdh.rcBound.top = clip_list[index].top;

		if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
			region_data->rdh.rcBound.bottom = clip_list[index].bottom;
	}

	// now we have computed the bounding rectangle region and set up the data
	// now let's set the clipping list
	if (FAILED(lpddclipper->SetClipList(region_data, 0)))
	{
		// release memory and return error
		free(region_data);
		return NULL;
	}

	if (FAILED(lpdds->SetClipper(lpddclipper)))
	{
		// release memory and return error
		free(region_data);
		return NULL;
	} 
	  // all is well, so release memory and send back the pointer to the new clipper
	free(region_data);
	return lpddclipper;
}

bool DxGraphDevice::DDrawFillSurface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client /*= NULL*/)
{
	DDBLTFX ddbltfx;
	INIT_DDRAW_STRUCT(ddbltfx);
	ddbltfx.dwFillColor = color;

	// 读取表面数据
	lpdds->Blt(client, 
		NULL,               
		NULL,       
		DDBLT_COLORFILL | DDBLT_WAIT,                   
		&ddbltfx);  
	return true;
}

void DxGraphDevice::LoadPattleFormFile(char* szFileName)
{
	FILE *fp;
	if (NULL == (fp = fopen(szFileName, "r")))
	{
		return;
	}

	for (int i = 0; i < MAX_COLORS_PALETTE; ++i)
	{
		fscanf(fp, "%c %c %c %c", &m_palette[i].peRed,
			&m_palette[i].peGreen,
			&m_palette[i].peBlue,
			&m_palette[i].peFlags);
	}

	fclose(fp);
}



UCHAR* DxGraphDevice::DDrawBacklockSurface()
{
	if (m_szBackBuffer)
	{
		return m_szBackBuffer;
	}

	INIT_DDRAW_STRUCT(m_ddsd);
	m_lpddsback->Lock(NULL, &m_ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	m_szBackBuffer = (UCHAR *)m_ddsd.lpSurface;
	m_nBackPatch = m_ddsd.lPitch;
	return m_szBackBuffer;
}


void DxGraphDevice::DDrawBackUnlockSurface()
{
	if (!m_szBackBuffer)
	{
		return;
	}

	m_lpddsback->Unlock(NULL);
	m_szBackBuffer = NULL;
	m_nBackPatch = 0;
}

void DxGraphDevice::DDrawFilp()
{
	if (m_szBackBuffer || m_szPrimaryBuffer)
	{
		return;
	}

	if (m_bWindows)
	{
		RECT    dest_rect;
		GetWindowRect(g_hwnd, &dest_rect);

		// compute the destination rectangle
		dest_rect.left += m_nClientX;
		dest_rect.top += m_nClientY;
		dest_rect.right = dest_rect.left + m_nWidth - 1;
		dest_rect.bottom = dest_rect.top + m_nHeight - 1;

		if (FAILED(m_lpddsprimary->Blt(&dest_rect, m_lpddsback, NULL, DDBLT_WAIT, NULL)))
			return;
	}
	else
	{
		while (FAILED(m_lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
	}
}

void DxGraphDevice::ClearSurface()
{
	DDrawFillSurface(m_lpddsback, 0);
}

void DxGraphDevice::DrawTextGDI(char* szText, int x, int y, int color)
{
	HDC xdc;
	if (FAILED(m_lpddsback->GetDC(&xdc)))
	{
		return;
	}

	SetTextColor(xdc, RGB(m_palette[color].peRed, m_palette[color].peGreen, m_palette[color].peBlue));

	// set background mode to transparent so black isn't copied
	SetBkMode(xdc, TRANSPARENT);

	// draw the text a
	TextOut(xdc, x, y, szText, strlen(szText));

	// release the dc
	m_lpddsback->ReleaseDC(xdc);
}

void DxGraphDevice::DrawTextGDI(char* szText, int x, int y, COLORREF color)
{
	HDC xdc;
	if (FAILED(m_lpddsback->GetDC(&xdc)))
	{
		return;
	}

	SetTextColor(xdc, color);

	// set background mode to transparent so black isn't copied
	SetBkMode(xdc, TRANSPARENT);

	// draw the text a
	TextOut(xdc, x, y, szText, strlen(szText));

	// release the dc
	m_lpddsback->ReleaseDC(xdc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// A = A(p1) + A(p2) + A(p3); 
bool DxGraphDevice::CullFace(int fType, const float4& p1, const float4& p2, const float4& p3)
{
	float fA = .0f;
	fA += p1.x() * p2.y() - p2.x() * p1.y();
	fA += p2.x() * p3.y() - p3.x() * p2.y();
	fA += p3.x() * p1.y() - p1.x() * p3.y();

	switch (fType)
	{
	case CULL_FACE_FRONT:
		if (fA >= 0.0f) return false;
		break;
	case CULL_FACE_BACK:
		if (fA <= 0.0f) return false;
		break;
	}

	return true;
}

void DxGraphDevice::DeviceDrawPrimitive(const RenderCVarlistPtr& cvList, const zbVertex4D& v1, const zbVertex4D& v2, const zbVertex4D& v3)
{
	// 背面剔除
	int nCullMode;
	cvList->QueryByName("cull_mode")->QueryVar().Value(nCullMode);
	if (CullFace(nCullMode, v1.v, v2.v, v3.v))
	{
		return;
	}

	zbVertex4D vertice[3] = { v1, v2, v3 };
	for (int i = 0; i < 3; i++)
	{
		zbVertex4D vertex = vertice[i];
	}
	//
	if (0)
	{
	} 
	else
	{
		DeviceDrawLine(int(v1.v.x()), int(v1.v.y()), int(v2.v.x()), int(v2.v.y()), 1920);
		DeviceDrawLine(int(v1.v.x()), int(v1.v.y()), int(v3.v.x()), int(v3.v.y()), 1920);
		DeviceDrawLine(int(v3.v.x()), int(v3.v.y()), int(v2.v.x()), int(v2.v.y()), 1920);
	}
}

void DxGraphDevice::DeviceDrawLine(int x1, int y1, int x2, int y2, UINT32 c)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	int ux = ((dx > 0) << 1) - 1;
	int uy = ((dy > 0) << 1) - 1;
	int x = x1, y = y1, eps;

	eps = 0; 
	dx = MathLib::Abs(dx); 
	dy = MathLib::Abs(dy);
	if (dx > dy)
	{
		for (x = x1; x != x2 + ux; x += ux)
		{
			DevicePixel(x, y, c);
			eps += dy;
			if ((eps << 1) >= dx) {
				y += uy;
				eps -= dx;
			}
		}
	}
	else 
	{
		for (y = y1; y != y2 + uy; y += uy)
		{
			DevicePixel(x, y, c);
			eps += dx;
			if ((eps << 1) >= dy) 
			{
				x += ux;
				eps -= dy;
			}
		}
	}
}

void DxGraphDevice::DevicePixel(int x, int y, UCHAR color)
{
	uint32_t *pView = reinterpret_cast<uint32_t*>(m_szBackBuffer);
	int nPitch2 = m_nBackPatch >> 2;
	if (x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
	{
		pView[y * nPitch2 + x] = color;
	}
}
