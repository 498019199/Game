#include "../Core/Context.h"
#include "CHelper.h"
#include "../Core/Compenent/CameraController.hpp"
#include "../Render/ICamera.h"
#include "../Render/ILight.h"
#include "../Render/IScene.h"
#include "../Render/ITexture.h"
#include "../Render/Renderable.h"
#include "../Render/IGameObject.h"
#include "../Render/Mesh.h"
#include <boost/assert.hpp>


void Context::Test()
{
	static int _1 = 0;
	if (_1 == 1)
	{
		return;
	}

	auto pScene = Context::Instance()->CreateObject<IScene>();
	if (nullptr == pScene)
	{
		return;
	}

	Context::Instance()->SetScene(pScene);
	auto model = SyncLoadModel("Dragon_meshml/Dragon.obj", 0);
	model->LoadMeshTexture();
	model->SetPosition(0.f, 2.f, -20.f);
	pScene->AddVisBase(model);
	_1 = 1;
}