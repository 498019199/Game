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

	// ����
	void Load();

	// ����
	void Refresh();
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
};
#endif//APP_H_
