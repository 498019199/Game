#pragma once

#include <render/Texture.h>
#include "D3D12Util.h"

namespace RenderWorker
{

struct D3D12DeviceLifetime;

class ZENGINE_D3D12_API D3D12Texture : public Texture
{
public:
	D3D12Texture(TextureType type, uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);
	~D3D12Texture() override;

	d3d12::Texture* GpuTexture() const noexcept
	{
		return texture_;
	}

	d3d12::TextureFormat GpuFormat() const noexcept
	{
		return gpu_format_;
	}

	void DeleteHWResource() override;
	bool HWResourceReady() const override;

	uint32_t Width(uint32_t level) const noexcept override;
	uint32_t Height(uint32_t level) const noexcept override;
	uint32_t Depth(uint32_t level) const noexcept override;

	void CopyToTexture(Texture& target, TextureFilter filter) override;
	void CopyToSubTexture1D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_width, uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_width,
		TextureFilter filter) override;
	void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level,
		uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
		TextureFilter filter) override;
	void CopyToSubTexture3D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_y_offset, uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth,
		uint32_t src_array_index, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_z_offset, uint32_t src_width, uint32_t src_height, uint32_t src_depth,
		TextureFilter filter) override;
	void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height, TextureFilter filter) override;

	void UpdateSubresource1D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t width,
		void const* data) override;
	void UpdateSubresource2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width,
		uint32_t height, void const* data, uint32_t row_pitch) override;
	void UpdateSubresource3D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset,
		uint32_t z_offset, uint32_t width, uint32_t height, uint32_t depth, void const* data, uint32_t row_pitch,
		uint32_t slice_pitch) override;
	void UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level, uint32_t x_offset,
		uint32_t y_offset, uint32_t width, uint32_t height, void const* data, uint32_t row_pitch) override;

protected:
	void Map1D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t width,
		void*& data) override;
	void Map2D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset,
		uint32_t width, uint32_t height, void*& data, uint32_t& row_pitch) override;
	void Map3D(uint32_t array_index, uint32_t level, TextureMapAccess tma, uint32_t x_offset, uint32_t y_offset,
		uint32_t z_offset, uint32_t width, uint32_t height, uint32_t depth, void*& data, uint32_t& row_pitch,
		uint32_t& slice_pitch) override;
	void MapCube(uint32_t array_index, CubeFaces face, uint32_t level, TextureMapAccess tma, uint32_t x_offset,
		uint32_t y_offset, uint32_t width, uint32_t height, void*& data, uint32_t& row_pitch) override;

	void Unmap1D(uint32_t array_index, uint32_t level) override;
	void Unmap2D(uint32_t array_index, uint32_t level) override;
	void Unmap3D(uint32_t array_index, uint32_t level) override;
	void UnmapCube(uint32_t array_index, CubeFaces face, uint32_t level) override;

	void Upload2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width,
		uint32_t height, void const* data, uint32_t row_pitch);

protected:
	std::shared_ptr<D3D12DeviceLifetime> device_lifetime_;
	d3d12::Device* device_{nullptr};
	d3d12::Texture* texture_{nullptr};
	d3d12::TextureFormat gpu_format_{d3d12::TEXTUREFORMAT_INVALID};
};

class D3D12Texture2D final : public D3D12Texture
{
public:
	D3D12Texture2D(uint32_t width, uint32_t height, uint32_t num_mip_maps, uint32_t array_size, ElementFormat format,
		uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

	uint32_t Width(uint32_t level) const noexcept override;
	uint32_t Height(uint32_t level) const noexcept override;

	void CopyToTexture(Texture& target, TextureFilter filter) override;
	void CopyToSubTexture2D(Texture& target, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset,
		uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, uint32_t src_array_index, uint32_t src_level,
		uint32_t src_x_offset, uint32_t src_y_offset, uint32_t src_width, uint32_t src_height,
		TextureFilter filter) override;

	void CreateHWResource(std::span<ElementInitData const> init_data, float4 const* clear_value_hint) override;

	void UpdateSubresource2D(uint32_t array_index, uint32_t level, uint32_t x_offset, uint32_t y_offset, uint32_t width,
		uint32_t height, void const* data, uint32_t row_pitch) override;

	void CopyToSubTextureCube(Texture& target, uint32_t dst_array_index, CubeFaces dst_face, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
		uint32_t src_array_index, CubeFaces src_face, uint32_t src_level, uint32_t src_x_offset, uint32_t src_y_offset,
		uint32_t src_width, uint32_t src_height, TextureFilter filter) override;

private:
	uint32_t width_{0};
	uint32_t height_{0};
};

class D3D12TextureCube final : public D3D12Texture
{
public:
	D3D12TextureCube(uint32_t size, uint32_t num_mip_maps, uint32_t array_size, ElementFormat format,
		uint32_t sample_count, uint32_t sample_quality, uint32_t access_hint);

	uint32_t Width(uint32_t level) const noexcept override;
	uint32_t Height(uint32_t level) const noexcept override;

	void CreateHWResource(std::span<ElementInitData const> init_data, float4 const* clear_value_hint) override;
	void UpdateSubresourceCube(uint32_t array_index, CubeFaces face, uint32_t level, uint32_t x_offset,
		uint32_t y_offset, uint32_t width, uint32_t height, void const* data, uint32_t row_pitch) override;

private:
	uint32_t size_{0};
};

} // namespace RenderWorker
