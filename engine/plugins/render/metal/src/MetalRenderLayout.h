#pragma once

#include <render/RenderLayout.h>
#include "MetalUtil.h"
#include <vector>

namespace RenderWorker
{

class MetalRenderLayout final : public RenderLayout
{
public:
	MetalRenderLayout() = default;

	void Active() const;

	std::vector<metal::VertexBufferDescription> const& VertexBufferDescs() const noexcept
	{
		return vb_descs_;
	}
	std::vector<metal::VertexAttribute> const& VertexAttributes() const noexcept
	{
		return attrs_;
	}
	std::vector<metal::Buffer*> const& VertexBuffers() const noexcept
	{
		return vbs_;
	}
	std::vector<uint32_t> const& VertexOffsets() const noexcept
	{
		return vb_offsets_;
	}

	metal::Buffer* IndexBuffer() const noexcept
	{
		return ib_;
	}
	metal::IndexElementSize IndexElementSize() const noexcept
	{
		return index_size_;
	}

private:
	mutable std::vector<metal::VertexBufferDescription> vb_descs_;
	mutable std::vector<metal::VertexAttribute> attrs_;
	mutable std::vector<metal::Buffer*> vbs_;
	mutable std::vector<uint32_t> vb_offsets_;
	mutable metal::Buffer* ib_{nullptr};
	mutable metal::IndexElementSize index_size_{metal::INDEXELEMENTSIZE_16BIT};
};

} // namespace RenderWorker
