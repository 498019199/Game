// 2018��1��7�� zhangbei �������

#ifndef _CAMERA_H_
#define _CAMERA_H_
#pragma once
#include "../../Core/Context.h"
#include "../../Core/Entity/Entity.h"
#include "../../Math/math.h"

class ICamera :public IEntity
{
public:
	STX_ENTITY(ICamera, IEntity);

	explicit ICamera(Context* pContext);

	~ICamera();

	static void RegisterObject(Context* pContext);

	virtual bool OnInit();

	virtual bool OnShut();

	float FOV() const	{	return m_fFov;	}

	float AspectRatio() const		{	return m_fAspectRatio;	}

	float NearPlane() const	{	return m_fNearClipZ;	}

	float FarPlane() const	{	return m_fFarClipZ;	}
	 
	float ViewDist() const { return m_fViewDist;  }

	const float4x4& ViewMatrix() const;
	const float4x4& ProjMatrix() const;
	const float4x4& InvertViewMatrix() const;
	const float4x4& InvertProjMatrix() const;
	const float3& GetEyePos() const;
	// Method:			ViewParams
	// Qualifier:			����������Ĺ۲����
	// eye_pos:			�������������ϵ�е�λ��
	// look_at	 :			��������ϵ�еĹ۲�㣨����ۿ���λ�ã�
	//up_vec	 :			��������ϵ���Ϸ�������
	//************************************
	void ViewParams(const float3& eye_pos, const float3& look_at, const float3& up_vec);

	// Method	:			ProjParams
	// Qualifier	:			�����������Ͷ�����
	// fFov			:			ˮƽ����ʹ�ֱ�������Ұ
	// fNearClip	:			���ü���
	//fFarClip		:			Զ�ü���
	//fAspectRatio	 :		��Ļ�Ŀ�߱�
	//************************************
	void ProjParams(float fFov, float fNearClip, float fFarClip, float fAspectRatio);

	// Method	:			ProjParams
	// Qualifier	:			�����������Ͷ�����
	// fFov			:			ˮƽ����ʹ�ֱ�������Ұ
	// fNearClip	:			���ü���
	//fFarClip		:			Զ�ü���
	//nWidth		:			��Ļ�Ŀ�
	//nHeight		:			��Ļ�ĸ�
	//************************************
	void ProjParams(float fFov, float fNearClip, float fFarClip, int nWidth, int nHeight);
private:
	int m_nState;						//״̬
	int m_nAttr;							// ����

	float4 m_v4Angle;				// UVN���ģ�͵�ע�ӷ���
	float3 m_pos;
	float3 m_v4U;						// UVN �����Ŀ��λ��
	float3 m_v4V;
	float3 m_v4N;

	float m_fViewDist;			// �Ӿ�
	float m_fFov;						// ˮƽ����ʹ�ֱ�������Ұ

	float m_fNearClipZ;				// ���ü���
	float m_fFarClipZ;					// Զ�ü���

	Plane m_plClipRT;					// �Ҳü���
	Plane m_plClipLT;					// ��ü���
	Plane m_plClipTP;					// �ϲü���
	Plane m_plClipBT;					// �²ü���

	float m_fViewPlaneWidth;	// ��ƽ��Ŀ��
	float m_fViewPlaneHeight;	// ��ƽ��ĸ߶�

	float m_fViewPortWidth;		// ��Ļ��
	float m_fViewPortHeight;		// ��Ļ��
	float m_fAspectRatio;			// ��Ļ�Ŀ�߱�

	float4x4 m_ViewMat;		// �洢�������굽�������任����
	float4x4 m_InvViewMat;

	float4x4 m_ProjMat;		// �洢������굽͸������任����
	float4x4 m_InvProjMat;

	float4x4 m_m4Screem;		// �洢͸�����굽��Ļ����任����
};
#endif//_CAMERA_H_
