// 2018年1月7日 zhangbei 可视对象

#ifndef _IVISBASE_H_
#define _IVISBASE_H_
#pragma once
#include "../../Core/Context.h"
#include "../Core/predefine.h"
#include "../Render/RenderMaterial.h"
#include "../Math/Math.h"

#include <vector>
#include <array>
typedef struct zbVertex4D
{
	int nAttr;							// 属性或亮度
	Color color;						// 颜色
	float4 v;							// 点
	float4 n;							// 顶点法线
	float2 t;							// 贴图坐标

	zbVertex4D()
		:nAttr(0)
	{}

	zbVertex4D(const float4& pos, const float4& normal, const float2 ts, const Color& clr)
		:nAttr(0), v(pos), n(normal), t(ts), color(clr)
	{}
}zbVertex4D;
typedef std::vector<zbVertex4D> VertexVec;
typedef std::shared_ptr<VertexVec> VertexBuffer;

enum FullCallType
{
	CULL_FACE_FRONT = 1,
	CULL_FACE_BACK = 2,
};

enum ClipType
{
	CLIP_X__MIN = 0x0001,
	CLIP_X__MID = 0x0002,
	CLIP_X__MAX = 0x0004,
	CLIP_Z__MIN = 0x0010,
	CLIP_Z__MID = 0x020,
	CLIP_Z__MAX = 0x0040,
	CLIP_Y__MIN = 0x0100,
	CLIP_Y__MID = 0x0200,
	CLIP_Y__MAX = 0x0400,
};

class Renderable :public IEntity
{
public:

	STX_ENTITY(Renderable, IEntity);

	explicit Renderable(Context* pContext);

	static void RegisterObject(Context* pContext);

	// 设置位置
	void SetPosition(const float3& pos);
	void SetPosition(float x, float y, float z);
	// 获取坐标
	float3 GetPosition() { return m_Position; }
	float GetPosX() { return m_Position.x(); }
	float GetPosY() { return m_Position.y(); }
	float GetPosZ() { return m_Position.z(); }

	// 设置缩放
	void SetScale(const float3& scale);
	void SetScale(float x, float y, float z);
	// 获取缩放
	float3 GetScale() { return m_Scale; }

	// 设置角度
	void SetAngle(const float3& angle);
	void SetAngle(float x, float y, float z);
	// 获取角度
	float3 GetAngle() { return m_Angle; }
private:
	// 初始化绑定数据
	void BindDeferredData(const RenderCVarlistPtr& cvList);
public:
	virtual void Render();

	virtual void OnRenderBegin();

	virtual void OnRenderEnd();

	template <typename ForwardIterator>
	void AssignSubVisbase(ForwardIterator first, ForwardIterator last)
	{
		m_SubVisbase.assign(first, last);
	}
	RenderablePtr const & SubVisBase(size_t id) const
	{
		return m_SubVisbase[id];
	}
	size_t GetSubVisBaseNum() { return m_SubVisbase.size(); }

	virtual RenderLayoutPtr GetRenderLayout() const = 0;
	virtual const RenderCVarlistPtr& GetRenderEffect() const;
protected:
	std::string strName;
	float3 m_Position;								// 位置
	float3 m_Scale;									// 缩放
	float3 m_Angle;									// 角度
	float4x4 m_ModleMat;							// 转换矩阵

	// 配置数据
	RenderCVarlistPtr m_cvList;
	RenderCVarParameter* pos_center;
	RenderCVarParameter* mvp_param;
	RenderCVarParameter* inv_mv_param;
	RenderCVarParameter* model_view_param;
	RenderCVarParameter* albedo_tex_param;
	RenderCVarParameter* albedo_clr_param;
	RenderCVarParameter* metalness_tex_param;
	RenderCVarParameter* metalness_clr_param;
	RenderCVarParameter* glossiness_tex_param;
	RenderCVarParameter* glossiness_clr_param;
	RenderCVarParameter* emissive_tex_param;
	RenderCVarParameter* emissive_clr_param;
	RenderCVarParameter* normal_tex_param;
	RenderCVarParameter* height_tex_param;
	RenderCVarParameter* cull_mode;

	RenderMaterialPtr m_Mtl;
	std::array<TexturePtr, RenderMaterial::TextureType::TS_TypeCount> m_Textures;

	std::vector<RenderablePtr> m_SubVisbase;
};
#endif//_IVISBASE_H_
