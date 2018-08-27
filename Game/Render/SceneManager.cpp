#include "SceneManager.h"
#include "Renderable.h"
#include "ICamera.h"

SceneManager::SceneManager(Context* pContext)
	:IEntity(pContext)
{
	auto camera = Context::Instance()->CreateObject<ICamera>();
	BOOST_ASSERT(nullptr != camera);
	float3 pos(0, 2.f, -2.5f);
	float3 at(0, 0, 1);
	camera->ViewParams(pos, at + pos, float3(0, 1, 0));
	camera->ProjParams(MathLib::PIdiv2, 0.1f, 500.f, Context::Instance()->GetWidth(), Context::Instance()->GetHeight());
	m_Camera = camera;
}

void SceneManager::RegisterObject(Context* pContext)
{
	pContext->RegisterFactory<SceneManager>();
}

SceneManager::~SceneManager()
{

}

void SceneManager::AddVisBase(VisBasePtr vis)
{
	m_VisBaseList.push_back(vis);
}

void SceneManager::Render()
{
	for (auto it : m_Lights)
	{
	}

	for (auto it : m_VisBaseList)
	{
		it->Render();
	}
}
