// 2018年8月15日 zhangbei 场景对象

#ifndef _SCENEOBJ_H_
#define _SCENEOBJ_H_
#pragma once

#include "../Core/predefine.h"

#include <vector>
#include <boost/noncopyable.hpp>

class SceneObject : boost::noncopyable
{
public:
	enum SOAttrib
	{
		SOA_Cullable = 1UL << 0,
		SOA_Overlay = 1UL << 1,
		SOA_Moveable = 1UL << 2,
		SOA_Invisible = 1UL << 3,
		SOA_NotCastShadow = 1UL << 4,
		SOA_SSS = 1UL << 5
	};

	explicit SceneObject(uint32_t nAttr);

	virtual ~SceneObject();
private:
	uint32_t m_nAttr;
	SceneObjectPtr m_Parent;
	std::vector<SceneObject> m_Children;
};
#endif//_SCENEOBJ_H_
