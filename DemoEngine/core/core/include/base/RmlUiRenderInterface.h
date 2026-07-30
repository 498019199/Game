#pragma once

#include <RmlUi/Core/RenderInterface.h>

#include <math/math.h>
#include <render/RenderEffect.h>
#include <render/RenderLayout.h>
#include <render/Texture.h>

#include <cstdint>
#include <unordered_map>

namespace RenderWorker
{

/// RmlUi 6.x renderer backed by RenderEffect + RenderEngine (no raw D3D PSO).
class RmlUiRenderInterfaceD3D11 final : public Rml::RenderInterface
{
public:
	RmlUiRenderInterfaceD3D11();
	~RmlUiRenderInterfaceD3D11() override;

	void SetViewportSize(int width, int height);

	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
		Rml::Span<const int> indices) override;
	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
		Rml::TextureHandle texture) override;
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, Rml::String const& source) override;
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
	void ReleaseTexture(Rml::TextureHandle texture_handle) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;
	void SetTransform(Rml::Matrix4f const* transform) override;

	bool Ready() const noexcept { return pipeline_ok_; }

private:
	void SetTransformsForDraw(Rml::Vector2f translation);
	bool BuildPipeline();
	void UpdateProjection();

	RenderEffectPtr effect_;
	RenderTechnique* tech_ {nullptr};
	RenderTechnique* tech_scissor_ {nullptr};
	RenderEffectParameter* transform_ep_ {nullptr};
	RenderEffectParameter* rml_tex_ep_ {nullptr};
	TexturePtr white_tex_;

	int viewport_width_ {1};
	int viewport_height_ {1};
	float4x4 projection_ {float4x4::Identity()};
	float4x4 element_transform_ {float4x4::Identity()};
	bool has_element_transform_ {false};
	bool scissor_enabled_ {false};
	Rml::Rectanglei scissor_region_ {};
	bool pipeline_ok_ {false};

	uint64_t next_texture_id_ {1};
	std::unordered_map<uint64_t, TexturePtr> textures_;
};

} // namespace RenderWorker
