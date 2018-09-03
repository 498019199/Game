// 移植 KlayGE 元素格式 头文件 2018年9月2日
#ifndef _VERTEX_ELEMENT_H
#define _VERTEX_ELEMENT_H
#pragma once

#include "ElementFormat.hpp"
enum VertexElementUsage
{
	// vertex positions
	VEU_Position = 0,
	// vertex normals included (for lighting)
	VEU_Normal,
	// Vertex colors - diffuse
	VEU_Diffuse,
	// Vertex colors - specular
	VEU_Specular,
	// Vertex blend weights
	VEU_BlendWeight,
	// Vertex blend indices
	VEU_BlendIndex,
	// at least one set of texture coords (exact number specified in class)
	VEU_TextureCoord,
	// Vertex tangent
	VEU_Tangent,
	// Vertex binormal
	VEU_Binormal
};

struct VertexElement
{
	VertexElement()
	{
	}
	VertexElement(VertexElementUsage usage, uint8_t usage_index, ElementFormat format)
		: usage(usage), usage_index(usage_index), format(format)
	{
	}

	VertexElementUsage usage;
	uint8_t usage_index;

	ElementFormat format;

	uint16_t element_size() const
	{
		return NumFormatBytes(format);
	}

	friend bool
		operator==(VertexElement const & lhs, VertexElement const & rhs)
	{
		return (lhs.usage == rhs.usage)
			&& (lhs.usage_index == rhs.usage_index)
			&& (lhs.format == rhs.format);
	}
};
#endif //_VERTEX_ELEMENT_H