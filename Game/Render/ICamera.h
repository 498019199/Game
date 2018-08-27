// 2018年1月7日 zhangbei 相机基类

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
	// Qualifier:			设置摄像机的观察矩阵
	// eye_pos:			相机在世界坐标系中的位置
	// look_at	 :			世界坐标系中的观察点（相机观看的位置）
	//up_vec	 :			世界坐标系向上方向向量
	//************************************
	void ViewParams(const float3& eye_pos, const float3& look_at, const float3& up_vec);

	// Method	:			ProjParams
	// Qualifier	:			设置摄像机的投射矩阵
	// fFov			:			水平方向和垂直方向的视野
	// fNearClip	:			近裁剪面
	//fFarClip		:			远裁剪面
	//fAspectRatio	 :		屏幕的宽高比
	//************************************
	void ProjParams(float fFov, float fNearClip, float fFarClip, float fAspectRatio);

	// Method	:			ProjParams
	// Qualifier	:			设置摄像机的投射矩阵
	// fFov			:			水平方向和垂直方向的视野
	// fNearClip	:			近裁剪面
	//fFarClip		:			远裁剪面
	//nWidth		:			屏幕的宽
	//nHeight		:			屏幕的高
	//************************************
	void ProjParams(float fFov, float fNearClip, float fFarClip, int nWidth, int nHeight);
private:
	int m_nState;						//状态
	int m_nAttr;							// 属性

	float4 m_v4Angle;				// UVN相机模型的注视方向
	float3 m_pos;
	float3 m_v4U;						// UVN 相机的目标位置
	float3 m_v4V;
	float3 m_v4N;

	float m_fViewDist;			// 视距
	float m_fFov;						// 水平方向和垂直方向的视野

	float m_fNearClipZ;				// 近裁剪面
	float m_fFarClipZ;					// 远裁剪面

	Plane m_plClipRT;					// 右裁剪面
	Plane m_plClipLT;					// 左裁剪面
	Plane m_plClipTP;					// 上裁剪面
	Plane m_plClipBT;					// 下裁剪面

	float m_fViewPlaneWidth;	// 视平面的宽度
	float m_fViewPlaneHeight;	// 视平面的高度

	float m_fViewPortWidth;		// 屏幕宽
	float m_fViewPortHeight;		// 屏幕高
	float m_fAspectRatio;			// 屏幕的宽高比

	float4x4 m_ViewMat;		// 存储世界坐标到相机坐标变换矩阵
	float4x4 m_InvViewMat;

	float4x4 m_ProjMat;		// 存储相机坐标到透视坐标变换矩阵
	float4x4 m_InvProjMat;

	float4x4 m_m4Screem;		// 存储透视坐标到屏幕坐标变换矩阵
};
#endif//_CAMERA_H_
