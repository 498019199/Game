// 2018年1月21日 19点14分从红龙那本总改成3D游戏编程技巧大师
#include "DxGraphDevice.h"
#include "../Platform/Window/Window.h"
#include "../Util/UtilTool.h"
#include "../Container/macro.h"
#include "../Container/RenderVariable.h"
#include "../Render/RenderLayout.h"
#include "../Render/IScene.h"
#include "../Render/ICamera.h"
#include "../Render/ITexture.h"
#include "../Math/Math.h"
#include <boost/assert.hpp>
std::unique_ptr<DxGraphDevice> DxGraphDevice::m_InstanceDevice = nullptr;

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

enum ClipMask : uint32_t
{
	CLIPMASK_X_POS = 0x1,
	CLIPMASK_X_NEG = 0x2,
	CLIPMASK_Y_POS = 0x4,
	CLIPMASK_Y_NEG = 0x8,
	CLIPMASK_Z_POS = 0x10,
	CLIPMASK_Z_NEG = 0x20,
};

static float4 Frustum[6] = 
{
	float4(-1.0f, 0.0f, 0.0f, 1.0f),
	float4(1.0f, 0.0f, 0.0f, 1.0f),
	float4(0.0f, -1.0f, 0.0f, 1.0f),
	float4(0.0f, 1.0f, 0.0f, 1.0f),
	float4(0.0f, 0.0f, -1.0f, 1.0f),
	float4(0.0f, 0.0f, 1.0f, 0.0f),
};

struct edge_t { zbVertex4D v, v1, v2; };
struct trapezoid_t { float top, bottom; edge_t left, right; };
struct scanline_t { zbVertex4D v, step; int x, y, w; };
struct v2f{float4 pos;float2 texcoord;Color color;float4 normal;float4 storage0;float4 storage1;float4 storage2;};

