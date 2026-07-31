#pragma once

#include <render/ShaderObject.h>
#include "SDL3Util.h"
#include <vector>
#include <string>

namespace RenderWorker
{

struct SDL3ShaderDesc
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

	struct BoundResourceDesc
	{
		std::string name;
		uint8_t type = 0; // D3D_SHADER_INPUT_TYPE
		uint16_t bind_point = 0;
	};
	std::vector<BoundResourceDesc> res_desc;

	uint16_t num_samplers = 0;
	uint16_t num_srvs = 0;
	uint16_t num_uavs = 0;
	uint16_t num_uniform_buffers = 0;
};

class SDL3ShaderStageObject final : public ShaderStageObject
{
public:
	explicit SDL3ShaderStageObject(ShaderStage stage);
	~SDL3ShaderStageObject() override;

	void StreamIn(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids,
		ResIdentifier& res) override;
	void StreamOut(std::ostream& os) override;
	void CompileShader(const RenderEffect& effect, const RenderTechnique& tech, const RenderPass& pass,
		const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) override;
	void CreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) override;

	SDL_GPUShader* GpuShader() const noexcept
	{
		return gpu_shader_;
	}

	SDL_GPUShaderFormat ShaderFormat() const noexcept
	{
		return shader_format_;
	}

	SDL3ShaderDesc const& GetShaderDesc() const noexcept
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
	void FillShaderDescFromReflection(void* d3d11_reflection);

private:
	std::vector<uint8_t> shader_code_;
	SDL_GPUShaderFormat shader_format_{SDL_GPU_SHADERFORMAT_DXIL};
	SDL_GPUShader* gpu_shader_{nullptr};
	std::string entry_point_{"main"};
	std::string shader_profile_;
	SDL3ShaderDesc shader_desc_;
	std::vector<uint8_t> cbuff_indices_;
};

class SDL3ShaderObject final : public ShaderObject
{
public:
	SDL3ShaderObject();
	explicit SDL3ShaderObject(std::shared_ptr<Immutable> immutable);

	ShaderObjectPtr Clone(RenderEffect& dst_effect) override;
	void Bind(const RenderEffect& effect) override;
	void Unbind() override;

	SDL_GPUShader* VertexShader() const noexcept
	{
		return vs_;
	}
	SDL_GPUShader* PixelShader() const noexcept
	{
		return ps_;
	}

private:
	void DoLinkShaders(RenderEffect& effect) override;

private:
	SDL_GPUShader* vs_{nullptr};
	SDL_GPUShader* ps_{nullptr};
};

} // namespace RenderWorker
