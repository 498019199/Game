#include "SDL3Texture.h"
#include "SDL3RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <algorithm>

namespace RenderWorker
{

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
		// ZEngine clears depth targets to 1 / stencil 0. Keep D3D12's optimized
		// clear value identical for every SDL3 depth texture, including auxiliary
		// full-size depth targets whose generic clear hint may be zero.
		float const clear_depth = 1.0f;
		Uint8 const clear_stencil = 0;
		props = SDL_CreateProperties();
		SDL3Check(props != 0, "SDL_CreateProperties(depth texture)");
		SDL3Check(SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, clear_depth),
			"SDL_SetFloatProperty(D3D12 clear depth)");
		SDL3Check(SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, clear_stencil),
			"SDL_SetNumberProperty(D3D12 clear stencil)");
		info.props = props;
	}
	else if (clear_value_hint && (usage & SDL_GPU_TEXTUREUSAGE_COLOR_TARGET))
	{
		props = SDL_CreateProperties();
		SDL3Check(props != 0, "SDL_CreateProperties(color texture)");
		SDL3Check(SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, (*clear_value_hint)[0]),
			"SDL_SetFloatProperty(D3D12 clear R)");
		SDL3Check(SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, (*clear_value_hint)[1]),
			"SDL_SetFloatProperty(D3D12 clear G)");
		SDL3Check(SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, (*clear_value_hint)[2]),
			"SDL_SetFloatProperty(D3D12 clear B)");
		SDL3Check(SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, (*clear_value_hint)[3]),
			"SDL_SetFloatProperty(D3D12 clear A)");
		info.props = props;
	}

	texture_ = SDL_CreateGPUTexture(device, &info);
	if (props)
	{
		SDL_DestroyProperties(props);
	}
	SDL3Check(texture_ != nullptr, "SDL_CreateGPUTexture");
	device_ = texture_ ? device : nullptr;
	device_lifetime_ = texture_ ? re.DeviceLifetime() : nullptr;
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

} // namespace RenderWorker
