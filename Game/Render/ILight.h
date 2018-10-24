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
		LT_Ambient = 0,		// ������
		LT_Sun,					// �����Դ
		LT_Directional,			// �����Դ
		LT_Point,					// ���Դ
		LT_Spot1,					// �۹�ƹ�Դ��
		LT_Spot2,					// �۹�ƹ�Դ����
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
	// Gourand ��ɫ 
	virtual void Update(const Color* color, float4* verts, Color* colors) const;
private:
	int m_nState;			// ״̬
	int m_nAttr;				// ����
	LightType m_nType;// ��Դ����
	float4 m_Pos;			// λ��
};

// ������
class AmbientLightSource :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit AmbientLightSource(Context* pContext, Color i3Ambient);

	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Ambient; // ������ǿ��
};

//// �����Դ
//class SunLightSource :public ILight
//{
//	virtual Color Update(const zbPolyF4D* ploys, const float4 n);
//private:
//	Color m_Diffuse; // ɢɫ��ǿ��
//};

// �����Դ
class DirectionalLightSource :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit DirectionalLightSource(Context* pContext, float4 f4Dir, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Diffuse; // ɢɫ��ǿ��
	float4 m_Direction; // ����
};

// ���Դ
class PointLightSource :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit PointLightSource(Context* pContext, float fKc, float fKl, float fKq, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Diffuse; // ɢɫ��ǿ��
	float m_fKc, m_fKl, m_fKq;//˥������
};

// �۹�ƹ�Դ��
class SpotLightSource1 :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit SpotLightSource1(Context* pContext, float fKc, float fKl, float fKq, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	Color m_Diffuse; // ɢɫ��ǿ��
	float m_fKc, m_fKl, m_fKq;//˥������
};

// �۹�ƹ�Դ����
class SpotLightSource2 :public ILight
{
	STX_ENTITY(AmbientLightSource, ILight);

	explicit SpotLightSource2(Context* pContext, float fKc, float fKl, float fKq,
		float fSpotInner, float fSpotOuter, float fPf, 
		float4 f4Dir, Color i3Diffuse);
	virtual void Update(const Color* base, float4* verts, Color* colors) const;
	virtual Color Update(const Color* base, float4* verts) const;
private:
	float m_fKc, m_fKl, m_fKq;//˥������
	float m_fSpotInner;		//�۹����׶��
	float m_fSpotOuter;		//�۹����׶��
	float m_fPf;					//�۹��ָ������
	float4 m_Direction; // ����
	Color m_Diffuse; // ɢɫ��ǿ��
};
#endif//_LIGHT_H_
