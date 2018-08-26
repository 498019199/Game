#include "../Core/ICore.h"
#include "../Core/LogModle.h"
#include "../Core/entity/Method.h"
#include "CHelper.h"
#include "define/definename.h"
#include "Visible/ICamera.h"
#include "Visible/ILight.h"
#include "Visible/Scene.h"
#include "Visible/ITexture.h"
#include "Visible/IVisBase.h"
#include "Visible/IGameObject.h"
#include "Visible/Mesh.h"
#include <boost/assert.hpp>
extern LogModle *g_Loger;

ICore* init_core()
{
	ICore *pCore = NEW ICore;
	if (nullptr == pCore)
	{
		return nullptr;
	}

	ITexture::setDefaultAlphaPixelFormat(ITexture::PixelFormat::RGB565);
	return pCore;
}

void init_core_list()
{
	REGISTER_ENTITY(ILight);
	//REGISTER_ENTITY(SceneObjMrg);
	REGISTER_ENTITY(Scene);
	REGISTER_ENTITY(ICamera);
	//REGISTER_ENTITY(StaticQueryMrg);
	REGISTER_ENTITY(LogModle);

	REGISTER_ENTITY(IVisBase);
	REGISTER_ENTITY(IGameObject);
}

void end_core()
{
	//CHelper::RemoveEntity(SYSTEM_SCENE_MRG);
	//CHelper::RemoveEntity(SYSTEM_QUERY_MRG);
	CHelper::RemoveEntity(SYSTEM_CAMERA);
	CHelper::RemoveEntity(SYSTEM_LIGHT);
	CHelper::RemoveEntity("Scene");
	CHelper::RemoveEntity("IVisBase");
	CHelper::RemoveEntity("IGameObject");
	CHelper::RemoveEntity("LogModle");
	delete g_pCore;
	g_pCore = nullptr;
	//delete g_Loger;
	g_Loger = nullptr;
}

void ICore::Test()
{
	static int _1 = 0;
	if (_1 == 1)
	{
		return;
	}

	Scene* pScene = static_cast<Scene*>(CHelper::CreateEntity("Scene"));
	if (nullptr == pScene)
	{
		return;
	}
	auto entity2 = static_cast<ICamera*>(CHelper::CreateEntity(SYSTEM_CAMERA));
	BOOST_ASSERT(nullptr != entity2);
	float3 pos(0, 2.f, -2.5f);
	float3 at(0, 0, 1);
	entity2->ViewParams(pos, at + pos, float3(0, 1, 0));
	entity2->ProjParams(MathLib::PIdiv2, 0.1f, 500.f, g_pCore->GetWidth(), g_pCore->GetHeight());
	pScene->AddCamera(CameraPtr(entity2));
	g_pCore->SetScene(pScene);
	IVisBase* entity1 = static_cast<IVisBase*>(CHelper::CreateEntityArgs("IVisBase"));
	if (nullptr == entity1)
	{
		return;
	}
	auto model = SyncLoadModel("nanosuit.meshml", 0);
	//entity1->CreateModle("ini\\obj\\box.ini");
	entity1->SetPosition(0.f, 2.f, -1.f);
	pScene->AddVisBase(VisBasePtr(entity1));
	_1 = 1;
}