#pragma once

#include <render/GraphicsBuffer.h>
#include "D3D12Util.h"
#include <vector>

namespace RenderWorker
{

struct D3D12DeviceLifetime;

enum class D3D12BufferBind : uint32_t
{
	Vertex = 1,
	Index = 2,
	Constant = 4,
};

class D3D12GraphicsBuffer final : public GraphicsBuffer
{
public:
	D3D12GraphicsBuffer(BufferUsage usage, uint32_t access_hint, D3D12BufferBind bind,
		uint32_t size_in_byte, uint32_t structure_byte_stride);
	~D3D12GraphicsBuffer() override;

	void CopyToBuffer(GraphicsBuffer& target) override;
	void CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size) override;

	void CreateHWResource(void const* init_data) override;
	void DeleteHWResource() override;
	bool HWResourceReady() const override;

	void UpdateSubresource(uint32_t offset, uint32_t size, void const* data) override;

	d3d12::Buffer* GpuBuffer() const noexcept
	{
		return buffer_;
	}

	uint8_t const* CpuData() const noexcept
	{
		return cpu_shadow_.data();
	}

	D3D12BufferBind BindFlags() const noexcept
	{
		return bind_;
	}

private:
	void* Map(BufferAccess ba) override;
	void Unmap() override;

	void Upload(uint32_t offset, uint32_t size, void const* data);

private:
	D3D12BufferBind bind_;
	std::shared_ptr<D3D12DeviceLifetime> device_lifetime_;
	d3d12::Device* device_{nullptr};
	d3d12::Buffer* buffer_{nullptr};
	std::vector<uint8_t> cpu_shadow_;
	bool mapped_{false};
	bool created_{false};
};

} // namespace RenderWorker
