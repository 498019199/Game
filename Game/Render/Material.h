// 2018��7��8�� zhangbei ����
#ifndef _MATERIAL_H_
#define _MATERIAL_H_
#pragma once

#include "../Math/Math.h"
#include "../Core/predefine.h"
#include <string>
class Material
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

	std::string m_strName;
	float4 m_f4Albedo;
	float m_fMetalness;
	float m_fGlossiness;
	float3 m_f3Emissive;
	std::array<std::string, TS_TypeCount> m_TexNames;
};

MaterialPtr SyncLoadRenderMaterial(const std::string strFileName, XMLNodePtr pFileParse = nullptr);
MaterialPtr ASyncLoadRenderMaterial(const std::string strFileName, XMLNodePtr pFileParse = nullptr);
#endif//_MATERIAL_H_