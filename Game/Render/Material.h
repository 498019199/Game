// 2018年7月8日 zhangbei 材质
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
		TS_Albedo,				// 反照率贴图；用于体现模型的纹理，颜色。
		TS_Metalness,			// 金属度贴图;  体现模型的金属高光反射。
		TS_Glossiness,			// 光泽度贴图；模型在某个角度看起来具有光泽。
		TS_Emissive,				// 自发光贴图；让模型自发光。
		TS_Normal,				// 法线贴图；用于增加模型的细节。
		TS_Height,				// 视差贴图；更有立体感的一种贴图方。

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