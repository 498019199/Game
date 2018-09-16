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

class Renderer:public IEntityEx
{
public:
	STX_ENTITY(Renderer, IEntity);

	Renderer(Context* pContext);

	virtual bool OnInit() override;

	virtual bool OnShut() override;

	virtual void Update() override;

	void Inilize(HWND hwnd, HINSTANCE hInstance);
	void SetFont(DxGraphDevice* device);
	void ProccessWinMsg(std::size_t nParam1, std::size_t nParam2);

	// 载入
	void Load();

	// 运行
	void Refresh();
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
};
#endif//APP_H_
