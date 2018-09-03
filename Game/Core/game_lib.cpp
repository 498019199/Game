#include "../Core/Context.h"
#include "CHelper.h"
#include "../Render/ICamera.h"
#include "../Render/ILight.h"
#include "../Render/SceneManager.h"
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

	auto pScene = Context::Instance()->CreateObject<SceneManager>();
	if (nullptr == pScene)
	{
		return;
	}

	Context::Instance()->SetScene(pScene);
	auto model = SyncLoadModel("Dragon_meshml/Dragon.obj", 0);
	model->LoadMeshTexture();
	model->SetPosition(0.f, 2.f, -1.f);
	pScene->AddVisBase(model);
	_1 = 1;
}