#pragma once

#include <render/RenderLayout.h>
#include "D3D12Util.h"
#include <vector>

namespace RenderWorker
{

class D3D12RenderLayout final : public RenderLayout
{
public:
	D3D12RenderLayout() = default;

	void Active() const;

	std::vector<d3d12::VertexBufferDescription> const& VertexBufferDescs() const noexcept
	{
		return vb_descs_;
	}
	std::vector<d3d12::VertexAttribute> const& VertexAttributes() const noexcept
	{
		return attrs_;
	}
	std::vector<d3d12::Buffer*> const& VertexBuffers() const noexcept
	{
		return vbs_;
	}
	std::vector<uint32_t> const& VertexOffsets() const noexcept
	{
		return vb_offsets_;
	}

	d3d12::Buffer* IndexBuffer() const noexcept
	{
		return ib_;
	}
	d3d12::IndexElementSize IndexElementSize() const noexcept
	{
		return index_size_;
	}

private:
	mutable std::vector<d3d12::VertexBufferDescription> vb_descs_;
	mutable std::vector<d3d12::VertexAttribute> attrs_;
	mutable std::vector<d3d12::Buffer*> vbs_;
	mutable std::vector<uint32_t> vb_offsets_;
	mutable d3d12::Buffer* ib_{nullptr};
	mutable d3d12::IndexElementSize index_size_{d3d12::INDEXELEMENTSIZE_16BIT};
};

} // namespace RenderWorker
