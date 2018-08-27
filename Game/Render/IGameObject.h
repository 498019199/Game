// 2018年1月7日 zhangbei 游戏对象插件管理

#ifndef _IGAMEOBJECT_H_
#define _IGAMEOBJECT_H_
#pragma once
#include "../../Core/Context.h"
#include "../../Core/Entity/Entity.h"
#include "../../Math/math.h"

class IGameObject :public IEntity
{
	STX_ENTITY(IGameObject, IEntity);
public:
	explicit IGameObject(Context* pContext);

	~IGameObject();

	virtual bool OnInit();

	virtual bool OnShut();
private:

};
#endif//_IGAMEOBJECT_H_
