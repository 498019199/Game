#include "D3D12Texture.h"
#include "D3D12RenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderFactory.h>
#include <algorithm>

namespace RenderWorker
{

D3D12TextureCube::D3D12TextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size, ElementFormat format,
	uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint)
	: D3D12Texture(TT_Cube, sample_count, sample_quality, access_hint), size_(size)
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

uint32_t D3D12TextureCube::Width(uint32_t level) const noexcept
{
	COMMON_ASSERT(level < mip_maps_num_);
	return std::max(1U, size_ >> level);
}

uint32_t D3D12TextureCube::Height(uint32_t level) const noexcept
{
	return Width(level);
}

void D3D12TextureCube::CreateHWResource(std::span<ElementInitData const> init_data, float4 const* /*clear_value_hint*/)
{
	DeleteHWResource();

	auto& re = checked_cast<D3D12RenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	d3d12::Device* device = re.Device();
	COMMON_ASSERT(device);

	d3d12::TextureCreateInfo info{};
	info.type = array_size_ > 1 ? d3d12::TEXTURETYPE_CUBE_ARRAY : d3d12::TEXTURETYPE_CUBE;
	info.format = D3D12Mapping::MappingFormat(format_);
	info.usage = d3d12::TEXTUREUSAGE_SAMPLER;
	info.width = size_;
	info.height = size_;
	info.layer_count_or_depth = 6 * array_size_;
	info.num_levels = mip_maps_num_;
	info.sample_count = d3d12::SAMPLECOUNT_1;

	texture_ = d3d12::CreateTexture(device, &info);
	D3D12Check(texture_ != nullptr, "d3d12::CreateTexture(cube)");
	device_ = texture_ ? device : nullptr;
	device_lifetime_ = texture_ ? re.DeviceLifetime() : nullptr;
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

void D3D12TextureCube::UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level, uint32_t x_offset,
	uint32_t y_offset, uint32_t width, uint32_t height, void const* data, uint32_t row_pitch)
{
	Upload2D(array_index * 6 + (face - CF_Positive_X), level, x_offset, y_offset, width, height, data, row_pitch);
}

} // namespace RenderWorker
