#include "IGameObject.h"

IGameObject::IGameObject(Context* pContext)
	:IEntity(pContext)
{

}

IGameObject::~IGameObject()
{

}

bool IGameObject::OnInit()
{
	return true;
}

bool IGameObject::OnShut()
{
	return true;
}