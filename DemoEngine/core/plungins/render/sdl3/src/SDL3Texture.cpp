#include "SDL3Texture.h"
#include "SDL3RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
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
		auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		if (re.Device())
		{
			SDL_ReleaseGPUTexture(re.Device(), texture_);
		}
		texture_ = nullptr;
		gpu_format_ = SDL_GPU_TEXTUREFORMAT_INVALID;
	}
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

	uint32_t const bpp = NumFormatBytes(format_);
	uint32_t const upload_size = row_pitch * height;
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
	src.pixels_per_row = row_pitch / std::max(1u, bpp);
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

SDL3Texture2D::SDL3Texture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
	ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: SDL3Texture(TT_2D, sample_count, sample_quality, access_hint), width_(width), height_(height)
{
	if (0 == num_mip_maps)
	{
		num_mip_maps = 1;
		uint32_t w = width;
		uint32_t h = height;
		while ((w != 1) || (h != 1))
		{
			++num_mip_maps;
			w = std::max(1U, w / 2);
			h = std::max(1U, h / 2);
		}
	}
	mip_maps_num_ = num_mip_maps;
	array_size_ = array_size;
	format_ = format;
}

uint32_t SDL3Texture2D::Width(uint32_t level) const noexcept
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, width_ >> level);
}

uint32_t SDL3Texture2D::Height(uint32_t level) const noexcept
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, height_ >> level);
}

void SDL3Texture2D::CopyToTexture(Texture& target, TextureFilter filter)
{
	CopyToSubTexture2D(target, 0, 0, 0, 0, target.Width(0), target.Height(0), 0, 0, 0, 0, Width(0), Height(0), filter);
}

void SDL3Texture2D::CopyToSubTexture2D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
	[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
	[[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height, [[maybe_unused]] uint32_t src_array_index,
	[[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset,
	[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
{
	LogWarn() << "[SDL3] CopyToSubTexture2D not fully implemented in MVP" << std::endl;
}

void SDL3Texture2D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const* clear_value_hint)
{
	DeleteHWResource();

	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUDevice* device = re.Device();
	COMMON_ASSERT(device);

	SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	if (access_hint_ & EAH_GPU_Write)
	{
		usage |= SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
	}
	if (IsDepthFormat(format_))
	{
		usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
		if (access_hint_ & EAH_GPU_Read)
		{
			usage |= SDL_GPU_TEXTUREUSAGE_SAMPLER;
		}
	}

		SDL_GPUTextureCreateInfo info{};
		info.type = (array_size_ > 1) ? SDL_GPU_TEXTURETYPE_2D_ARRAY : SDL_GPU_TEXTURETYPE_2D;
		info.format = SDL3Mapping::MappingFormat(format_);
		// Apple Silicon often lacks D24S8; fall back to D32F(+S8) when needed.
		if (IsDepthFormat(format_))
		{
			SDL_GPUTextureFormat candidates[4]{};
			int n = 0;
			if (format_ == EF_D24S8)
			{
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
			}
			else if (format_ == EF_D32F)
			{
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT;
			}
			else if (format_ == EF_D16)
			{
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
				candidates[n++] = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
			}
			else
			{
				candidates[n++] = info.format;
			}
			info.format = candidates[0];
			for (int i = 0; i < n; ++i)
			{
				if (SDL_GPUTextureSupportsFormat(device, candidates[i], SDL_GPU_TEXTURETYPE_2D, usage))
				{
					info.format = candidates[i];
					break;
				}
			}
		}
		info.usage = usage;
		info.width = width_;
		info.height = height_;
		info.layer_count_or_depth = array_size_;
		info.num_levels = mip_maps_num_;
		info.sample_count = SDL_GPU_SAMPLECOUNT_1;

	// D3D12 stores OptimizedClearValue at create time; clears must match or warn #821.
	SDL_PropertiesID props = 0;
	if (IsDepthFormat(format_))
	{
		float const clear_depth = clear_value_hint ? (*clear_value_hint)[0] : 1.0f;
		Uint8 const clear_stencil =
			clear_value_hint ? static_cast<Uint8>((*clear_value_hint)[1]) : static_cast<Uint8>(0);
		props = SDL_CreateProperties();
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, clear_depth);
		SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, clear_stencil);
		info.props = props;
	}
	else if (clear_value_hint && (usage & SDL_GPU_TEXTUREUSAGE_COLOR_TARGET))
	{
		props = SDL_CreateProperties();
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, (*clear_value_hint)[0]);
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, (*clear_value_hint)[1]);
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, (*clear_value_hint)[2]);
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, (*clear_value_hint)[3]);
		info.props = props;
	}

	texture_ = SDL_CreateGPUTexture(device, &info);
	if (props)
	{
		SDL_DestroyProperties(props);
	}
	SDL3Check(texture_ != nullptr, "SDL_CreateGPUTexture");
	gpu_format_ = info.format;

	if (!init_data.empty() && texture_)
	{
		COMMON_ASSERT(init_data.size() >= array_size_ * mip_maps_num_);
		for (uint32_t a = 0; a < array_size_; ++a)
		{
			for (uint32_t m = 0; m < mip_maps_num_; ++m)
			{
				auto const& sub = init_data[a * mip_maps_num_ + m];
				if (sub.data)
				{
					Upload2D(a, m, 0, 0, Width(m), Height(m), sub.data, sub.row_pitch);
				}
			}
		}
	}
}

