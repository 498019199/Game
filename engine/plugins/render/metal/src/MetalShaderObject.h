#pragma once

#include <render/ShaderObject.h>
#include "MetalUtil.h"
#include <vector>
#include <string>

namespace RenderWorker
{

struct MetalDeviceLifetime;

struct MetalShaderDesc
{
	struct ConstantBufferDesc
	{
		struct VariableDesc
		{
			std::string name;
			uint32_t start_offset = 0;
			uint8_t type = 0;
			uint8_t rows = 0;
			uint8_t columns = 0;
			uint16_t elements = 0;
		};
		std::vector<VariableDesc> var_desc;

		std::string name;
		size_t name_hash = 0;
		uint32_t size = 0;
		uint16_t bind_point = 0;
	};
	std::vector<ConstantBufferDesc> cb_desc;

	enum class ResourceKind { Texture, Sampler };

	struct BoundResourceDesc
	{
		std::string name;
		ResourceKind type{};
		uint16_t bind_point = 0;
	};
	std::vector<BoundResourceDesc> res_desc;

	uint16_t num_samplers = 0;
	uint16_t num_srvs = 0;
	uint16_t num_uavs = 0;
	uint16_t num_uniform_buffers = 0;
};

class MetalShaderStageObject final : public ShaderStageObject
{
public:
	explicit MetalShaderStageObject(ShaderStage stage);
	~MetalShaderStageObject() override;

	void StreamIn(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids,
		ResIdentifier& res) override;
	void StreamOut(std::ostream& os) override;
	void CompileShader(const RenderEffect& effect, const RenderTechnique& tech, const RenderPass& pass,
		const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) override;
	void CreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) override;

	metal::Shader* GpuShader() const noexcept
	{
		return gpu_shader_;
	}

	metal::ShaderFormat ShaderFormat() const noexcept
	{
		return shader_format_;
	}

	MetalShaderDesc const& GetShaderDesc() const noexcept
	{
		return shader_desc_;
	}

	std::vector<uint8_t> const& CBufferIndices() const noexcept
	{
		return cbuff_indices_;
	}

	std::string const& EntryPoint() const noexcept
	{
		return entry_point_;
	}

protected:
	std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;

private:
	void ReleaseGpuShader();
	void FillCBufferIndices(RenderEffect const& effect);

private:
	std::vector<uint8_t> shader_code_;
	metal::ShaderFormat shader_format_{metal::SHADERFORMAT_MSL};
	std::shared_ptr<MetalDeviceLifetime> device_lifetime_;
	metal::Device* gpu_device_{nullptr};
	metal::Shader* gpu_shader_{nullptr};
	std::string entry_point_{"main"};
	std::string shader_profile_;
	MetalShaderDesc shader_desc_;
	std::vector<uint8_t> cbuff_indices_;
};

class MetalShaderObject final : public ShaderObject
{
public:
	MetalShaderObject();
	explicit MetalShaderObject(std::shared_ptr<Immutable> immutable);

	ShaderObjectPtr Clone(RenderEffect& dst_effect) override;
	void Bind(const RenderEffect& effect) override;
	void Unbind() override;

	metal::Shader* VertexShader() const noexcept
	{
		return vs_;
	}
	metal::Shader* PixelShader() const noexcept
	{
		return ps_;
	}

private:
	void DoLinkShaders(RenderEffect& effect) override;

private:
	metal::Shader* vs_{nullptr};
	metal::Shader* ps_{nullptr};
};

} // namespace RenderWorker