DxGraphDevice::DxGraphDevice(Context* pContext)
	:IEntityEx(pContext),m_pixel_format(0), m_szPrimaryBuffer(NULL), m_szBackBuffer(NULL),
	m_nPrimaryPatch(0), m_nBackPatch(0)
{
	m_hWnd = pContext->Instance()->AppInstance()->GetMainWin()->GetHWnd();
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

	m_DeepZbuffer = NEW float[m_nWidth * m_nHeight];
	BOOST_ASSERT(m_DeepZbuffer);
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
	float4x4 mv;
	cvList->QueryByName("mvp")->Value(mv);
	auto vb = *(layout->GetVertexStream().get());
	zbVertex4D vertices[64 * 3];
	for (uint32_t i = 0; i < vb.size() ; i+=3)
	{
		vertices[0].v = MathLib::MatrixMulVector(vb[i].v, mv);
		vertices[1].v = MathLib::MatrixMulVector(vb[i + 1].v, mv);
		vertices[2].v = MathLib::MatrixMulVector(vb[i + 2].v, mv);

		vertices[0].t = vb[i].t;
		vertices[1].t = vb[i + 1].t;
		vertices[2].t = vb[i + 2].t;

		vertices[0].color = vb[i].color;
		vertices[1].color = vb[i + 1].color;
		vertices[2].color = vb[i + 2].color;
		DrawTriangle(cvList, vertices);
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

float4 DxGraphDevice::ViewportTransform(const float4& vert)
{
	float rhw = 1.0f / vert.w();
	float x = (vert.x() * rhw + 1.0f) * m_nWidth* 0.5f;
	float y = (1.0f - vert.y() * rhw) *  m_nHeight * 0.5f;
	float z = vert.z() * rhw;
	float w = 1.0f;

	return float4(x, y, z, w);
}


void DxGraphDevice::ViewportTransformReverse(float4 y, const float4& x, float w, float width, float height)
{
	y.x() = (x.x() * 2 / width - 1.0f) * w;
	y.y() = (1.0f - x.y() * 2 / height) * w;
	y.z() = x.z() * w;
	y.w() = w;
}

void DxGraphDevice::DrawTriangle(const RenderCVarlistPtr& cvList, zbVertex4D* vertices)
{
	uint32_t andClipMask = 0;
	uint32_t clipMask = 0;
	for (size_t i = 0; i < 3; ++i)
	{
		uint32_t vertexClipMask = 0;
		if (vertices[i].v.x() > vertices[i].v.w())
			vertexClipMask |= CLIPMASK_X_POS;
		if (vertices[i].v.x() < -vertices[i].v.w())
			vertexClipMask |= CLIPMASK_X_NEG;
		if (vertices[i].v.y() > vertices[i].v.w())
			vertexClipMask |= CLIPMASK_Y_POS;
		if (vertices[i].v.y() < -vertices[i].v.w())
			vertexClipMask |= CLIPMASK_Y_NEG;
		if (vertices[i].v.z() > vertices[i].v.w())
			vertexClipMask |= CLIPMASK_Z_POS;
		if (vertices[i].v.z() < 0.0f)
			vertexClipMask |= CLIPMASK_Z_NEG;

		clipMask |= vertexClipMask;
		if (!i)
			andClipMask = vertexClipMask;
		else
			andClipMask &= vertexClipMask;
	}

	if (andClipMask)
		return;

	bool triangles[64] = { 0 };
	triangles[0] = true;
	if (!clipMask)
	{
		DrawPrimitive(cvList, vertices, triangles, 1);
	}
	else
	{
		uint32_t numTriangles = 1;
		if (clipMask & CLIPMASK_X_POS)
			ClipVertices(Frustum[0], vertices, triangles, numTriangles);
		if (clipMask & CLIPMASK_X_NEG)
			ClipVertices(Frustum[1], vertices, triangles, numTriangles);
		if (clipMask & CLIPMASK_Y_POS)
			ClipVertices(Frustum[2], vertices, triangles, numTriangles);
		if (clipMask & CLIPMASK_Y_NEG)
			ClipVertices(Frustum[3], vertices, triangles, numTriangles);
		if (andClipMask & CLIPMASK_Z_POS)
			ClipVertices(Frustum[4], vertices, triangles, numTriangles);
		if (andClipMask & CLIPMASK_Z_NEG)
			ClipVertices(Frustum[5], vertices, triangles, numTriangles);

		DrawPrimitive(cvList, vertices, triangles, numTriangles);
	}
}

void DxGraphDevice::DrawPrimitive(const RenderCVarlistPtr& cvList, zbVertex4D* vertices, bool(&triangles)[64], uint32_t numTriangles)
{
	int nCullMode;
	cvList->QueryByName("cull_mode")->QueryVar().Value(nCullMode);
	for (uint32_t i = 0; i < numTriangles; ++i)
	{
		zbVertex4D projected[3];
		v2f vfs[3];
		for (uint32_t j = 0; j < 3; ++j)
		{
			vfs[j].texcoord = vertices[i + j].t;
			vfs[j].color = vertices[i + j].color;
		}

		if (triangles[i])
		{
			uint32_t index = i * 3;
			projected[0].v = ViewportTransform(vertices[index].v);
			projected[1].v = ViewportTransform(vertices[index + 1].v);
			projected[2].v = ViewportTransform(vertices[index + 2].v);

			// 背面剔除
			IF_CONTINUE(!CullFace(nCullMode, projected[index].v, projected[index + 1].v, projected[index + 2].v));
			DrawTriangle2D(cvList, projected, vfs);
		}
	}
}

// 对三角形点排序，分割成平顶三角形，平底三角形
int DxGraphDevice::TrapezoidInitTriangle(trapezoid_t *trap, const zbVertex4D *p1, const zbVertex4D *p2, const zbVertex4D *p3)
{
	// 直线
	if (p1->v.y() == p2->v.y() && p1->v.y() == p3->v.y()) return 0;
	if (p1->v.x() == p2->v.x() && p1->v.x() == p3->v.x()) return 0;

	// 对点进行排序，p1 < p2 < p3
	if (p1->v.y() > p2->v.y()) std::swap(p1, p2);
	if (p1->v.y() > p3->v.y()) std::swap(p1, p3); 
	if (p2->v.y() > p3->v.y()) std::swap(p2, p3);

	// 区分三角形
	if (p1->v.y() == p2->v.y())
	{
		if (p1->v.x() > p2->v.x()) std::swap(p1, p2);
		//TRI_TYPE_FLAT_TOP
		trap[0].top = p1->v.y();
		trap[0].bottom = p3->v.y();
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p3;
		trap[0].right.v1 = *p2;
		trap[0].right.v2 = *p3;
		return (trap[0].top < trap[0].bottom) ? 1 : 0;
	}
	else if (p2->v.y() == p3->v.y())
	{
		//TRI_TYPE_FLAT_BOTTOM
		if (p2->v.x() > p3->v.x()) std::swap(p2, p3);
		trap[0].top = p1->v.y();
		trap[0].bottom = p3->v.y();
		trap[0].left.v1 = *p1;
		trap[0].left.v2 = *p2;
		trap[0].right.v1 = *p1;
		trap[0].right.v2 = *p3;
		return (trap[0].top < trap[0].bottom) ? 1 : 0;
	}
	else
	{
		//TRI_TYPE_FLAT_NORMAL
		trap[0].top = p1->v.y();
		trap[0].bottom = p2->v.y();
		trap[1].top = p2->v.y();
		trap[1].bottom = p3->v.y();
		float k = (p3->v.y() - p1->v.y()) / (p2->v.y() - p1->v.y());
		float x = p1->v.x() + (p2->v.x() - p1->v.x()) * k;

		if (x <= p3->v.x()) 
		{		
			trap[0].left.v1 = *p1;
			trap[0].left.v2 = *p2;
			trap[0].right.v1 = *p1;
			trap[0].right.v2 = *p3;
			trap[1].left.v1 = *p2;
			trap[1].left.v2 = *p3;
			trap[1].right.v1 = *p1;
			trap[1].right.v2 = *p3;
		}
		else 
		{					
			trap[0].left.v1 = *p1;
			trap[0].left.v2 = *p3;
			trap[0].right.v1 = *p1;
			trap[0].right.v2 = *p2;
			trap[1].left.v1 = *p1;
			trap[1].left.v2 = *p3;
			trap[1].right.v1 = *p2;
			trap[1].right.v2 = *p3;
		}
	}

	return 2;
}

void DxGraphDevice::VertexInterp(zbVertex4D& v, const zbVertex4D& v1, const zbVertex4D& v2, float t)
{
	v.v = MathLib::Lerp(v1.v, v2.v, t);
	v.n = MathLib::Lerp(v1.n, v2.n, t);
	v.t = MathLib::Lerp(v1.t, v2.t, t);
	v.color = MathLib::Lerp(v1.color, v2.color, t);
}

void DxGraphDevice::VertexAdd(zbVertex4D& y, const zbVertex4D& x)
{
	y.v += x.v;
	y.t += x.t;
	y.color += x.color;
}

// 根据左右两边的端点，初始化计算出扫描线的起点和步长
void DxGraphDevice::TrapezoidToScanline(trapezoid_t *traps, scanline_t& scanline, int y)
{
	float s1 = traps->left.v2.v.y() - traps->left.v1.v.y();
	float s2 = traps->right.v2.v.y() - traps->right.v1.v.y();
	float t1 = (y - traps->left.v1.v.y()) / s1;
	float t2 = (y - traps->right.v1.v.y()) / s2;
	VertexInterp(traps->left.v, traps->left.v1, traps->left.v2, t1);
	VertexInterp(traps->right.v, traps->right.v1, traps->right.v2, t2);

	float width = traps->right.v.v.x() - traps->left.v.v.x();
	scanline.x = MathLib::RoundToInt(traps->left.v.v.x() + 0.5f);
	scanline.w = MathLib::RoundToInt(traps->right.v.v.x() + 0.5f) - scanline.x;
	if (traps->left.v.v.x() >= traps->right.v.v.x())
		scanline.w = 0;

	scanline.y = y;
	scanline.v = traps->left.v;

	float inv = 1.0f / width;
	scanline.step.v = (traps->right.v.v - traps->left.v.v) * inv;
	scanline.step.t = (traps->right.v.t - traps->left.v.t) * inv;
	scanline.step.color = (traps->right.v.color - traps->left.v.color) * inv;
}

void DxGraphDevice::DrawScanline(const RenderCVarlistPtr& cvList, scanline_t& scanline, zbVertex4D* vertices, v2f *vfs)
{
	int y = scanline.y;
	int x = scanline.x;
	int count = scanline.w;
	int width = m_nWidth;
	uint32_t *pView = reinterpret_cast<uint32_t*>(m_szBackBuffer);
	for (; count > 0 && x < width; x++, count--)
	{
		float rhw = scanline.v.v.w();
		if (m_DeepZbuffer == NULL || rhw >= m_DeepZbuffer[y*width + x])
			if (m_DeepZbuffer != NULL)
				m_DeepZbuffer[y*width + x] = rhw;

		float4 interpos = scanline.v.v;
		Color color;
		v2f vf;
		float4 barycenter;
		float w = 1.0f / scanline.v.v.w();

		ViewportTransformReverse(interpos, interpos, w, float(m_nWidth), float(m_nHeight));
		MathLib::ComputeBarycentricCoords3d(barycenter, vertices[0].v, vertices[1].v, vertices[2].v, interpos);
		V2fInterpolating(vf, vfs[0], vfs[1], vfs[2], barycenter.x(), barycenter.y(), barycenter.z());

		FragShader(vf, cvList, color);
		uint32_t _1 = color.ARGB();
		if (0 == _1) 
			_1 = 1920;
		pView[y*width + x] = _1;
		VertexAdd(scanline.v, scanline.step);
	}
}

void DxGraphDevice::FragShader(v2f& vf, const RenderCVarlistPtr& cvList, Color& color)
{
	float2 tex = vf.texcoord;

	float4 albedo_clr, diffuse_clr, specular_clr;
	float shininess_clr;
	cvList->QueryByName("albedo_clr")->Value(albedo_clr);
	cvList->QueryByName("diffuse_clr")->Value(diffuse_clr);
	cvList->QueryByName("specular_clr")->Value(specular_clr);
	cvList->QueryByName("shininess_clr")->Value(shininess_clr);

	TexturePtr pTex;
	cvList->QueryByName("albedo_tex")->Value(pTex);
	if (pTex && 0 == pTex->GetName().compare("Dragon_meshml/022green.dds"))
	{
		color = pTex->GetTextureColor(tex.x(), tex.y(), vf.pos.w(), 15);
	}
	else
	{
		float4 tmp = albedo_clr;
		tmp += diffuse_clr;
		tmp += specular_clr;

		color.a() = 1;
		color.r() = tmp.x();
		color.g() = tmp.y();
		color.b() = tmp.z();
	}

	if (0 == color.ARGB())
	{
		color = vf.color;
	}
}

template<typename T>
void Interpolating(T& dest, const T& src1, const T& src2, const T& src3, float a, float b, float c)
{
	dest = T::Zero();
	T each = src1;
	each *= a;
	dest += each;
	each = src2;
	each *= b;
	dest += each;
	each = src3;
	each *= c;
	dest += each;
}

void DxGraphDevice::V2fInterpolating(v2f& dest, const v2f& src1, const v2f& src2, const v2f& src3, float a, float b, float c)
{
	Interpolating(dest.pos, src1.pos, src2.pos, src3.pos, a, b, c);
	Interpolating(dest.color, src1.color, src2.color, src3.color, a, b, c);
	Interpolating(dest.texcoord, src1.texcoord, src2.texcoord, src3.texcoord, a, b, c);
	Interpolating(dest.normal, src1.normal, src2.normal, src3.normal, a, b, c);
	Interpolating(dest.storage0, src1.storage0, src2.storage0, src3.storage0, a, b, c);
	Interpolating(dest.storage1, src1.storage1, src2.storage1, src3.storage1, a, b, c);
	Interpolating(dest.storage2, src1.storage2, src2.storage2, src3.storage2, a, b, c);
}

void DxGraphDevice::DrawTriangle2D(const RenderCVarlistPtr& cvList, zbVertex4D* vertices, v2f *vfs)
{
	if (RENDER_TYPE_WIREFRAME & Context::Instance()->GetRenderType())
	{
		zbVertex4D v1 = vertices[0], v2 = vertices[1], v3 = vertices[2];
		DeviceDrawLine(int(v1.v.x()), int(v1.v.y()), int(v2.v.x()), int(v2.v.y()), 1920);
		DeviceDrawLine(int(v1.v.x()), int(v1.v.y()), int(v3.v.x()), int(v3.v.y()), 1920);
		DeviceDrawLine(int(v3.v.x()), int(v3.v.y()), int(v2.v.x()), int(v2.v.y()), 1920);
	} 
	if (RENDER_TYPE_TEXTURE & Context::Instance()->GetRenderType())
	{
		trapezoid_t traps[2];
		int n = TrapezoidInitTriangle(traps, &vertices[0], &vertices[1], &vertices[2]);
		for (int i = 0; i < n; ++i)
		{
			int top = MathLib::RoundToInt(traps->top);
			int bottom = MathLib::RoundToInt(traps->bottom);
			for (int j = top; j < bottom; j++)
			{
				IF_BREAK(j > m_nHeight && j < 0);
				// 插值计算
				scanline_t scanline;
				TrapezoidToScanline(traps, scanline, j);
				DrawScanline(cvList, scanline, vertices, vfs);
			}
		}
	}
}

void DxGraphDevice::ClipVertices(float4 plane, zbVertex4D* vertices, bool *triangles, uint32_t& numTriangles)
{
	uint32_t num = numTriangles;
	for (uint32_t i = 0; i < num; ++i)
	{
		if (triangles[i])
		{
			uint32_t nIndex = i * 3;
			float f0 = MathLib::Dot(plane, vertices[nIndex].v);
			float f1 = MathLib::Dot(plane, vertices[nIndex + 1].v);
			float f2 = MathLib::Dot(plane, vertices[nIndex + 2].v);

			// If all vertices behind the plane, reject triangle
			if (f0 < 0.0f && f1 < 0.0f && f2 < 0.0f)
			{
				triangles[i] = false;
				continue;
			}
			// If 2 vertices behind the plane, create a new triangle in-place
			else if (f0 < 0.0f && f1 < 0.0f)
			{
				vertices[nIndex] = ClipEdge(vertices[nIndex], vertices[nIndex + 2], f0, f2);
				vertices[nIndex + 1] = ClipEdge(vertices[nIndex + 1], vertices[nIndex + 2], f1, f2);
			}
			else if (f0 < 0.0f && f2 < 0.0f)
			{
				vertices[nIndex] = ClipEdge(vertices[nIndex], vertices[nIndex + 1], f0, f1);
				vertices[nIndex + 2] = ClipEdge(vertices[nIndex + 2], vertices[nIndex + 1], f2, f1);
			}
			else if (f1 < 0.0f && f2 < 0.0f)
			{
				vertices[nIndex + 1] = ClipEdge(vertices[nIndex + 1], vertices[nIndex], f1, f0);
				vertices[nIndex + 2] = ClipEdge(vertices[nIndex + 2], vertices[nIndex], f2, f0);
			}
			// 1 vertex behind the plane: create one new triangle, and modify one in-place
			else if (f0 < 0.0f)
			{
				unsigned newIdx = numTriangles * 3;
				triangles[numTriangles] = true;
				++numTriangles;

				vertices[newIdx] = ClipEdge(vertices[nIndex], vertices[nIndex + 2], f0, f2);
				vertices[newIdx + 1] = vertices[nIndex] = ClipEdge(vertices[nIndex], vertices[nIndex + 1], f0, f1);
				vertices[newIdx + 2] = vertices[nIndex + 2];
			}
			else if (f1 < 0.0f)
			{
				unsigned newIdx = numTriangles * 3;
				triangles[numTriangles] = true;
				++numTriangles;

				vertices[newIdx + 1] = ClipEdge(vertices[nIndex + 1], vertices[nIndex], f1, f0);
				vertices[newIdx + 2] = vertices[nIndex + 1] = ClipEdge(vertices[nIndex + 1], vertices[nIndex + 2], f1, f2);
				vertices[newIdx] = vertices[nIndex];
			}
			else if (f2 < 0.0f)
			{
				unsigned newIdx = numTriangles * 3;
				triangles[numTriangles] = true;
				++numTriangles;

				vertices[newIdx + 2] = ClipEdge(vertices[nIndex + 2], vertices[nIndex + 1], f2, f1);
				vertices[newIdx] = vertices[nIndex + 2] = ClipEdge(vertices[nIndex + 2], vertices[nIndex], f2, f0);
				vertices[newIdx + 1] = vertices[nIndex + 1];
			}
		}
	}
}

zbVertex4D DxGraphDevice::ClipEdge(const zbVertex4D& v0, const zbVertex4D& v1, float f0, float f1) const
{
	zbVertex4D tmp;
	float t = f0 / (f0 - f1);
	tmp.v = v0.v + t * (v1.v - v0.v);
	tmp.t = v0.t + t * (v1.t - v0.t);
	tmp.color = v0.color + t * (v1.color - v0.color);
	return tmp;
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

	delete[] m_DeepZbuffer;
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
		GetWindowRect(m_hWnd, &dest_rect);

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
	case CULL_FACE_BACK:
		if (fA >= 0.0f) return false;
		break;
	case CULL_FACE_FRONT:
		if (fA <= 0.0f) return false;
		break;
	}

	return true;
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