void SDL3Texture2D::UpdateSubresource2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset,
	uint32_t width, uint32_t height, void const* data, uint32_t row_pitch)
{
	Upload2D(array_index, level, x_offset, y_offset, width, height, data, row_pitch);
}

void SDL3Texture2D::CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face,
	uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
	uint32_t src_array_index, [[maybe_unused]] CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset,
	uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, [[maybe_unused]] TextureFilter filter)
{
	COMMON_ASSERT(TT_Cube == target.Type());
	auto* dst_cube = dynamic_cast<SDL3TextureCube*>(&target);
	if (!dst_cube || !texture_ || !dst_cube->GpuTexture())
	{
		LogError() << "[SDL3] CopyToSubTextureCube requires SDL3TextureCube destination" << std::endl;
		return;
	}
	if ((src_width != dst_width) || (src_height != dst_height) || (format_ != target.Format()))
	{
		LogError() << "[SDL3] CopyToSubTextureCube: size/format mismatch not supported yet" << std::endl;
		return;
	}

	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUDevice* device = re.Device();
	if (!device)
	{
		return;
	}

	SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
	if (!cmd)
	{
		return;
	}
	SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(cmd);
	SDL_GPUTextureLocation src{};
	src.texture = texture_;
	src.mip_level = src_level;
	src.layer = src_array_index;
	src.x = src_x_offset;
	src.y = src_y_offset;
	src.z = 0;

	SDL_GPUTextureLocation dst{};
	dst.texture = dst_cube->GpuTexture();
	dst.mip_level = dst_level;
	dst.layer = dst_array_index * 6 + (dst_face - CF_Positive_X);
	dst.x = dst_x_offset;
	dst.y = dst_y_offset;
	dst.z = 0;

	SDL_CopyGPUTextureToTexture(copy, &src, &dst, dst_width, dst_height, 1, false);
	SDL_EndGPUCopyPass(copy);
	SDL_SubmitGPUCommandBuffer(cmd);
}

SDL3TextureCube::SDL3TextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size, ElementFormat format,
	uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: SDL3Texture(TT_Cube, sample_count, sample_quality, access_hint), size_(size)
{
	if (0 == num_mip_maps)
	{
		num_mip_maps = 1;
		uint32_t w = size;
		while (w != 1)
		{
			++num_mip_maps;
			w = std::max(1U, w / 2);
		}
	}
	mip_maps_num_ = num_mip_maps;
	array_size_ = array_size;
	format_ = format;
}

uint32_t SDL3TextureCube::Width(uint32_t level) const noexcept
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, size_ >> level);
}

uint32_t SDL3TextureCube::Height(uint32_t level) const noexcept
{
	return Width(level);
}

void SDL3TextureCube::CreateHWResource(std::span<ElementInitData const> init_data, float4 const* /*clear_value_hint*/)
{
	DeleteHWResource();

	auto& re = checked_cast<SDL3RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SDL_GPUDevice* device = re.Device();
	COMMON_ASSERT(device);

	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_CUBE;
	info.format = SDL3Mapping::MappingFormat(format_);
	info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	info.width = size_;
	info.height = size_;
	info.layer_count_or_depth = 6 * array_size_;
	info.num_levels = mip_maps_num_;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;

	texture_ = SDL_CreateGPUTexture(device, &info);
	SDL3Check(texture_ != nullptr, "SDL_CreateGPUTexture(cube)");
	gpu_format_ = info.format;

	if (!init_data.empty() && texture_)
	{
		COMMON_ASSERT(init_data.size() >= array_size_ * 6 * mip_maps_num_);
		for (uint32_t a = 0; a < array_size_; ++a)
		{
			for (uint32_t face = 0; face < 6; ++face)
			{
				for (uint32_t m = 0; m < mip_maps_num_; ++m)
				{
					auto const& sub = init_data[(a * 6 + face) * mip_maps_num_ + m];
					if (sub.data)
					{
						Upload2D(a * 6 + face, m, 0, 0, Width(m), Height(m), sub.data, sub.row_pitch);
					}
				}
			}
		}
	}
}

void SDL3TextureCube::UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level, uint32_t x_offset,
	uint32_t y_offset, uint32_t width, uint32_t height, void const* data, uint32_t row_pitch)
{
	Upload2D(array_index * 6 + (face - CF_Positive_X), level, x_offset, y_offset, width, height, data, row_pitch);
}

} // namespace RenderWorker
