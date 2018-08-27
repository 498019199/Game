#include "../Render/RenderLayout.h"
#include "../Util/UtilTool.h"
RenderLayout::RenderLayout()
{
	m_Vertexs = MakeSharedPtr<VertexVec>();
}

void RenderLayout::BindIndexStream(uint32_t nLods, const std::vector<int3>& Indecs)
{
	m_VertexsIndex.resize(nLods);
	memcpy(&m_VertexsIndex[0], &Indecs[0], nLods * sizeof(int3));
}

void RenderLayout::BindVertexStream(const VertexBuffer& vb)
{
	m_Vertexs = vb;
}

const std::vector<int3>& RenderLayout::GetIndexStream() const
{
	return m_VertexsIndex;
}

VertexBuffer RenderLayout::GetVertexStream() const
{
	return m_Vertexs;
}

