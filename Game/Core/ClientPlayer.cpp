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

	auto model = SyncLoadModel("Dragon_meshml/Dragon.obj", 0);
	model->LoadMeshTexture();
	model->SetPosition(0.f, 2.f, -20.f);
	m_pScene->AddVisBase(model);
	m_SceneObj.push_back(model);
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
