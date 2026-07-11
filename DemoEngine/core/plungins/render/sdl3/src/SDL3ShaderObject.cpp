#include "SDL3ShaderObject.h"

namespace RenderWorker
{

SDL3ShaderStageObject::SDL3ShaderStageObject(ShaderStage stage)
	: ShaderStageObject(stage)
{
}

void SDL3ShaderStageObject::StreamIn([[maybe_unused]] const RenderEffect& effect,
	[[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids, [[maybe_unused]] ResIdentifier& res)
{
	is_validate_ = true;
	hw_res_ready_ = true;
}

void SDL3ShaderStageObject::StreamOut([[maybe_unused]] std::ostream& os)
{
}

void SDL3ShaderStageObject::CompileShader([[maybe_unused]] const RenderEffect& effect,
	[[maybe_unused]] const RenderTechnique& tech, [[maybe_unused]] const RenderPass& pass,
	[[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
	is_validate_ = true;
}

void SDL3ShaderStageObject::CreateHwShader([[maybe_unused]] const RenderEffect& effect,
	[[maybe_unused]] const std::array<uint32_t, ShaderStageNum>& shader_desc_ids)
{
	hw_res_ready_ = true;
}

std::string_view SDL3ShaderStageObject::GetShaderProfile([[maybe_unused]] RenderEffect const& effect,
	[[maybe_unused]] uint32_t shader_desc_id) const
{
	return "main";
}

SDL3ShaderObject::SDL3ShaderObject() = default;

SDL3ShaderObject::SDL3ShaderObject(std::shared_ptr<Immutable> immutable)
	: ShaderObject(std::move(immutable))
{
}

ShaderObjectPtr SDL3ShaderObject::Clone([[maybe_unused]] RenderEffect& dst_effect)
{
	auto ret = MakeSharedPtr<SDL3ShaderObject>(immutable_);
	ret->hw_res_ready_ = hw_res_ready_;
	return ret;
}

void SDL3ShaderObject::Bind([[maybe_unused]] const RenderEffect& effect)
{
}

void SDL3ShaderObject::Unbind()
{
}

void SDL3ShaderObject::DoLinkShaders([[maybe_unused]] RenderEffect& effect)
{
}

} // namespace RenderWorker
