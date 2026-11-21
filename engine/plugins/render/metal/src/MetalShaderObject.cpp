#include "MetalShaderObject.h"
#include "MetalRenderEngine.h"
#include <base/ZEngine.h>
#include <render/RenderEffect.h>
#include <render/RenderFactory.h>
#include <common/Util.h>
#include <common/Hash.h>
#include <common/Log.h>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>
#include "MetalShaderCompiler.h"


namespace RenderWorker
{

namespace
{

char const* HlslStageProfile(ShaderStage stage)
{
	switch (stage)
	{
	case ShaderStage::Vertex:
		return "vs_6_0";
	case ShaderStage::Pixel:
		return "ps_6_0";
	default:
		return "";
	}
}


metal::ShaderStage ToGpuStage(ShaderStage stage)
{
	return (stage == ShaderStage::Pixel) ? metal::SHADERSTAGE_FRAGMENT : metal::SHADERSTAGE_VERTEX;
}


} // namespace

MetalShaderStageObject::MetalShaderStageObject(ShaderStage stage) : ShaderStageObject(stage)
{
}

MetalShaderStageObject::~MetalShaderStageObject()
{
	ReleaseGpuShader();
}

void MetalShaderStageObject::ReleaseGpuShader()
{
	if (gpu_shader_)
	{
		if (device_lifetime_ && (device_lifetime_->device == gpu_device_))
		{
			metal::ReleaseShader(gpu_device_, gpu_shader_);
		}
		gpu_shader_ = nullptr;
	}
	device_lifetime_.reset();
	gpu_device_ = nullptr;
}


void MetalShaderStageObject::FillCBufferIndices(RenderEffect const& effect)
{
	cbuff_indices_.clear();
	if (shader_desc_.cb_desc.empty())
	{
		return;
	}

	cbuff_indices_.resize(shader_desc_.cb_desc.size());
	for (size_t c = 0; c < shader_desc_.cb_desc.size(); ++c)
	{
		uint32_t i = 0;
		for (; i < effect.NumCBuffers(); ++i)
		{
			if (effect.CBufferByIndex(i)->NameHash() == shader_desc_.cb_desc[c].name_hash)
			{
				cbuff_indices_[c] = static_cast<uint8_t>(i);
				break;
			}
		}
		if (i >= effect.NumCBuffers())
		{
			LogError() << "[Metal] CBuffer not found in effect: " << shader_desc_.cb_desc[c].name << std::endl;
			cbuff_indices_[c] = 0xFF;
		}
	}
}

void MetalShaderStageObject::StreamIn([[maybe_unused]] const RenderEffect& effect,
	[[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids, [[maybe_unused]] ResIdentifier& res)
{
	is_validate_ = false;
	hw_res_ready_ = false;
}

void MetalShaderStageObject::StreamOut([[maybe_unused]] std::ostream& os)
{
}

void MetalShaderStageObject::CompileShader(const RenderEffect& effect, const RenderTechnique& tech,
	const RenderPass& pass, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
	shader_code_.clear();
	shader_desc_ = {};
	cbuff_indices_.clear();
	is_validate_ = false;

	if (stage_ != ShaderStage::Vertex && stage_ != ShaderStage::Pixel)
	{
		return;
	}

	uint32_t const shader_desc_id = shader_desc_ids[std::to_underlying(stage_)];
	auto const& sd = effect.GetShaderDesc(shader_desc_id);
	if (sd.func_name.empty())
	{
		return;
	}

	entry_point_ = sd.func_name;
	shader_profile_ = std::string(GetShaderProfile(effect, shader_desc_id));
	if (shader_profile_.empty())
	{
		return;
	}

    try
    {
        std::string const msl = CompileMetalShader(effect, tech, pass, stage_, entry_point_, shader_desc_);
        shader_code_.assign(msl.begin(), msl.end());
        shader_code_.push_back(0); // Metal consumes a null-terminated MSL string.
        shader_format_ = metal::SHADERFORMAT_MSL;
        FillCBufferIndices(effect);
    }
    catch (std::exception const& e)
    {
        shader_desc_ = {};
        shader_code_.clear();
        LogError() << "[Metal] HLSL/Metal compile failed for " << entry_point_ << ": " << e.what() << std::endl;
    }

	is_validate_ = !shader_code_.empty();
}

void MetalShaderStageObject::CreateHwShader(const RenderEffect& effect,
	[[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
	ReleaseGpuShader();
	hw_res_ready_ = false;
	if (!is_validate_ || shader_code_.empty())
	{
		return;
	}

	auto& re = checked_cast<MetalRenderEngine&>(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	metal::Device* device = re.Device();
	if (!device)
	{
		return;
	}

	if (cbuff_indices_.empty() && !shader_desc_.cb_desc.empty())
	{
		FillCBufferIndices(effect);
	}

	metal::ShaderCreateInfo info{};
	info.code_size = shader_code_.size();
	info.code = shader_code_.data();
	info.entrypoint = entry_point_.c_str();
	info.format = shader_format_;
	info.stage = ToGpuStage(stage_);
	info.num_samplers = shader_desc_.num_samplers;
	info.num_storage_textures = 0;
	info.num_storage_buffers = 0;
	info.num_uniform_buffers = shader_desc_.num_uniform_buffers;
	if (info.num_uniform_buffers == 0 && !shader_desc_.cb_desc.empty())
	{
		info.num_uniform_buffers = static_cast<uint32_t>(shader_desc_.cb_desc.size());
	}

	gpu_shader_ = metal::CreateShader(device, &info);
	if (!gpu_shader_)
	{
		LogError() << "[Metal] metal::CreateShader failed: " << metal::LastError() << std::endl;
		return;
	}
	gpu_device_ = device;
	device_lifetime_ = re.DeviceLifetime();
	hw_res_ready_ = true;
}

std::string_view MetalShaderStageObject::GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const
{
	std::string_view profile = effect.GetShaderDesc(shader_desc_id).profile;
	if (profile.empty() || profile == "auto")
	{
		return HlslStageProfile(stage_);
	}
	if (profile.find("vs_") == 0)
	{
		return HlslStageProfile(ShaderStage::Vertex);
	}
	if (profile.find("ps_") == 0)
	{
		return HlslStageProfile(ShaderStage::Pixel);
	}
	return profile;
}

MetalShaderObject::MetalShaderObject() = default;

MetalShaderObject::MetalShaderObject(std::shared_ptr<Immutable> immutable) : ShaderObject(std::move(immutable))
{
}

ShaderObjectPtr MetalShaderObject::Clone([[maybe_unused]] RenderEffect& dst_effect)
{
	auto ret = MakeSharedPtr<MetalShaderObject>(immutable_);
	ret->hw_res_ready_ = hw_res_ready_;
	ret->vs_ = vs_;
	ret->ps_ = ps_;
	return ret;
}

void MetalShaderObject::Bind([[maybe_unused]] const RenderEffect& effect)
{
	vs_ = nullptr;
	ps_ = nullptr;
	auto const& vs_stage = Stage(ShaderStage::Vertex);
	auto const& ps_stage = Stage(ShaderStage::Pixel);
	if (vs_stage)
	{
		vs_ = checked_cast<MetalShaderStageObject&>(*vs_stage).GpuShader();
	}
	if (ps_stage)
	{
		ps_ = checked_cast<MetalShaderStageObject&>(*ps_stage).GpuShader();
	}
}

void MetalShaderObject::Unbind()
{
	vs_ = nullptr;
	ps_ = nullptr;
}

void MetalShaderObject::DoLinkShaders(RenderEffect& effect)
{
	for (auto stage : {ShaderStage::Vertex, ShaderStage::Pixel})
	{
		auto const& stage_obj = Stage(stage);
		if (!stage_obj)
		{
			continue;
		}

		auto const& native_stage = checked_cast<MetalShaderStageObject const&>(*stage_obj);
		auto const& cbuff_indices = native_stage.CBufferIndices();
		if (cbuff_indices.empty())
		{
			continue;
		}

		auto const& shader_desc = native_stage.GetShaderDesc();
		for (size_t i = 0; i < cbuff_indices.size(); ++i)
		{
			if (cbuff_indices[i] == 0xFF)
			{
				continue;
			}

			auto* cbuff = effect.CBufferByIndex(cbuff_indices[i]);
				cbuff->Resize(shader_desc.cb_desc[i].size);
				auto const& var_descs = shader_desc.cb_desc[i].var_desc;
				if (cbuff->NumParameters() != var_descs.size())
				{
					LogWarn() << "[Metal] CBuffer '" << shader_desc.cb_desc[i].name
							  << "' param count mismatch: effect=" << cbuff->NumParameters()
							  << " reflection=" << var_descs.size()
							  << " (binding by name)" << std::endl;
				}
				for (uint32_t j = 0; j < cbuff->NumParameters(); ++j)
				{
					RenderEffectParameter* param = effect.ParameterByIndex(cbuff->ParameterIndex(j));
					// Prefer name match so effect/reflection order can diverge safely.
					uint32_t var_index = j;
					if (j < var_descs.size() && param->Name() != var_descs[j].name)
					{
						var_index = static_cast<uint32_t>(-1);
						for (uint32_t v = 0; v < var_descs.size(); ++v)
						{
							if (param->Name() == var_descs[v].name)
							{
								var_index = v;
								break;
							}
						}
						if (var_index == static_cast<uint32_t>(-1))
						{
							LogError() << "[Metal] CBuffer var not in reflection: " << param->Name() << std::endl;
							continue;
						}
					}
					else if (j >= var_descs.size())
					{
						continue;
					}

					auto const& vd = var_descs[var_index];
					uint32_t stride;
					if (param->Type() == REDT_struct)
					{
						stride = 1;
					}
					else if (vd.elements > 0)
					{
						stride = (param->Type() != REDT_float4x4) ? 16u : 64u;
					}
					else
					{
						stride = (param->Type() != REDT_float4x4) ? 4u : 16u;
					}
					param->BindToCBuffer(effect, cbuff_indices[i], vd.start_offset, stride);
				}
		}
	}

	hw_res_ready_ = true;
	for (auto stage : {ShaderStage::Vertex, ShaderStage::Pixel})
	{
		auto const& s = immutable_->shader_stages_[std::to_underlying(stage)];
		if (s && !s->HWResourceReady())
		{
			hw_res_ready_ = false;
			break;
		}
	}
}

} // namespace RenderWorker
