#include "DxTextPrintWindows.h"
#include <stdio.h>
LPD3DXFONT              g_pTextFlamatYH;				// ����

DxTextPrintWindows::DxTextPrintWindows()
{
}

DxTextPrintWindows::~DxTextPrintWindows()
{
	if (g_pTextFlamatYH)
	{
		g_pTextFlamatYH->Release();
	}
}

bool DxTextPrintWindows::InitFont(DxGraphDevice* device, const char* sFormat)
{
	//��������  
	D3DXCreateFont(device->GetDevice(), 23, 0, 1000, 0, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 0, sFormat, &g_pTextFlamatYH);

	return true;
}

void DxTextPrintWindows::PrintWindows(int3 v3Pos, float fps, char ch, bool* s, const char* szMsg)
{
	RECT formatRect;
	GetClientRect(m_hWnd, &formatRect);
	char szBuffer[1024] = {0};
	int charCount = sprintf_s(szBuffer, "FPS:%0.3f", fps);
	if (g_pTextFlamatYH)
	{
		g_pTextFlamatYH->DrawText(NULL, szBuffer, charCount, &formatRect,
			DT_TOP | DT_RIGHT , D3DCOLOR_RGBA(0, 239, 136, 255));
	}
	
	// ��ӡ���λ��
	if (1)
	{
		charCount = sprintf_s(szBuffer, "Mouth:%d,%d,%d:%c,%c,%c,%c,", v3Pos.x(), v3Pos.y(), v3Pos.z(),
			s[0],s[1],s[2],s[3]);
		g_pTextFlamatYH->DrawText(NULL, szBuffer, charCount, &formatRect,
			DT_TOP | DT_LEFT, D3DXCOLOR(1.0f, 0.5f, 0.0f, 1.0f));
	}
	// ��ӡ����
	if (1)
	{
		charCount = sprintf_s(szBuffer, "keyboard:%c", ch);
		formatRect.top += 25;/**/
		g_pTextFlamatYH->DrawText(NULL, szBuffer, charCount, &formatRect,
			DT_NOCLIP |DT_SINGLELINE | DT_LEFT, D3DCOLOR_RGBA(0, 239, 136, 255));
	}
}
