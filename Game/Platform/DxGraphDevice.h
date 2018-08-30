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

	// ��������
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

	void ClipPolys(const RenderCVarlistPtr& cvList, const zbVertex4D& v1, const zbVertex4D& v2, const zbVertex4D& v3);
	
	void DoRender(const RenderCVarlistPtr& cvList, const RenderLayoutPtr& layout);

	// ��������
	bool CullFace(int fType, const float4& p1, const float4& p2, const float4& p3);

	// ���� render_state ����ԭʼ������
	void DeviceDrawPrimitive(const RenderCVarlistPtr& cvList, const zbVertex4D& v1,const zbVertex4D& v2, const zbVertex4D& v3);

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
	int m_nPrimaryPatch;
	int m_nBackPatch;
};
#endif//DX_GRAPHDEVICE_H_