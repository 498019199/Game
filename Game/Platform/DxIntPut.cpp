#include "../Platform/DxIntPut.h"

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }      //�Զ���һ��SAFE_RELEASE()��,����COM��Դ���ͷ� 
extern HWND g_hwnd;
LPDIRECTINPUTDEVICE8		g_pMouseDevice;
LPDIRECTINPUTDEVICE8		g_pKeyboardDevice;
DxIntPut::DxIntPut()
{
	m_pDirectInput = NULL;
	::ZeroMemory(&m_diMouseState, sizeof(m_diMouseState));
	::ZeroMemory(m_pKeyStateBuffer, sizeof(m_pKeyStateBuffer));
}

DxIntPut::~DxIntPut()
{
	ShutDown();
}

bool DxIntPut::InitDevice(HINSTANCE hInstance)
{
	if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_HEADER_VERSION, IID_IDirectInput8, (void**)&m_pDirectInput, NULL)))
	{
		return false;
	}

	CreateMouse();
	CreateKeyBoard();
	return true;
}

void DxIntPut::Execute()
{
	// ��ȡ�������  
	::ZeroMemory(&m_diMouseState, sizeof(m_diMouseState));
	Read(g_pMouseDevice, (LPVOID)&m_diMouseState, sizeof(m_diMouseState));

	// ��ȡ��������  
	::ZeroMemory(m_pKeyStateBuffer, sizeof(m_pKeyStateBuffer));
	Read(g_pKeyboardDevice, (LPVOID)m_pKeyStateBuffer, sizeof(m_pKeyStateBuffer));
}

void DxIntPut::ShutDown()
{
	if(nullptr != g_pMouseDevice)
	{
		g_pMouseDevice->Unacquire();
		SAFE_RELEASE(g_pMouseDevice);
	}
	if (nullptr != g_pMouseDevice)
	{
		g_pKeyboardDevice->Unacquire();
		SAFE_RELEASE(g_pKeyboardDevice);
	}
	SAFE_RELEASE(m_pDirectInput);
}

int3 DxIntPut::GetMousePos()
{
	static int3 v3Pos(0, 0, 0);
	if (g_pMouseDevice)
	{
		v3Pos.x() = m_diMouseState.lX;
		v3Pos.y() = m_diMouseState.lY;
		v3Pos.z() = m_diMouseState.lZ;
	}

	return v3Pos;
}

void DxIntPut::GetMouseButton(bool(&a)[4])
{
	if (m_diMouseState.rgbButtons[0] & 0x80)
		a[0] = true;
	else if (m_diMouseState.rgbButtons[1] & 0x80)
		a[1] = true;
	else if (m_diMouseState.rgbButtons[2] & 0x80)
		a[2] = true;
	else if (m_diMouseState.rgbButtons[3] & 0x80)
		a[3] = true;
}

char DxIntPut::GetKeyCode()
{
	if (m_pKeyStateBuffer[DIK_W] & 0x80)
		return 'w';
	else if (m_pKeyStateBuffer[DIK_S] & 0x80)
		return 's';
	else if (m_pKeyStateBuffer[DIK_A] & 0x80)
		return 'a';
	else if (m_pKeyStateBuffer[DIK_D] & 0x80)
		return 'd';

	return 0;
}

void DxIntPut::CreateMouse()
{
	// �������ݸ�ʽ��Э������  
	m_pDirectInput->CreateDevice(GUID_SysMouse, &g_pMouseDevice, NULL);
	g_pMouseDevice->SetDataFormat(&c_dfDIMouse);
	//g_pMouseDevice->SetCooperativeLevel(g_hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	//��ȡ�豸����Ȩ  
	g_pMouseDevice->Acquire();
}

void DxIntPut::CreateKeyBoard()
{
	// �������ݸ�ʽ��Э������  
	m_pDirectInput->CreateDevice(GUID_SysKeyboard, &g_pKeyboardDevice, NULL);
	g_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	g_pKeyboardDevice->SetCooperativeLevel(g_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	//��ȡ�豸����Ȩ  
	g_pKeyboardDevice->Acquire();
}

bool DxIntPut::Read(IDirectInputDevice8 *pDIDevice, void* pBuffer, int nSize)
{
	HRESULT hr;
	while (true)
	{
		pDIDevice->Poll();             // ��ѯ�豸  
		pDIDevice->Acquire();          // ��ȡ�豸�Ŀ���Ȩ  
		if (SUCCEEDED(hr = pDIDevice->GetDeviceState(nSize, pBuffer)))
			break;
		if (hr != DIERR_INPUTLOST || hr != DIERR_NOTACQUIRED)
			return false;
		if (FAILED(pDIDevice->Acquire()))
			return false;
	}

	return true;
}

