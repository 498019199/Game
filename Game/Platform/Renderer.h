#ifndef APP_H_
#define APP_H_
#pragma once
#include "../Platform/DxGraphDevice.h"
#include "../Platform/DxTextPrintWindows.h"
#include "../Platform/DxIntPut.h"
#include "../System/Timer.h"

enum class GameState
{
	GAME_INIT,				// 游戏初始化
	GAME_MENU,			// 游戏处于登陆
	GAME_START,			// 游戏正在开始运行
	GAME_RUN,				// 游戏运行中
	GAME_RESET,			// 游戏重新启动
	GAME_EXIT,				// 游戏退出
};

class Renderer
{
public:
	// 初始化
	Renderer& Init()
	{
		static Renderer s_App;
		return s_App;
	}

	void Inilize(int nWidth, int nHeight, const char* szWorkPath, HINSTANCE hInstance);
	void SetFont(DxGraphDevice* device);
	void ProccessWinMsg(std::size_t nParam1, std::size_t nParam2);

	// 计时开始
	void StarTimer() { m_Timer.ReStart(); }
	// 计时暂停
	bool WaitTimer(uint32_t nWaitTime)
	{ 
		while (m_Timer.Elapsed() < nWaitTime)
		{
			//double _1 = m_Timer.Elapse();
		}

		m_Timer.ReStart();
		return true;
	}
	// 计时结束

	// 载入
	void Load();

	// 运行
	void Run();

	// 退出
	void Exit();

	// 用于计算帧速率  
	float GetFPS();
private:
	// 渲染处理
	void Render();

	// 逻辑处理
	void Display(float second);
private:
	int g_GameState;			// 游戏状态
	int g_ErrorMsg;			// 游戏错误标识
	DxIntPut m_IntPut;
	DxTextPrintWindows m_TextPrint;
	// 时间，帧数管理
	Timer m_Timer;
	float m_fAppTime;
	float m_fFrameTime;
	uint32_t m_nTotalNumFrames;
	uint32_t n_nNumFrame;
	float m_fAccumulate;
	float m_fFPS;
};
#endif//APP_H_
