// 2018年9月28日 zhangbei 模型加载
#ifndef _CLIENT_PLAYER_H_
#define _CLIENT_PLAYER_H_
#pragma once
#include "../Core/predefine.h"
#include "../Core/App.h"
#include "../Platform/MsgInput/MInput.hpp"
class ClientPlayer:public App
{
public:
	ClientPlayer();

	void OnCreate();
private:
	void LoadScene();

	void CreatePlayer();

	void InputHandler(const InputEngine& sender, const InputAction& action);

	void DoUpdateOverlay();
	uint32_t DoUpdate(uint32_t pass);
private:
	std::vector<RenderablePtr> m_SceneObj;
	IScenePtr m_pScene;
	FirstPersonCameraControllerPtr m_CameraControalPtr;
};
#endif//_CLIENT_PLAYER_H_
