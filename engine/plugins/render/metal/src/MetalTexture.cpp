#include "MetalTexture.h"
#include "MetalRenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <render/TexCompression.h>
#include <cstring>
#include <algorithm>

namespace RenderWorker
{

MetalTexture::MetalTexture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: Texture(type, sample_count, sample_quality, access_hint)
{
}

MetalTexture::~MetalTexture()
{
	DeleteHWResource();
}

void MetalTexture::DeleteHWResource()
{
	if (texture_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			metal::ReleaseTexture(device_, texture_);
		}
		texture_ = nullptr;
		gpu_format_ = metal::TEXTUREFORMAT_INVALID;
	}
	device_lifetime_.reset();
	device_ = nullptr;
}

bool MetalTexture::HWResourceReady() const
{
	return texture_ != nullptr;
}

uint32_t MetalTexture::Width([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

uint32_t MetalTexture::Height([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

uint32_t MetalTexture::Depth([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

void MetalTexture::CopyToTexture(Texture& target, TextureFilter filter)
{
	CopyToSubTexture2D(target, 0, 0, 0, 0, target.Width(0), target.Height(0), 0, 0, 0, 0, Width(0), Height(0), filter);
}

void MetalTexture::CopyToSubTexture1D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("MetalTexture1D not implemented in MVP");
}

void MetalTexture::CopyToSubTexture2D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("CopyToSubTexture2D must be overridden");
}

void MetalTexture::CopyToSubTexture3D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("MetalTexture3D not implemented in MVP");
}

void MetalTexture::CopyToSubTextureCube(Texture&, uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	LogError() << "[Metal] CopyToSubTextureCube not supported on this texture type" << std::endl;
}

void MetalTexture::UpdateSubresource1D(uint32_t, uint32_t, uint32_t, uint32_t, void const*)
{
	ZENGINE_UNREACHABLE("MetalTexture1D not implemented in MVP");
}

void MetalTexture::UpdateSubresource2D(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void const*, uint32_t)
{
	ZENGINE_UNREACHABLE("UpdateSubresource2D must be overridden");
}

void MetalTexture::UpdateSubresource3D(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void const*, uint32_t, uint32_t)
{
	ZENGINE_UNREACHABLE("MetalTexture3D not implemented in MVP");
}

void MetalTexture::UpdateSubresourceCube(uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void const*, uint32_t)
{
	LogError() << "[Metal] UpdateSubresourceCube not supported on this texture type" << std::endl;
}

void MetalTexture::Map1D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, void*&)
{
	ZENGINE_UNREACHABLE("Map1D not implemented in MVP");
}

void MetalTexture::Map2D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, void*&, uint32_t&)
{
	ZENGINE_UNREACHABLE("Map2D not implemented in MVP");
}

void MetalTexture::Map3D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void*&, uint32_t&, uint32_t&)
{
	ZENGINE_UNREACHABLE("Map3D not implemented in MVP");
}

void MetalTexture::MapCube(uint32_t, CubeFaces, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, void*&,
	uint32_t&)
{
	ZENGINE_UNREACHABLE("MapCube not implemented in MVP");
}

void MetalTexture::Unmap1D(uint32_t, uint32_t) {}
void MetalTexture::Unmap2D(uint32_t, uint32_t) {}
void MetalTexture::Unmap3D(uint32_t, uint32_t) {}
void MetalTexture::UnmapCube(uint32_t, CubeFaces, uint32_t) {}

void MetalTexture::Upload2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width,
	uint32_t height, void const* data, uint32_t row_pitch)
{
	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	metal::Device* device = re.Device();
	if (!device || !texture_ || !data)
	{
		return;
	}

	uint32_t const block_width = BlockWidth(format_);
	uint32_t const block_height = BlockHeight(format_);
	uint32_t const block_bytes = BlockBytes(format_);
	uint32_t const block_rows = (height + block_height - 1) / block_height;
	uint32_t const upload_size = row_pitch * block_rows;
	metal::TransferBufferCreateInfo tb_info{};
	tb_info.usage = metal::TRANSFERBUFFERUSAGE_UPLOAD;
	tb_info.size = upload_size;
	metal::TransferBuffer* transfer = metal::CreateTransferBuffer(device, &tb_info);
	if (!transfer)
	{
		LogError() << "[Metal] texture transfer create failed: " << metal::LastError() << std::endl;
		return;
	}

	void* mapped = metal::MapTransferBuffer(device, transfer, false);
	if (!mapped)
	{
		metal::ReleaseTransferBuffer(device, transfer);
		return;
	}
	std::memcpy(mapped, data, upload_size);
	metal::UnmapTransferBuffer(device, transfer);

	metal::CommandBuffer* cmd = metal::AcquireCommandBuffer(device);
	if (!cmd)
	{
		metal::ReleaseTransferBuffer(device, transfer);
		return;
	}

	metal::CopyPass* copy = metal::BeginCopyPass(cmd);
	metal::TextureTransferInfo src{};
	src.transfer_buffer = transfer;
	src.offset = 0;
	src.pixels_per_row = (row_pitch / block_bytes) * block_width;
	src.rows_per_layer = height;

	metal::TextureRegion dst{};
	dst.texture = texture_;
	dst.mip_level = level;
	dst.layer = array_index;
	dst.x = x_offset;
	dst.y = y_offset;
	dst.z = 0;
	dst.w = width;
	dst.h = height;
	dst.d = 1;
	metal::UploadToTexture(copy, &src, &dst, false);
	metal::EndCopyPass(copy);
	metal::SubmitCommandBuffer(cmd);
	metal::ReleaseTransferBuffer(device, transfer);
}

} // namespace RenderWorker
