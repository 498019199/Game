#pragma once

#include <render/RenderLayout.h>
#include "SDL3Util.h"
#include <vector>

namespace RenderWorker
{

class SDL3RenderLayout final : public RenderLayout
{
public:
	SDL3RenderLayout() = default;

	void Active() const;

	std::vector<SDL_GPUVertexBufferDescription> const& VertexBufferDescs() const noexcept
	{
		return vb_descs_;
	}
	std::vector<SDL_GPUVertexAttribute> const& VertexAttributes() const noexcept
	{
		return attrs_;
	}
	std::vector<SDL_GPUBuffer*> const& VertexBuffers() const noexcept
	{
		return vbs_;
	}
	std::vector<Uint32> const& VertexOffsets() const noexcept
	{
		return vb_offsets_;
	}

	SDL_GPUBuffer* IndexBuffer() const noexcept
	{
		return ib_;
	}
	SDL_GPUIndexElementSize IndexElementSize() const noexcept
	{
		return index_size_;
	}

private:
	mutable std::vector<SDL_GPUVertexBufferDescription> vb_descs_;
	mutable std::vector<SDL_GPUVertexAttribute> attrs_;
	mutable std::vector<SDL_GPUBuffer*> vbs_;
	mutable std::vector<Uint32> vb_offsets_;
	mutable SDL_GPUBuffer* ib_{nullptr};
	mutable SDL_GPUIndexElementSize index_size_{SDL_GPU_INDEXELEMENTSIZE_16BIT};
};

} // namespace RenderWorker
