#ifndef DX_INTPUT_H_
#define DX_INTPUT_H_
#pragma once

#pragma comment(lib, "Dinput8.lib") 
#define DIRECTINPUT_HEADER_VERSION 0x0800  
#ifndef DIRECTINPUT_VERSION  
	#define DIRECTINPUT_VERSION       DIRECTINPUT_HEADER_VERSION  
#endif//DIRECTINPUT_VERSION
#define KEYBOARD_NUMBER 256
#include <dinput.h>
#include "DxGraphDevice.h"
#include "../Container/var_type.h"

class DxIntPut
{
public:
	DxIntPut();

	~DxIntPut();

	bool InitDevice(HINSTANCE hInstance, HWND hwnd);

	void Execute();

	void ShutDown();

	int3 GetMousePos();
	
	void GetMouseButton(bool (&a)[4]);

	char GetKeyCode();
private:
	// 创建鼠标硬件
	void CreateMouse();

	// 创建键盘硬件
	void CreateKeyBoard();

	// 读取硬件信息
	bool Read(IDirectInputDevice8  *pDIDevice, void* pBuffer, int nSize);
private:
	LPDIRECTINPUT8 m_pDirectInput;
	HWND m_hwnd;
	// 鼠标
	DIMOUSESTATE            m_diMouseState;
	// 键盘
	char                    m_pKeyStateBuffer[KEYBOARD_NUMBER];
};
#endif//DX_INTPUT_H_