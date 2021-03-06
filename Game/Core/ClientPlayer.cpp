#include "ClientPlayer.h"
#include "../Render/IScene.h"
#include "../Render/Mesh.h"
#include "../Core/Compenent/CameraController.hpp"

enum
{
	Exit,
};

InputActionDefine actions[] =
{
	InputActionDefine(Exit, KS_Escape),
};

ClientPlayer::ClientPlayer()
	:App("ClientPlayer", nullptr)
{

}

void ClientPlayer::OnCreate()
{
	LoadScene();
	m_CameraControalPtr = Context::Instance()->CreateObject<FirstPersonCameraController>();
	m_CameraControalPtr->AttachCamera(*m_pScene->ActiveCamera());
	m_CameraControalPtr->Scalers(0.05f, 0.5f);

	InputEngine& inputEngine(*Context::Instance()->GetSubsystem<MsgInputEngine>());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));
	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(
		[this](InputEngine const & sender, InputAction const & action)
	{
		this->InputHandler(sender, action);
	});
	inputEngine.ActionMap(actionMap, input_handler);

	//auto model = SyncLoadModel("Dragon_meshml/Dragon.obj", 0);
	//model->LoadMeshTexture();
	//model->SetPosition(0.f, 90.f, -20.f);
	//model->SetScale(0.1f, 0.1f, 0.1f);
	//m_pScene->AddVisBase(model);

	auto cube = SyncLoadModel("Dragon_meshml/cube.obj", 0);
	cube->LoadMeshTexture();
	cube->SetPosition(0.f, 0.f, 0.f);
	cube->SetScale(1.f, 1.f, 1.f);
	m_pScene->AddVisBase(cube);
	m_SceneObj.push_back(cube);

	//auto model = SyncLoadModel("Dragon_meshml/Infinite-Level_02.obj", 0);
	//model->LoadMeshTexture();
	//model->SetPosition(0.f, 0.f, 0.f);
	//model->SetScale(5.f, 5.f, 5.f);
	//m_pScene->AddVisBase(model);
}

void ClientPlayer::LoadScene()
{
	m_pScene = Context::Instance()->CreateObject<IScene>();
	BOOST_ASSERT(nullptr != m_pScene);
	Context::Instance()->SetScene(m_pScene);
}

void ClientPlayer::CreatePlayer()
{

}

void ClientPlayer::InputHandler(const InputEngine& sender, const InputAction& action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void ClientPlayer::DoUpdateOverlay()
{
	Context::Instance()->DisPlay(FrameTime());
}

uint32_t ClientPlayer::DoUpdate(uint32_t pass)
{
	return 0;
}
