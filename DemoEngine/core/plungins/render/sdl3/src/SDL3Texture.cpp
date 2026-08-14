#include "SDL3Texture.h"
#include "SDL3RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <render/TexCompression.h>
#include <cstring>
#include <algorithm>

namespace RenderWorker
{

SDL3Texture::SDL3Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: Texture(type, sample_count, sample_quality, access_hint)
{
}

SDL3Texture::~SDL3Texture()
{
	DeleteHWResource();
}

void SDL3Texture::DeleteHWResource()
{
	if (texture_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			SDL_ReleaseGPUTexture(device_, texture_);
		}
		texture_ = nullptr;
		gpu_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	}
	device_lifetime_.reset();
	device_ = nullptr;
}

bool SDL3Texture::HWResourceReady() const
{
	return texture_ != nullptr;
}

uint32_t SDL3Texture::Width([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

uint32_t SDL3Texture::Height([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

uint32_t SDL3Texture::Depth([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

void SDL3Texture::CopyToTexture(Texture& target, TextureFilter filter)
{
	CopyToSubTexture2D(target, 0, 0, 0, 0, target.Width(0), target.Height(0), 0, 0, 0, 0, Width(0), Height(0), filter);
}

void SDL3Texture::CopyToSubTexture1D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("SDL3Texture1D not implemented in MVP");
}

void SDL3Texture::CopyToSubTexture2D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("CopyToSubTexture2D must be overridden");
}

void SDL3Texture::CopyToSubTexture3D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("SDL3Texture3D not implemented in MVP");
}

void SDL3Texture::CopyToSubTextureCube(Texture&, uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	LogError() << "[SDL3] CopyToSubTextureCube not supported on this texture type" << std::endl;
}

void SDL3Texture::UpdateSubresource1D(uint32_t, uint32_t, uint32_t, uint32_t, void const*)
{
	ZENGINE_UNREACHABLE("SDL3Texture1D not implemented in MVP");
}

void SDL3Texture::UpdateSubresource2D(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void const*, uint32_t)
{
	ZENGINE_UNREACHABLE("UpdateSubresource2D must be overridden");
}

void SDL3Texture::UpdateSubresource3D(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void const*, uint32_t, uint32_t)
{
	ZENGINE_UNREACHABLE("SDL3Texture3D not implemented in MVP");
}

void SDL3Texture::UpdateSubresourceCube(uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void const*, uint32_t)
{
	LogError() << "[SDL3] UpdateSubresourceCube not supported on this texture type" << std::endl;
}

void SDL3Texture::Map1D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, void*&)
{
	ZENGINE_UNREACHABLE("Map1D not implemented in MVP");
}

void SDL3Texture::Map2D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, void*&, uint32_t&)
{
	ZENGINE_UNREACHABLE("Map2D not implemented in MVP");
}

void SDL3Texture::Map3D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void*&, uint32_t&, uint32_t&)
{
	ZENGINE_UNREACHABLE("Map3D not implemented in MVP");
}

void SDL3Texture::MapCube(uint32_t, CubeFaces, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, void*&,
	uint32_t&)
{
	ZENGINE_UNREACHABLE("MapCube not implemented in MVP");
}

void SDL3Texture::Unmap1D(uint32_t, uint32_t) {}
void SDL3Texture::Unmap2D(uint32_t, uint32_t) {}
void SDL3Texture::Unmap3D(uint32_t, uint32_t) {}
void SDL3Texture::UnmapCube(uint32_t, CubeFaces, uint32_t) {}

void SDL3Texture::Upload2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width,
	uint32_t height, void const* data, uint32_t row_pitch)
{
	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUDevice* device = re.Device();
	if (!device || !texture_ || !data)
	{
		return;
	}

	uint32_t const block_width = BlockWidth(format_);
	uint32_t const block_height = BlockHeight(format_);
	uint32_t const block_bytes = BlockBytes(format_);
	uint32_t const block_rows = (height + block_height - 1) / block_height;
	uint32_t const upload_size = row_pitch * block_rows;
	SDL_GPUTransferBufferCreateInfo tb_info{};
	tb_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	tb_info.size = upload_size;
	SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &tb_info);
	if (!transfer)
	{
		LogError() << "[SDL3] texture transfer create failed: " << SDL_GetError() << std::endl;
		return;
	}

	void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
	if (!mapped)
	{
		SDL_ReleaseGPUTransferBuffer(device, transfer);
		return;
	}
	std::memcpy(mapped, data, upload_size);
	SDL_UnmapGPUTransferBuffer(device, transfer);

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd)
	{
		SDL_ReleaseGPUTransferBuffer(device, transfer);
		return;
	}

	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
	SDL_GPUTextureTransferInfo src{};
	src.transfer_buffer = transfer;
	src.offset = 0;
	src.pixels_per_row = (row_pitch / block_bytes) * block_width;
	src.rows_per_layer = height;

	SDL_GPUTextureRegion dst{};
	dst.texture = texture_;
	dst.mip_level = level;
	dst.layer = array_index;
	dst.x = x_offset;
	dst.y = y_offset;
	dst.z = 0;
	dst.w = width;
	dst.h = height;
	dst.d = 1;
	SDL_UploadToGPUTexture(copy, &src, &dst, false);
	SDL_EndGPUCopyPass(copy);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_ReleaseGPUTransferBuffer(device, transfer);
}

} // namespace RenderWorker
