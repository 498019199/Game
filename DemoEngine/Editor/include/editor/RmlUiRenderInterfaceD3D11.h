#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Matrix4.h>

#include <d3d11.h>

#include <cstdint>
#include <unordered_map>

namespace EditorWorker {

/// Minimal RmlUi 6.x renderer for D3D11: geometry compile, textures (incl. font atlas), scissor, optional element transform.
class RmlUiRenderInterfaceD3D11 final : public Rml::RenderInterface {
public:
	RmlUiRenderInterfaceD3D11(ID3D11Device* device, ID3D11DeviceContext* imm_ctx);
	~RmlUiRenderInterfaceD3D11() override;

	void SetViewportSize(int width, int height);

	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
		Rml::Span<const int> indices) override;
	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
		Rml::TextureHandle texture) override;
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
	void ReleaseTexture(Rml::TextureHandle texture_handle) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;

	void SetTransform(const Rml::Matrix4f* transform) override;

private:
	void EnsurePipeline();
	void SetTransformsForDraw(Rml::Vector2f translation);
	void UploadConstantBuffer(const float* row_major_m16);

	ID3D11Device* device_{};
	ID3D11DeviceContext* ctx_{};

	int viewport_width_{1};
	int viewport_height_{1};

	Rml::Matrix4f projection_{};
	const Rml::Matrix4f* element_transform_{nullptr};

	ID3D11VertexShader* vs_{};
	ID3D11PixelShader* ps_{};
	ID3D11InputLayout* layout_{};
	ID3D11Buffer* cb_{};
	ID3D11SamplerState* sampler_{};
	ID3D11BlendState* blend_{};
	ID3D11RasterizerState* rs_{};
	ID3D11RasterizerState* rs_scissor_{};
	ID3D11DepthStencilState* dss_no_depth_{};

	ID3D11ShaderResourceView* white_srv_{};
	ID3D11Texture2D* white_tex_{};

	bool scissor_enabled_{false};
	Rml::Rectanglei scissor_region_{};

	uint64_t next_texture_id_{1};
	std::unordered_map<uint64_t, ID3D11ShaderResourceView*> textures_;

	bool pipeline_ok_{false};
};

} // namespace EditorWorker
