#pragma once

#include <render/GraphicsBuffer.h>
#include "MetalUtil.h"
#include <vector>

namespace RenderWorker
{

struct MetalDeviceLifetime;

enum class MetalBufferBind : uint32_t
{
	Vertex = 1,
	Index = 2,
	Constant = 4,
};

class MetalGraphicsBuffer final : public GraphicsBuffer
{
public:
	MetalGraphicsBuffer(BufferUsage usage, uint32_t access_hint, MetalBufferBind bind,
		uint32_t size_in_byte, uint32_t structure_byte_stride);
	~MetalGraphicsBuffer() override;

	void CopyToBuffer(GraphicsBuffer& target) override;
	void CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

	void CreateHWResource(void const* init_data) override;
	void DeleteHWResource() override;
	bool HWResourceReady() const override;

	void UpdateSubresource(uint32_t offset, uint32_t size, void const* data) override;

	metal::Buffer* GpuBuffer() const noexcept
	{
		return buffer_;
	}

	uint8_t const* CpuData() const noexcept
	{
		return cpu_shadow_.data();
	}

	MetalBufferBind BindFlags() const noexcept
	{
		return bind_;
	}

private:
	void* Map(BufferAccess ba) override;
	void Unmap() override;

	void Upload(uint32_t offset, uint32_t size, void const* data);

private:
	MetalBufferBind bind_;
	std::shared_ptr<MetalDeviceLifetime> device_lifetime_;
	metal::Device* device_{nullptr};
	metal::Buffer* buffer_{nullptr};
	std::vector<uint8_t> cpu_shadow_;
	bool mapped_{false};
	bool created_{false};
};

} // namespace RenderWorker
