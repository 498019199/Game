#ifndef _LIGHT_H_
#define _LIGHT_H_
#pragma once
#include "../Core/Context.h"
#include "../Core/Entity/Entity.h"
#include "../Math/Color.h"
#include "../Math/math.h"

class ILight :public IEntity
{
	enum LightType
	{
		LT_None = -1,
		LT_Ambient = 0,		// 环境光
		LT_Sun,					// 发光光源
		LT_Directional,			// 定向光源
		LT_Point,					// 点光源
		LT_Spot1,					// 聚光灯光源简单
		LT_Spot2,					// 聚光灯光源复杂
		LT_LIGHT_MAX,
	};
public:
	STX_ENTITY(ILight, IEntity);

	explicit ILight(Context* pContext);

	~ILight();

	virtual bool OnInit();

	virtual bool OnShut();

public:
	int GetState() const { return m_nState; }
	int GetAttr() const { return m_nAttr; }
	LightType GetLightType() const { return m_nType; }
	float4 GetPosition() const { return m_Pos; }

	// 
	virtual Color Update(const Color* base, float4* verts) const;
	// Gourand 着色 
	virtual void Update(const Color* color, float4* verts, Color* colors) const;
private:
	int m_nState;			// 状态
	int m_nAttr;				// 属性
	LightType m_nType;// 光源类型
	float4 m_Pos;			// 位置
};

// 环境光
class AmbientLightSource :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit AmbientLightSource(Context* pContext, Color i3Ambient);

	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Ambient; // 环境光强度
};

//// 发光光源
//class SunLightSource :public ILight
//{
//	virtual Color Update(const zbPolyF4D* ploys, const float4 n);
//private:
//	Color m_Diffuse; // 散色光强度
//};

// 定向光源
class DirectionalLightSource :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit DirectionalLightSource(Context* pContext, float4 f4Dir, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Diffuse; // 散色光强度
	float4 m_Direction; // 方向
};

// 点光源
class PointLightSource :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit PointLightSource(Context* pContext, float fKc, float fKl, float fKq, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Diffuse; // 散色光强度
	float m_fKc, m_fKl, m_fKq;//衰减因子
};

// 聚光灯光源简单
class SpotLightSource1 :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit SpotLightSource1(Context* pContext, float fKc, float fKl, float fKq, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Diffuse; // 散色光强度
	float m_fKc, m_fKl, m_fKq;//衰减因子
};

// 聚光灯光源复杂
class SpotLightSource2 :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit SpotLightSource2(Context* pContext, float fKc, float fKl, float fKq,
		float fSpotInner, float fSpotOuter, float fPf, 
		float4 f4Dir, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	float m_fKc, m_fKl, m_fKq;//衰减因子
	float m_fSpotInner;		//聚光灯内锥角
	float m_fSpotOuter;		//聚光灯外锥角
	float m_fPf;					//聚光灯指数因子
	float4 m_Direction; // 方向
	Color m_Diffuse; // 散色光强度
};
#endif//_LIGHT_H_
