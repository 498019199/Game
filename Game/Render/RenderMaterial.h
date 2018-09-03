// 2018��7��8�� zhangbei ����
#ifndef _MATERIAL_H_
#define _MATERIAL_H_
#pragma once

#include "../Math/Math.h"
#include "../Core/predefine.h"
#include <string>
class RenderMaterial
{
public:
	enum TextureType : uint32_t
	{
		TS_Albedo,				// ��������ͼ����������ģ�͵�������ɫ��
		TS_Metalness,			// ��������ͼ;  ����ģ�͵Ľ����߹ⷴ�䡣
		TS_Glossiness,			// �������ͼ��ģ����ĳ���Ƕȿ��������й���
		TS_Emissive,				// �Է�����ͼ����ģ���Է��⡣
		TS_Normal,				// ������ͼ����������ģ�͵�ϸ�ڡ�
		TS_Height,				// �Ӳ���ͼ����������е�һ����ͼ����

		TS_TypeCount
	};

	enum SurfaceDetailMode
	{
		SDM_Parallax = 0,
		SDM_FlatTessellation,
		SDM_SmoothTessellation
	};

	std::string m_strName;
	float4 m_f4Albedo;
	float m_fMetalness;
	float m_fGlossiness;
	float3 m_f3Emissive;
	std::array<std::string, TS_TypeCount> m_TexNames;

	// ����ֲ����
	bool transparent;
	float alpha_test;
	bool sss;
	bool two_sided;

	RenderMaterial::SurfaceDetailMode detail_mode;
	float2 height_offset_scale;
	float4 tess_factors;
};

RenderMaterialPtr SyncLoadRenderMaterial(const std::string strFileName, XMLNodePtr pFileParse = nullptr);
RenderMaterialPtr ASyncLoadRenderMaterial(const std::string strFileName, XMLNodePtr pFileParse = nullptr);
#endif//_MATERIAL_H_