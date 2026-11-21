#include "D3D12GraphicsBuffer.h"
#include "D3D12RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <common/Log.h>
#include <cstring>

namespace RenderWorker
{

D3D12GraphicsBuffer::D3D12GraphicsBuffer(BufferUsage usage, uint32_t access_hint, D3D12BufferBind bind,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
	: GraphicsBuffer(usage, access_hint, size_in_byte, structure_byte_stride), bind_(bind)
{
	cpu_shadow_.resize(size_in_byte, 0);
}

D3D12GraphicsBuffer::~D3D12GraphicsBuffer()
{
	DeleteHWResource();
}

void D3D12GraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
{
	CopyToSubBuffer(target, 0, 0, size_in_byte_);
}

void D3D12GraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size)
{
	COMMON_ASSERT(src_offset + size <= size_in_byte_);
	COMMON_ASSERT(dst_offset + size <= target.Size());

	auto& dst = checked_cast<D3D12GraphicsBuffer&>(target);
	dst.UpdateSubresource(dst_offset, size, cpu_shadow_.data() + src_offset);
}

void D3D12GraphicsBuffer::CreateHWResource(void const* init_data)
{
	DeleteHWResource();

	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	d3d12::Device* device = re.Device();
	if (!device)
	{
		LogError() << "[D3D12] CreateHWResource called before GPU device is ready" << std::endl;
		return;
	}

	if (init_data)
	{
		std::memcpy(cpu_shadow_.data(), init_data, size_in_byte_);
	}

	// d3d12:: uniform data is pushed from CPU; constant buffers stay CPU-side only.
	if (bind_ == D3D12BufferBind::Constant)
	{
		created_ = true;
		return;
	}

	d3d12::BufferUsageFlags usage = (bind_ == D3D12BufferBind::Index) ? d3d12::BUFFERUSAGE_INDEX
																	: d3d12::BUFFERUSAGE_VERTEX;
	d3d12::BufferCreateInfo info{};
	info.usage = usage;
	info.size = size_in_byte_;
	buffer_ = d3d12::CreateBuffer(device, &info);
	D3D12Check(buffer_ != nullptr, "d3d12::CreateBuffer");
	device_ = buffer_ ? device : nullptr;
	device_lifetime_ = buffer_ ? re.DeviceLifetime() : nullptr;
	created_ = buffer_ != nullptr;

	if (init_data && buffer_)
	{
		// Always upload from the CPU shadow and wait on a dedicated CB so
		// delay-created mesh VBs are visible before the first draw.
		Upload(0, size_in_byte_, cpu_shadow_.data());
	}
}

void D3D12GraphicsBuffer::DeleteHWResource()
{
	if (buffer_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			d3d12::ReleaseBuffer(device_, buffer_);
		}
		buffer_ = nullptr;
	}
	device_lifetime_.reset();
	device_ = nullptr;
	created_ = false;
}

bool D3D12GraphicsBuffer::HWResourceReady() const
{
	return created_;
}

void D3D12GraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const* data)
{
	COMMON_ASSERT(offset + size <= size_in_byte_);
	COMMON_ASSERT(data);
	std::memcpy(cpu_shadow_.data() + offset, data, size);
	if (buffer_)
	{
		Upload(offset, size, data);
	}
}

void* D3D12GraphicsBuffer::Map([[maybe_unused]] BufferAccess ba)
{
	mapped_ = true;
	return cpu_shadow_.data();
}

void D3D12GraphicsBuffer::Unmap()
{
	if (mapped_)
	{
		mapped_ = false;
		if (buffer_)
		{
			Upload(0, size_in_byte_, cpu_shadow_.data());
		}
	}
}

	void D3D12GraphicsBuffer::Upload(uint32_t offset, uint32_t size, void const* data)
	{
		auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		d3d12::Device* device = re.Device();
		if (!device || !buffer_ || !data || size == 0)
		{
			return;
		}

		d3d12::TransferBufferCreateInfo tb_info{};
		tb_info.usage = d3d12::TRANSFERBUFFERUSAGE_UPLOAD;
		tb_info.size = size;
		d3d12::TransferBuffer* transfer = d3d12::CreateTransferBuffer(device, &tb_info);
		if (!transfer)
		{
			LogError() << "[D3D12] d3d12::CreateTransferBuffer failed: " << d3d12::LastError() << std::endl;
			return;
		}

		void* mapped = d3d12::MapTransferBuffer(device, transfer, false);
		if (!mapped)
		{
			LogError() << "[D3D12] d3d12::MapTransferBuffer failed: " << d3d12::LastError() << std::endl;
			d3d12::ReleaseTransferBuffer(device, transfer);
			return;
		}
		std::memcpy(mapped, data, size);
		d3d12::UnmapTransferBuffer(device, transfer);

		// Prefer a dedicated submit for large/static uploads so transfer lifetime
		// is unambiguous. Frame CB is fine for small dynamic updates (Unmap).
		d3d12::CommandBuffer* frame_cmd = re.CurrentCommandBuffer();
		bool const prefer_owned = (size >= 256) || (frame_cmd == nullptr);
		bool const owned_cmd = prefer_owned;
		d3d12::CommandBuffer* cmd = prefer_owned ? nullptr : frame_cmd;
		if (!cmd)
		{
			cmd = d3d12::AcquireCommandBuffer(device);
			if (!cmd)
			{
				LogError() << "[D3D12] AcquireGPUCommandBuffer(upload) failed: " << d3d12::LastError() << std::endl;
				d3d12::ReleaseTransferBuffer(device, transfer);
				return;
			}
		}

		// Copy pass cannot run during an active render pass.
		if (re.CurrentRenderPass())
		{
			re.EndRenderPass();
		}

		d3d12::CopyPass* copy = d3d12::BeginCopyPass(cmd);
		d3d12::TransferBufferLocation src{};
		src.transfer_buffer = transfer;
		src.offset = 0;
		d3d12::BufferRegion dst{};
		dst.buffer = buffer_;
		dst.offset = offset;
		dst.size = size;
		d3d12::UploadToBuffer(copy, &src, &dst, false);
		d3d12::EndCopyPass(copy);

		if (owned_cmd)
		{
			d3d12::SubmitCommandBuffer(cmd);
			// Ensure the GPU finished reading the transfer buffer before we release it.
			d3d12::WaitForIdle(device);
			d3d12::ReleaseTransferBuffer(device, transfer);
		}
		else
		{
			// Frame CB owns the timeline; release is deferred safely by the command buffer.
			d3d12::ReleaseTransferBuffer(device, transfer);
		}
	}

} // namespace RenderWorker
