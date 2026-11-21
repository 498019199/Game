#include "MetalGraphicsBuffer.h"
#include "MetalRenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <common/Log.h>
#include <cstring>

namespace RenderWorker
{

MetalGraphicsBuffer::MetalGraphicsBuffer(BufferUsage usage, uint32_t access_hint, MetalBufferBind bind,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
	: GraphicsBuffer(usage, access_hint, size_in_byte, structure_byte_stride), bind_(bind)
{
	cpu_shadow_.resize(size_in_byte, 0);
}

MetalGraphicsBuffer::~MetalGraphicsBuffer()
{
	DeleteHWResource();
}

void MetalGraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
{
	CopyToSubBuffer(target, 0, 0, size_in_byte_);
}

void MetalGraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size)
{
	COMMON_ASSERT(src_offset + size <= size_in_byte_);
	COMMON_ASSERT(dst_offset + size <= target.Size());

	auto& dst = checked_cast<MetalGraphicsBuffer&>(target);
	dst.UpdateSubresource(dst_offset, size, cpu_shadow_.data() + src_offset);
}

void MetalGraphicsBuffer::CreateHWResource(void const* init_data)
{
	DeleteHWResource();

	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	metal::Device* device = re.Device();
	if (!device)
	{
		LogError() << "[Metal] CreateHWResource called before GPU device is ready" << std::endl;
		return;
	}

	if (init_data)
	{
		std::memcpy(cpu_shadow_.data(), init_data, size_in_byte_);
	}

	// metal:: uniform data is pushed from CPU; constant buffers stay CPU-side only.
	if (bind_ == MetalBufferBind::Constant)
	{
		created_ = true;
		return;
	}

	metal::BufferUsageFlags usage = (bind_ == MetalBufferBind::Index) ? metal::BUFFERUSAGE_INDEX
																	: metal::BUFFERUSAGE_VERTEX;
	metal::BufferCreateInfo info{};
	info.usage = usage;
	info.size = size_in_byte_;
	buffer_ = metal::CreateBuffer(device, &info);
	MetalCheck(buffer_ != nullptr, "metal::CreateBuffer");
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

void MetalGraphicsBuffer::DeleteHWResource()
{
	if (buffer_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			metal::ReleaseBuffer(device_, buffer_);
		}
		buffer_ = nullptr;
	}
	device_lifetime_.reset();
	device_ = nullptr;
	created_ = false;
}

bool MetalGraphicsBuffer::HWResourceReady() const
{
	return created_;
}

void MetalGraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const* data)
{
	COMMON_ASSERT(offset + size <= size_in_byte_);
	COMMON_ASSERT(data);
	std::memcpy(cpu_shadow_.data() + offset, data, size);
	if (buffer_)
	{
		Upload(offset, size, data);
	}
}

void* MetalGraphicsBuffer::Map([[maybe_unused]] BufferAccess ba)
{
	mapped_ = true;
	return cpu_shadow_.data();
}

void MetalGraphicsBuffer::Unmap()
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

	void MetalGraphicsBuffer::Upload(uint32_t offset, uint32_t size, void const* data)
	{
		auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		metal::Device* device = re.Device();
		if (!device || !buffer_ || !data || size == 0)
		{
			return;
		}

		metal::TransferBufferCreateInfo tb_info{};
		tb_info.usage = metal::TRANSFERBUFFERUSAGE_UPLOAD;
		tb_info.size = size;
		metal::TransferBuffer* transfer = metal::CreateTransferBuffer(device, &tb_info);
		if (!transfer)
		{
			LogError() << "[Metal] metal::CreateTransferBuffer failed: " << metal::LastError() << std::endl;
			return;
		}

		void* mapped = metal::MapTransferBuffer(device, transfer, false);
		if (!mapped)
		{
			LogError() << "[Metal] metal::MapTransferBuffer failed: " << metal::LastError() << std::endl;
			metal::ReleaseTransferBuffer(device, transfer);
			return;
		}
		std::memcpy(mapped, data, size);
		metal::UnmapTransferBuffer(device, transfer);

		// Prefer a dedicated submit for large/static uploads so transfer lifetime
		// is unambiguous. Frame CB is fine for small dynamic updates (Unmap).
		metal::CommandBuffer* frame_cmd = re.CurrentCommandBuffer();
		bool const prefer_owned = (size >= 256) || (frame_cmd == nullptr);
		bool const owned_cmd = prefer_owned;
		metal::CommandBuffer* cmd = prefer_owned ? nullptr : frame_cmd;
		if (!cmd)
		{
			cmd = metal::AcquireCommandBuffer(device);
			if (!cmd)
			{
				LogError() << "[Metal] AcquireGPUCommandBuffer(upload) failed: " << metal::LastError() << std::endl;
				metal::ReleaseTransferBuffer(device, transfer);
				return;
			}
		}

		// Copy pass cannot run during an active render pass.
		if (re.CurrentRenderPass())
		{
			re.EndRenderPass();
		}

		metal::CopyPass* copy = metal::BeginCopyPass(cmd);
		metal::TransferBufferLocation src{};
		src.transfer_buffer = transfer;
		src.offset = 0;
		metal::BufferRegion dst{};
		dst.buffer = buffer_;
		dst.offset = offset;
		dst.size = size;
		metal::UploadToBuffer(copy, &src, &dst, false);
		metal::EndCopyPass(copy);

		if (owned_cmd)
		{
			metal::SubmitCommandBuffer(cmd);
			// Ensure the GPU finished reading the transfer buffer before we release it.
			metal::WaitForIdle(device);
			metal::ReleaseTransferBuffer(device, transfer);
		}
		else
		{
			// Frame CB owns the timeline; release is deferred safely by the command buffer.
			metal::ReleaseTransferBuffer(device, transfer);
		}
	}

} // namespace RenderWorker
