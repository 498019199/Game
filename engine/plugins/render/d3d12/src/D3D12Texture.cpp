#include "D3D12Texture.h"
#include "D3D12RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <render/TexCompression.h>
#include <cstring>
#include <algorithm>

namespace RenderWorker
{

D3D12Texture::D3D12Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: Texture(type, sample_count, sample_quality, access_hint)
{
}

D3D12Texture::~D3D12Texture()
{
	DeleteHWResource();
}

void D3D12Texture::DeleteHWResource()
{
	if (texture_)
	{
		if (device_lifetime_ && (device_lifetime_->device == device_))
		{
			d3d12::ReleaseTexture(device_, texture_);
		}
		texture_ = nullptr;
		gpu_format_ = d3d12::TEXTUREFORMAT_INVALID;
	}
	device_lifetime_.reset();
	device_ = nullptr;
}

bool D3D12Texture::HWResourceReady() const
{
	return texture_ != nullptr;
}

uint32_t D3D12Texture::Width([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

uint32_t D3D12Texture::Height([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

uint32_t D3D12Texture::Depth([[maybe_unused]] uint32_t level) const noexcept
{
	return 1;
}

void D3D12Texture::CopyToTexture(Texture& target, TextureFilter filter)
{
	CopyToSubTexture2D(target, 0, 0, 0, 0, target.Width(0), target.Height(0), 0, 0, 0, 0, Width(0), Height(0), filter);
}

void D3D12Texture::CopyToSubTexture1D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("D3D12Texture1D not implemented in MVP");
}

void D3D12Texture::CopyToSubTexture2D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("CopyToSubTexture2D must be overridden");
}

void D3D12Texture::CopyToSubTexture3D(Texture&, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	ZENGINE_UNREACHABLE("D3D12Texture3D not implemented in MVP");
}

void D3D12Texture::CopyToSubTextureCube(Texture&, uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, TextureFilter)
{
	LogError() << "[D3D12] CopyToSubTextureCube not supported on this texture type" << std::endl;
}

void D3D12Texture::UpdateSubresource1D(uint32_t, uint32_t, uint32_t, uint32_t, void const*)
{
	ZENGINE_UNREACHABLE("D3D12Texture1D not implemented in MVP");
}

void D3D12Texture::UpdateSubresource2D(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, void const*, uint32_t)
{
	ZENGINE_UNREACHABLE("UpdateSubresource2D must be overridden");
}

void D3D12Texture::UpdateSubresource3D(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void const*, uint32_t, uint32_t)
{
	ZENGINE_UNREACHABLE("D3D12Texture3D not implemented in MVP");
}

void D3D12Texture::UpdateSubresourceCube(uint32_t, CubeFaces, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void const*, uint32_t)
{
	LogError() << "[D3D12] UpdateSubresourceCube not supported on this texture type" << std::endl;
}

void D3D12Texture::Map1D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, void*&)
{
	ZENGINE_UNREACHABLE("Map1D not implemented in MVP");
}

void D3D12Texture::Map2D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, void*&, uint32_t&)
{
	ZENGINE_UNREACHABLE("Map2D not implemented in MVP");
}

void D3D12Texture::Map3D(uint32_t, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
	void*&, uint32_t&, uint32_t&)
{
	ZENGINE_UNREACHABLE("Map3D not implemented in MVP");
}

void D3D12Texture::MapCube(uint32_t, CubeFaces, uint32_t, TextureMapAccess, uint32_t, uint32_t, uint32_t, uint32_t, void*&,
	uint32_t&)
{
	ZENGINE_UNREACHABLE("MapCube not implemented in MVP");
}

void D3D12Texture::Unmap1D(uint32_t, uint32_t) {}
void D3D12Texture::Unmap2D(uint32_t, uint32_t) {}
void D3D12Texture::Unmap3D(uint32_t, uint32_t) {}
void D3D12Texture::UnmapCube(uint32_t, CubeFaces, uint32_t) {}

void D3D12Texture::Upload2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width,
	uint32_t height, void const* data, uint32_t row_pitch)
{
	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	d3d12::Device* device = re.Device();
	if (!device || !texture_ || !data)
	{
		return;
	}

	uint32_t const block_width = BlockWidth(format_);
	uint32_t const block_height = BlockHeight(format_);
	uint32_t const block_bytes = BlockBytes(format_);
	uint32_t const block_rows = (height + block_height - 1) / block_height;
	uint32_t const upload_size = row_pitch * block_rows;
	d3d12::TransferBufferCreateInfo tb_info{};
	tb_info.usage = d3d12::TRANSFERBUFFERUSAGE_UPLOAD;
	tb_info.size = upload_size;
	d3d12::TransferBuffer* transfer = d3d12::CreateTransferBuffer(device, &tb_info);
	if (!transfer)
	{
		LogError() << "[D3D12] texture transfer create failed: " << d3d12::LastError() << std::endl;
		return;
	}

	void* mapped = d3d12::MapTransferBuffer(device, transfer, false);
	if (!mapped)
	{
		d3d12::ReleaseTransferBuffer(device, transfer);
		return;
	}
	std::memcpy(mapped, data, upload_size);
	d3d12::UnmapTransferBuffer(device, transfer);

	d3d12::CommandBuffer* cmd = d3d12::AcquireCommandBuffer(device);
	if (!cmd)
	{
		d3d12::ReleaseTransferBuffer(device, transfer);
		return;
	}

	d3d12::CopyPass* copy = d3d12::BeginCopyPass(cmd);
	d3d12::TextureTransferInfo src{};
	src.transfer_buffer = transfer;
	src.offset = 0;
	src.pixels_per_row = (row_pitch / block_bytes) * block_width;
	src.rows_per_layer = height;

	d3d12::TextureRegion dst{};
	dst.texture = texture_;
	dst.mip_level = level;
	dst.layer = array_index;
	dst.x = x_offset;
	dst.y = y_offset;
	dst.z = 0;
	dst.w = width;
	dst.h = height;
	dst.d = 1;
	d3d12::UploadToTexture(copy, &src, &dst, false);
	d3d12::EndCopyPass(copy);
	d3d12::SubmitCommandBuffer(cmd);
	d3d12::ReleaseTransferBuffer(device, transfer);
}

} // namespace RenderWorker
