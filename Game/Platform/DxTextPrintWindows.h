#ifndef DX_TEXT_PRINTWINDOWS_H_
#define DX_TEXT_PRINTWINDOWS_H_
#pragma once
#include "DxGraphDevice.h"
#include "../SDK/Dx/Include/d3dx9.h"
/*#include <d3dx9.h>  我加了，为什么还用不了/(ㄒoㄒ)/~~*/
class DxTextPrintWindows
{
public:
	DxTextPrintWindows();

	~DxTextPrintWindows();

	bool InitFont(DxGraphDevice* device, const char* szFormat);

	void PrintWindows(int3 v3Pos, float fps, char ch, bool* s, const char* szMsg);
private:
	HWND	m_hWnd;				// Win32 Window handle
};
#endif//DX_TEXT_PRINTWINDOWS_H_