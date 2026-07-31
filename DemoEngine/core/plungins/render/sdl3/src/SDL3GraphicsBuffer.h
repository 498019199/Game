#pragma once

#include <render/GraphicsBuffer.h>
#include "SDL3Util.h"
#include <vector>

namespace RenderWorker
{

enum class SDL3BufferBind : uint32_t
{
	Vertex = 1,
	Index = 2,
	Constant = 4,
};

class SDL3GraphicsBuffer final : public GraphicsBuffer
{
public:
	SDL3GraphicsBuffer(BufferUsage usage, uint32_t access_hint, SDL3BufferBind bind,
		uint32_t size_in_byte, uint32_t structure_byte_stride);
	~SDL3GraphicsBuffer() override;

	void CopyToBuffer(GraphicsBuffer& target) override;
	void CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

	void CreateHWResource(void const* init_data) override;
	void DeleteHWResource() override;
	bool HWResourceReady() const override;

	void UpdateSubresource(uint32_t offset, uint32_t size, void const* data) override;

	SDL_GPUBuffer* GpuBuffer() const noexcept
	{
		return buffer_;
	}

	uint8_t const* CpuData() const noexcept
	{
		return cpu_shadow_.data();
	}

	SDL3BufferBind BindFlags() const noexcept
	{
		return bind_;
	}

private:
	void* Map(BufferAccess ba) override;
	void Unmap() override;

	void Upload(uint32_t offset, uint32_t size, void const* data);

private:
	SDL3BufferBind bind_;
	SDL_GPUBuffer* buffer_{nullptr};
	std::vector<uint8_t> cpu_shadow_;
	bool mapped_{false};
	bool created_{false};
};

} // namespace RenderWorker
