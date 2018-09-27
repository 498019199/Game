// 2018年1月7日 zhangbei 管理渲染流水
#ifndef _SCENE_H_
#define _SCENE_H_
#pragma once
#include "../Core/predefine.h"
#include "../../Core/Context.h"
#include "../../Core/entity/Entity.h"
#include "../../Math/math.h"

class IScene :public IEntity
{
public:
	STX_ENTITY(IScene, IEntity);

	IScene(Context* pContext);

	static void RegisterObject(Context* pContext);

	~IScene();

	uint32_t GetPolysNum() const;

	// 获取当前相机
	CameraPtr ActiveCamera() { return m_Camera; }

	void AddVisBase(RenderablePtr vis);
	void Render();
private:
	int m_nAttr;												// 属性

	std::vector<RenderablePtr> m_VisBaseList;	// 渲染列表指针数组
	CameraPtr m_Camera;
	FirstPersonCameraControllerPtr m_CameraControalPtr;
	std::vector<LightPtr> m_Lights;
};
#endif//_SCENE_H_
