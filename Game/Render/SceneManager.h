// 2018��1��7�� zhangbei ������Ⱦ��ˮ
#ifndef _SCENE_H_
#define _SCENE_H_
#pragma once
#include "../Core/predefine.h"
#include "../../Core/Context.h"
#include "../../Core/entity/Entity.h"
#include "../../Math/math.h"

class SceneManager :public IEntity
{
public:
	STX_ENTITY(SceneManager, IEntity);

	SceneManager(Context* pContext);

	static void RegisterObject(Context* pContext);

	~SceneManager();

	// ��ȡ��ǰ���
	CameraPtr ActiveCamera() { return m_Camera; }

	void AddVisBase(VisBasePtr vis);
	void Render();
private:
	int m_nAttr;												// ����

	std::vector<VisBasePtr> m_VisBaseList;	// ��Ⱦ�б�ָ������
	CameraPtr m_Camera;
	std::vector<LightPtr> m_Lights;
};
#endif//_SCENE_H_
