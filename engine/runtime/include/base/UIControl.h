#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Matrix4.h>

#include <d3d11.h>

#include <cstdint>
#include <unordered_map>

namespace EditorWorker 
{

/// Minimal RmlUi 6.x renderer for D3D11: geometry compile, textures (incl. font atlas), scissor, optional element transform.
class UIRectRenderable : public Rml::RenderInterface , public RenderWorker::Renderable
{
public:
	UIRectRenderable( const TexturePtr& texture, const RenderEffectPtr& effect );
	~UIRectRenderable() override;

	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
		Rml::TextureHandle texture) override;
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, 
		Rml::Span<const int> indices) override;
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	void ReleaseTexture(Rml::TextureHandle texture_handle) override;
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;

	bool Empty() const
	void OnRenderBegin() override;
	void OnRenderEnd() override;
	void Render() override;

	void SetViewportSize(int width, int height);
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;
	void SetTransform(const Rml::Matrix4f* transform) override;

private:

	void SetTransformsForDraw(Rml::Vector2f translation);


	int viewport_width_{1};
	int viewport_height_{1};

	Rml::Matrix4f projection_{};
	const Rml::Matrix4f* element_transform_{nullptr};

	bool restart_;

	TexturePtr texture_;

	std::unique_ptr<TransientBuffer> tb_vb_;
	std::unique_ptr<TransientBuffer> tb_ib_;
	std::vector<SubAlloc> tb_vb_sub_allocs_;
	std::vector<SubAlloc> tb_ib_sub_allocs_;
};

} // namespace EditorWorker
