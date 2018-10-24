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

	// �����ʾ��
	int GetWidth() const { return m_nWidth;  }

	// �����ʾ��
	int GetHeight() const { return m_nHeight;  }

	// ��ס��������
	UCHAR* DDrawBacklockSurface();

	// ������������
	void DDrawBackUnlockSurface();

	// �����������
	void DDrawFilp();

	// �������
	void ClearSurface();
	//************************************
	// Qualifier:											��Ļ���
	// Parameter: char * szText					�������
	// Parameter: int x								������ʼ����x
	// Parameter: int y								������ʼ����y
	// Parameter: COLORREF color			������ɫ
	//************************************
	void DrawTextGDI(char* szText, int x, int y, COLORREF color);
	void DrawTextGDI(char* szText, int x, int y, int color);

	// �ͷ�directX
	void ShutDown();

	//************************************
	// Qualifier:
	// Parameter: HWND hwnd				���ھ��
	// Parameter: UINT nWidth				��Ļ��
	// Parameter: UINT nHeight			��Ļ��
	//************************************
	bool InitDevice(HWND hwnd, UINT  nWidth, UINT  nHeight, UINT nClientX, UINT nClintY, bool bWindows);

	// ��ʼ��directdraw
	bool InitDraw(HWND hwnd);

	void BeginRender();

	void EndRender();

	VertexBuffer MakeCreationVertexBuffer(const VertexVec& vb);

	// �޳�������
	int CullingPolys(zbVertex4D* ps, int* encodes);

	// ת������Ļ����
	float4 ViewportTransform(const float4& vert);
	void ViewportTransformReverse(float4 y, const float4& x, float w, float width, float height);;

	void DrawTriangle(const RenderCVarlistPtr& cvList, zbVertex4D* vertices);
	void DrawPrimitive(const RenderCVarlistPtr& cvList, zbVertex4D* vertices, bool(&triangles)[64], uint32_t numTriangles);
	void DrawTriangle2D(const RenderCVarlistPtr& cvList, zbVertex4D* vertices, v2f *vfs);
	
	int TrapezoidInitTriangle(trapezoid_t *trap, const zbVertex4D *p1, const zbVertex4D *p2, const zbVertex4D *p3);
	void VertexInterp(zbVertex4D& v, const zbVertex4D& v1, const zbVertex4D& v2, float t);
	void VertexAdd(zbVertex4D& y, const zbVertex4D& x);
	void TrapezoidToScanline(trapezoid_t *traps, scanline_t& scanline, int y);
	
	void DrawScanline(const RenderCVarlistPtr& cvList, scanline_t& scanline, zbVertex4D* vertices, v2f *vfs);
	void FragShader(v2f& vf, const RenderCVarlistPtr& cvList, Color& color);
	void V2fInterpolating(v2f& dest, const v2f& src1, const v2f& src2, const v2f& src3, float a, float b, float c);

	void ClipVertices(float4 plane, zbVertex4D* vertices, bool *triangles, uint32_t& numTriangles);
	zbVertex4D ClipEdge(const zbVertex4D& v0, const zbVertex4D& v1, float f0, float f1) const;
	
	void DoRender(const RenderCVarlistPtr& cvList, const RenderLayoutPtr& layout);

	// ��������
	bool CullFace(int fType, const float4& p1, const float4& p2, const float4& p3);

	// ����
	void DeviceDrawLine(int x1, int y1, int x2, int y2, UINT32 c);

	// ����
	void DevicePixel(int x, int y, UCHAR color);
private:
	// �����ڶ�����
	LPDIRECTDRAWSURFACE7  DDrawCreateSurface(UINT nWidth, UINT  nHeight, int nSurfaceType = 0, int nColorType = 0);

	// ���ü�����
	LPDIRECTDRAWCLIPPER DDrawAttachClipper(LPDIRECTDRAWSURFACE7 lpdds, int nNumRect, LPRECT clip_list);

	// ��ȡ��������
	bool DDrawFillSurface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client = NULL);

	// ������ɫ�ļ�
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
	int m_pixel_format;					// ��ʾ���ظ�ʽ
	DDSURFACEDESC2       m_ddsd;
	DDSCAPS2             m_ddscaps;//��ʾ����Ĺ��ܿ�������
	LPDIRECTDRAWPALETTE  m_lpddpal;              // ��ɫ���ָ��
	LPDIRECTDRAWSURFACE7  m_lpddsprimary;	// ��ʾ����
	LPDIRECTDRAWSURFACE7  m_lpddsback;			// �������
	LPDIRECTDRAWCLIPPER  m_lpddclipper;          // dd clipper for back surface
	LPDIRECTDRAWCLIPPER  m_lpddclipperwin;       // dd clipper for window
	UCHAR                *m_szPrimaryBuffer;      // ��ʾ������Ƶ����
	PALETTEENTRY        m_palette[256];         // color palette
	UCHAR                *m_szBackBuffer;         // ���������Ƶ����
	float *m_DeepZbuffer;             // ��Ȼ���
	int m_nPrimaryPatch;
	int m_nBackPatch;
};
#endif//DX_GRAPHDEVICE_H_