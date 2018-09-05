// 2018年8月26日 zhangbei 顶点和索引缓存
#ifndef _STX_RENDER_LAYOUT_H_
#define _STX_RENDER_LAYOUT_H_
#pragma once
#include "Renderable.h"
#include <vector>
#include <boost/noncopyable.hpp>
class RenderLayout : boost::noncopyable
{
public:
	RenderLayout();

	void BindIndexStream(uint32_t nLods, const std::vector<int3>& Indecs);

	void BindVertexStream(const VertexBuffer& vb);

	const std::vector<int3>& GetIndexStream() const;

	VertexBuffer GetVertexStream() const;

	uint32_t GetTriCount() const { return m_nTriCount; }
private:
	VertexBuffer m_Vertexs; // 顶点集合
	uint32_t m_nTriCount;
	std::vector<int3> m_VertexsIndex;		// 顶点索引集合
};
#endif//_STX_RENDER_LAYOUT_H_
