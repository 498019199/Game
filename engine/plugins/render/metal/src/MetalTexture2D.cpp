#include "MetalTexture.h"
#include "MetalRenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <algorithm>

namespace RenderWorker
{

MetalTexture2D::MetalTexture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size,
	ElementFormat format, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: MetalTexture(TT_2D, sample_count, sample_quality, access_hint), width_(width), height_(height)
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

uint32_t MetalTexture2D::Width(uint32_t level) const noexcept
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, width_ >> level);
}

uint32_t MetalTexture2D::Height(uint32_t level) const noexcept
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, height_ >> level);
}

void MetalTexture2D::CopyToTexture(Texture& target, TextureFilter filter)
{
	CopyToSubTexture2D(target, 0, 0, 0, 0, target.Width(0), target.Height(0), 0, 0, 0, 0, Width(0), Height(0), filter);
}

void MetalTexture2D::CopyToSubTexture2D([[maybe_unused]] Texture& target, [[maybe_unused]] uint32_t dst_array_index,
	[[maybe_unused]] uint32_t dst_level, [[maybe_unused]] uint32_t dst_x_offset, [[maybe_unused]] uint32_t dst_y_offset,
	[[maybe_unused]] uint32_t dst_width, [[maybe_unused]] uint32_t dst_height, [[maybe_unused]] uint32_t src_array_index,
	[[maybe_unused]] uint32_t src_level, [[maybe_unused]] uint32_t src_x_offset, [[maybe_unused]] uint32_t src_y_offset,
	[[maybe_unused]] uint32_t src_width, [[maybe_unused]] uint32_t src_height, [[maybe_unused]] TextureFilter filter)
{
	LogWarn() << "[Metal] CopyToSubTexture2D not fully implemented in MVP" << std::endl;
}

void MetalTexture2D::CreateHWResource(std::span<ElementInitData const> init_data, float4 const* clear_value_hint)
{
	DeleteHWResource();

	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	metal::Device* device = re.Device();
	COMMON_ASSERT(device);

	metal::TextureUsageFlags usage = metal::TEXTUREUSAGE_SAMPLER;
	if (access_hint_ & EAH_GPU_Write)
	{
		usage |= metal::TEXTUREUSAGE_COLOR_TARGET;
	}
	if (IsDepthFormat(format_))
	{
		usage = metal::TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
		if (access_hint_ & EAH_GPU_Read)
		{
			usage |= metal::TEXTUREUSAGE_SAMPLER;
		}
	}

	metal::TextureCreateInfo info{};
	info.type = (array_size_ > 1) ? metal::TEXTURETYPE_2D_ARRAY : metal::TEXTURETYPE_2D;
	info.format = MetalMapping::MappingFormat(format_);
	// Apple Silicon often lacks D24S8; fall back to D32F(+S8) when needed.
	if (IsDepthFormat(format_))
	{
		metal::TextureFormat candidates[4]{};
		int n = 0;
		if (format_ == EF_D24S8)
		{
			candidates[n++] = metal::TEXTUREFORMAT_D24_UNORM_S8_UINT;
			candidates[n++] = metal::TEXTUREFORMAT_D32_FLOAT_S8_UINT;
			candidates[n++] = metal::TEXTUREFORMAT_D32_FLOAT;
		}
		else if (format_ == EF_D32F)
		{
			candidates[n++] = metal::TEXTUREFORMAT_D32_FLOAT;
			candidates[n++] = metal::TEXTUREFORMAT_D32_FLOAT_S8_UINT;
		}
		else if (format_ == EF_D16)
		{
			candidates[n++] = metal::TEXTUREFORMAT_D16_UNORM;
			candidates[n++] = metal::TEXTUREFORMAT_D32_FLOAT;
		}
		else
		{
			candidates[n++] = info.format;
		}
		info.format = candidates[0];
		for (int i = 0; i < n; ++i)
		{
			if (metal::TextureSupportsFormat(device, candidates[i], metal::TEXTURETYPE_2D, usage))
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
	info.sample_count = metal::SAMPLECOUNT_1;

    if (clear_value_hint)
        for (unsigned i=0; i<4; ++i) info.clear_color[i] = (*clear_value_hint)[i];
    texture_ = metal::CreateTexture(device, &info);
	MetalCheck(texture_ != nullptr, "metal::CreateTexture");
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

void MetalTexture2D::UpdateSubresource2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset,
	uint32_t width, uint32_t height, void const* data, uint32_t row_pitch)
{
	Upload2D(array_index, level, x_offset, y_offset, width, height, data, row_pitch);
}

void MetalTexture2D::CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face,
	uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
	uint32_t src_array_index, [[maybe_unused]] CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset,
	uint32_t src_y_offset, uint32_t src_width, uint32_t src_height, [[maybe_unused]] TextureFilter filter)
{
	COMMON_ASSERT(TT_Cube == target.Type());
	auto* dst_cube = dynamic_cast<MetalTextureCube*>(&target);
	if (!dst_cube || !texture_ || !dst_cube->GpuTexture())
	{
		LogError() << "[Metal] CopyToSubTextureCube requires MetalTextureCube destination" << std::endl;
		return;
	}
	if ((src_width != dst_width) || (src_height != dst_height) || (format_ != target.Format()))
	{
		LogError() << "[Metal] CopyToSubTextureCube: size/format mismatch not supported yet" << std::endl;
		return;
	}

	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	metal::Device* device = re.Device();
	if (!device)
	{
		return;
	}

	metal::CommandBuffer* cmd = metal::AcquireCommandBuffer(device);
	if (!cmd)
	{
		return;
	}
	metal::CopyPass* copy = metal::BeginCopyPass(cmd);
	metal::TextureLocation src{};
	src.texture = texture_;
	src.mip_level = src_level;
	src.layer = src_array_index;
	src.x = src_x_offset;
	src.y = src_y_offset;
	src.z = 0;

	metal::TextureLocation dst{};
	dst.texture = dst_cube->GpuTexture();
	dst.mip_level = dst_level;
	dst.layer = dst_array_index * 6 + (dst_face - CF_Positive_X);
	dst.x = dst_x_offset;
	dst.y = dst_y_offset;
	dst.z = 0;

	metal::CopyTextureToTexture(copy, &src, &dst, dst_width, dst_height, 1, false);
	metal::EndCopyPass(copy);
	metal::SubmitCommandBuffer(cmd);
}

} // namespace RenderWorker
