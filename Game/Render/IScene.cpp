#include "IScene.h"
#include "Mesh.h"
#include "Renderable.h"
#include "ICamera.h"
#include "../Platform/MsgInput/MInput.hpp"
#include "../Core/Compenent/CameraController.hpp"
IScene::IScene(Context* pContext)
	:IEntity(pContext)
{
	auto camera = Context::Instance()->CreateObject<ICamera>();
	BOOST_ASSERT(nullptr != camera);
	float3 pos(0, 2.f, -2.5f);
	float3 at(0, 0, 1);
	camera->ViewParams(pos, at + pos, float3(0, 1, 0));
	camera->ProjParams(MathLib::PIdiv2, 0.1f, 500.f, Context::Instance()->GetWidth(), Context::Instance()->GetHeight());
	m_Camera = camera;

	m_CameraControalPtr = Context::Instance()->CreateObject<FirstPersonCameraController>();
	m_CameraControalPtr->AttachCamera(*m_Camera);
	m_CameraControalPtr->Scalers(0.05f, 5.f);
}

void IScene::RegisterObject(Context* pContext)
{
	pContext->RegisterFactory<IScene>();
}

IScene::~IScene()
{

}

uint32_t IScene::GetPolysNum() const
{
	uint32_t nCount = 0;
	for (auto it : m_VisBaseList)
	{
		for (uint32_t i = 0; i < it->GetSubVisBaseNum(); ++i)
		{
			auto vis = reinterpret_cast<StaticMesh*>(it->SubVisBase(i).get());
			auto layer = vis->GetRenderLayout();
			nCount += layer->GetTriCount();
		}
	}

	return nCount;
}

void IScene::AddVisBase(RenderablePtr vis)
{
	m_VisBaseList.push_back(vis);
}

void IScene::Render()
{
	auto ie = Context::Instance()->Instance()->GetSubsystem<MsgInputEngine>();
	ie->Update();
	for (auto it : m_Lights)
	{
	}

	for (auto it : m_VisBaseList)
	{
		it->Render();
	}
}
