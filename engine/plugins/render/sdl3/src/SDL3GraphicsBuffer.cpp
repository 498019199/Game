#include "SDL3GraphicsBuffer.h"
#include "SDL3RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <common/Log.h>
#include <cstring>

namespace RenderWorker
{

SDL3GraphicsBuffer::SDL3GraphicsBuffer(BufferUsage usage, uint32_t access_hint, SDL3BufferBind bind,
	uint32_t size_in_byte, uint32_t structure_byte_stride)
	: GraphicsBuffer(usage, access_hint, size_in_byte, structure_byte_stride), bind_(bind)
{
	cpu_shadow_.resize(size_in_byte, 0);
}

SDL3GraphicsBuffer::~SDL3GraphicsBuffer()
{
	DeleteHWResource();
}

void SDL3GraphicsBuffer::CopyToBuffer(GraphicsBuffer& target)
{
	CopyToSubBuffer(target, 0, 0, size_in_byte_);
}

void SDL3GraphicsBuffer::CopyToSubBuffer(GraphicsBuffer& target, uint32_t dst_offset, uint32_t src_offset, uint32_t size)
{
	COMMON_ASSERT(src_offset + size <= size_in_byte_);
	COMMON_ASSERT(dst_offset + size <= target.Size());

	auto& dst = checked_cast<SDL3GraphicsBuffer&>(target);
	dst.UpdateSubresource(dst_offset, size, cpu_shadow_.data() + src_offset);
}

void SDL3GraphicsBuffer::CreateHWResource(void const* init_data)
{
	DeleteHWResource();

	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUDevice* device = re.Device();
	if (!device)
	{
		LogError() << "[SDL3] CreateHWResource called before GPU device is ready" << std::endl;
		return;
	}

	if (init_data)
	{
		std::memcpy(cpu_shadow_.data(), init_data, size_in_byte_);
	}

	// SDL_GPU uniform data is pushed from CPU; constant buffers stay CPU-side only.
	if (bind_ == SDL3BufferBind::Constant)
	{
		created_ = true;
		return;
	}

	SDL_GPUBufferUsageFlags usage = (bind_ == SDL3BufferBind::Index) ? SDL_GPU_BUFFERUSAGE_INDEX
																	: SDL_GPU_BUFFERUSAGE_VERTEX;
	SDL_GPUBufferCreateInfo info{};
	info.usage = usage;
	info.size = size_in_byte_;
	buffer_ = SDL_CreateGPUBuffer(device, &info);
	SDL3Check(buffer_ != nullptr, "SDL_CreateGPUBuffer");
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

void SDL3GraphicsBuffer::DeleteHWResource()
{
	if (buffer_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			SDL_ReleaseGPUBuffer(device_, buffer_);
		}
		buffer_ = nullptr;
	}
	device_lifetime_.reset();
	device_ = nullptr;
	created_ = false;
}

bool SDL3GraphicsBuffer::HWResourceReady() const
{
	return created_;
}

void SDL3GraphicsBuffer::UpdateSubresource(uint32_t offset, uint32_t size, void const* data)
{
	COMMON_ASSERT(offset + size <= size_in_byte_);
	COMMON_ASSERT(data);
	std::memcpy(cpu_shadow_.data() + offset, data, size);
	if (buffer_)
	{
		Upload(offset, size, data);
	}
}

void* SDL3GraphicsBuffer::Map([[maybe_unused]] BufferAccess ba)
{
	mapped_ = true;
	return cpu_shadow_.data();
}

void SDL3GraphicsBuffer::Unmap()
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

	void SDL3GraphicsBuffer::Upload(uint32_t offset, uint32_t size, void const* data)
	{
		auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		SDL_GPUDevice* device = re.Device();
		if (!device || !buffer_ || !data || size == 0)
		{
			return;
		}

		SDL_GPUTransferBufferCreateInfo tb_info{};
		tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		tb_info.size = size;
		SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &tb_info);
		if (!transfer)
		{
			LogError() << "[SDL3] SDL_CreateGPUTransferBuffer failed: " << SDL_GetError() << std::endl;
			return;
		}

		void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
		if (!mapped)
		{
			LogError() << "[SDL3] SDL_MapGPUTransferBuffer failed: " << SDL_GetError() << std::endl;
			SDL_ReleaseGPUTransferBuffer(device, transfer);
			return;
		}
		std::memcpy(mapped, data, size);
		SDL_UnmapGPUTransferBuffer(device, transfer);

		// Prefer a dedicated submit for large/static uploads so transfer lifetime
		// is unambiguous. Frame CB is fine for small dynamic updates (Unmap).
		SDL_GPUCommandBuffer* frame_cmd = re.CurrentCommandBuffer();
		bool const prefer_owned = (size >= 256) || (frame_cmd == nullptr);
		bool const owned_cmd = prefer_owned;
		SDL_GPUCommandBuffer* cmd = prefer_owned ? nullptr : frame_cmd;
		if (!cmd)
		{
			cmd = SDL_AcquireGPUCommandBuffer(device);
			if (!cmd)
			{
				LogError() << "[SDL3] AcquireGPUCommandBuffer(upload) failed: " << SDL_GetError() << std::endl;
				SDL_ReleaseGPUTransferBuffer(device, transfer);
				return;
			}
		}

		// Copy pass cannot run during an active render pass.
		if (re.CurrentRenderPass())
		{
			re.EndRenderPass();
		}

		SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
		SDL_GPUTransferBufferLocation src{};
		src.transfer_buffer = transfer;
		src.offset = 0;
		SDL_GPUBufferRegion dst{};
		dst.buffer = buffer_;
		dst.offset = offset;
		dst.size = size;
		SDL_UploadToGPUBuffer(copy, &src, &dst, false);
		SDL_EndGPUCopyPass(copy);

		if (owned_cmd)
		{
			SDL_SubmitGPUCommandBuffer(cmd);
			// Ensure the GPU finished reading the transfer buffer before we release it.
			SDL_WaitForGPUIdle(device);
			SDL_ReleaseGPUTransferBuffer(device, transfer);
		}
		else
		{
			// Frame CB owns the timeline; release is deferred safely by SDL tracking.
			SDL_ReleaseGPUTransferBuffer(device, transfer);
		}
	}

} // namespace RenderWorker
