#ifndef APP_H_
#define APP_H_
#pragma once
#include "../Platform/DxGraphDevice.h"
#include "../Platform/DxTextPrintWindows.h"
#include "../Platform/DxIntPut.h"
#include "../System/Timer.h"

enum class GameState
{
	GAME_INIT,				// ��Ϸ��ʼ��
	GAME_MENU,			// ��Ϸ���ڵ�½
	GAME_START,			// ��Ϸ���ڿ�ʼ����
	GAME_RUN,				// ��Ϸ������
	GAME_RESET,			// ��Ϸ��������
	GAME_EXIT,				// ��Ϸ�˳�
};

class Renderer
{
public:
	// ��ʼ��
	Renderer& Init()
	{
		static Renderer s_App;
		return s_App;
	}

	void Inilize(int nWidth, int nHeight, const char* szWorkPath, HINSTANCE hInstance);
	void SetFont(DxGraphDevice* device);
	void ProccessWinMsg(std::size_t nParam1, std::size_t nParam2);

	// ��ʱ��ʼ
	void StarTimer() { m_Timer.ReStart(); }
	// ��ʱ��ͣ
	bool WaitTimer(uint32_t nWaitTime)
	{ 
		while (m_Timer.Elapsed() < nWaitTime)
		{
			//double _1 = m_Timer.Elapse();
		}

		m_Timer.ReStart();
		return true;
	}
	// ��ʱ����

	// ����
	void Load();

	// ����
	void Run();

	// �˳�
	void Exit();

	// ���ڼ���֡����  
	float GetFPS();
private:
	// ��Ⱦ����
	void Render();

	// �߼�����
	void Display(float second);
private:
	int g_GameState;			// ��Ϸ״̬
	int g_ErrorMsg;			// ��Ϸ�����ʶ
	DxIntPut m_IntPut;
	DxTextPrintWindows m_TextPrint;
	// ʱ�䣬֡������
	Timer m_Timer;
	float m_fAppTime;
	float m_fFrameTime;
	uint32_t m_nTotalNumFrames;
	uint32_t n_nNumFrame;
	float m_fAccumulate;
	float m_fFPS;
};
#endif//APP_H_
