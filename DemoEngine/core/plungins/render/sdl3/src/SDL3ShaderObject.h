#pragma once

#include <render/ShaderObject.h>

namespace RenderWorker
{

class SDL3ShaderStageObject final : public ShaderStageObject
{
public:
	explicit SDL3ShaderStageObject(ShaderStage stage);

	void StreamIn(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids,
		ResIdentifier& res) override;
	void StreamOut(std::ostream& os) override;
	void CompileShader(const RenderEffect& effect, const RenderTechnique& tech, const RenderPass& pass,
		const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) override;
	void CreateHwShader(const RenderEffect& effect, const std::array<uint32_t, ShaderStageNum>& shader_desc_ids) override;

protected:
	std::string_view GetShaderProfile(RenderEffect const& effect, uint32_t shader_desc_id) const override;
};

class SDL3ShaderObject final : public ShaderObject
{
public:
	SDL3ShaderObject();
	explicit SDL3ShaderObject(std::shared_ptr<Immutable> immutable);

	ShaderObjectPtr Clone(RenderEffect& dst_effect) override;
	void Bind(const RenderEffect& effect) override;
	void Unbind() override;

private:
	void DoLinkShaders(RenderEffect& effect) override;
};

} // namespace RenderWorker
