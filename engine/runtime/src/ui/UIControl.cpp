#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Vertex.h>

#include <d3dcompiler.h>

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace EditorWorker {

UIRectRenderable::UIRectRenderable( const TexturePtr& texture, const RenderEffectPtr& effect )
	:Renderable(L"UIRect"), texture_(texture)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	rls_[0] = rf.MakeRenderLayout();
	restart_ = rf.RenderEngineInstance().DeviceCaps().primitive_restart_support;
	if (restart_)
	{
		rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);
	}
	else
	{
		rls_[0]->TopologyType(RenderLayout::TT_TriangleList);
	}

	effect_ = effect;
	if (texture)
	{
		technique_ = effect->TechniqueByName("UITec");
	}
	else
	{
		technique_ = effect->TechniqueByName("UITecNoTex");
	}

	uint32_t const INDEX_PER_QUAD = restart_ ? 5 : 6;
	uint32_t const INIT_NUM_QUAD = 1024;
	tb_vb_ = MakeUniquePtr<TransientBuffer>(static_cast<uint32_t>(INIT_NUM_QUAD * 4 * sizeof(UIManager::VertexFormat)), TransientBuffer::BF_Vertex);
	tb_ib_ = MakeUniquePtr<TransientBuffer>(static_cast<uint32_t>(INIT_NUM_QUAD * INDEX_PER_QUAD * sizeof(uint16_t)), TransientBuffer::BF_Index);
	// MSVC 下 Rml::Vertex 常在 colour 后插入对齐填充；APPEND_ALIGNED_ELEMENT 会把 TEXCOORD 对齐错，导致三角形无效。
	rls_[0]->BindVertexStream(tb_vb_->GetBuffer(), MakeSpan({VertexElement(VEU_Position, 0, EF_BGR32F),
		VertexElement(VEU_Diffuse, 0, EF_ABGR32F), VertexElement(VEU_TextureCoord, 0, EF_GR32F)}));
	rls_[0]->BindIndexStream(tb_ib_->GetBuffer(), EF_R16UI);
}

UIRectRenderable::~UIRectRenderable()
{
}

void RmlUiRenderInterfaceD3D11::SetViewportSize(int width, int height)
{
	viewport_width_ = (std::max)(1, width);
	viewport_height_ = (std::max)(1, height);
	projection_ = Rml::Matrix4f::ProjectOrtho(0.f, float(viewport_width_), float(viewport_height_), 0.f, -1.f, 1.f);
}

void RmlUiRenderInterfaceD3D11::SetTransformsForDraw(Rml::Vector2f translation)
{
	Rml::Matrix4f world = Rml::Matrix4f::Translate(translation.x, translation.y, 0.f);
	if (element_transform_)
	{
		world = (*element_transform_) * world;
	}
	Rml::Matrix4f mvp = projection_ * world;

	*(effect_->ParameterByName("Transform")) = reinterpret_cast<floatx4x4>(mvp);
}

Rml::CompiledGeometryHandle RmlUiRenderInterfaceD3D11::CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
	Rml::Span<const int> indices)
{
	if (!pipeline_ok_ || !device_ || vertices.empty() || indices.empty())
	{
		return {};
	}

	auto* geo = new CompiledGeometryD3D11{};
	geo->index_count = UINT(indices.size());

	D3D11_BUFFER_DESC vbd{};
	vbd.ByteWidth = UINT(sizeof(Rml::Vertex) * vertices.size());
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vsd{};
	vsd.pSysMem = vertices.data();
	if (FAILED(device_->CreateBuffer(&vbd, &vsd, &geo->vb)))
	{
		delete geo;
		return {};
	}

	D3D11_BUFFER_DESC ibd{};
	ibd.ByteWidth = UINT(sizeof(int) * indices.size());
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA isd{};
	isd.pSysMem = indices.data();
	if (FAILED(device_->CreateBuffer(&ibd, &isd, &geo->ib)))
	{
		geo->vb->Release();
		delete geo;
		return {};
	}

	return reinterpret_cast<Rml::CompiledGeometryHandle>(geo);
}

void RmlUiRenderInterfaceD3D11::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
	Rml::TextureHandle texture)
{
	if (!pipeline_ok_ || !ctx_ || !vs_ || !ps_ || !layout_ || !cb_ || geometry == 0)
	{
		return;
	}
	auto* geo = reinterpret_cast<CompiledGeometryD3D11*>(geometry);

	float blend_factor[4] = {0, 0, 0, 0};
	UINT sample_mask = 0xffffffff;
	ctx_->OMSetBlendState(blend_, blend_factor, sample_mask);
	ctx_->OMSetDepthStencilState(dss_no_depth_, 0);
	ctx_->RSSetState(scissor_enabled_ ? rs_scissor_ : rs_);

	D3D11_VIEWPORT vp{};
	vp.Width = float(viewport_width_);
	vp.Height = float(viewport_height_);
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	ctx_->RSSetViewports(1, &vp);

	if (scissor_enabled_)
	{
		D3D11_RECT r{};
		r.left = scissor_region_.Left();
		r.top = scissor_region_.Top();
		// RmlUi rectangles use inclusive max corner; D3D scissor right/bottom are exclusive.
		r.right = scissor_region_.Right() + 1;
		r.bottom = scissor_region_.Bottom() + 1;
		ctx_->RSSetScissorRects(1, &r);
	}

	SetTransformsForDraw(translation);

	ID3D11ShaderResourceView* srv = white_srv_;
	if (texture)
	{
		auto it = textures_.find(static_cast<uint64_t>(texture));
		if (it != textures_.end() && it->second)
		{
			srv = it->second;
		}
	}
	ctx_->PSSetShaderResources(0, 1, &srv);
	ctx_->PSSetSamplers(0, 1, &sampler_);

	UINT stride = UINT(sizeof(Rml::Vertex));
	UINT offset = 0;
	ctx_->IASetInputLayout(layout_);
	ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx_->IASetVertexBuffers(0, 1, &geo->vb, &stride, &offset);
	ctx_->IASetIndexBuffer(geo->ib, DXGI_FORMAT_R32_UINT, 0);
	ctx_->VSSetShader(vs_, nullptr, 0);
	ctx_->PSSetShader(ps_, nullptr, 0);
	ctx_->DrawIndexed(geo->index_count, 0, 0);

	ID3D11ShaderResourceView* null_srv = nullptr;
	ctx_->PSSetShaderResources(0, 1, &null_srv);
}

void RmlUiRenderInterfaceD3D11::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	if (geometry == 0)
	{
		return;
	}
	auto* geo = reinterpret_cast<CompiledGeometryD3D11*>(geometry);
	if (geo->vb)
	{
		geo->vb->Release();
	}
	if (geo->ib)
	{
		geo->ib->Release();
	}
	delete geo;
}

Rml::TextureHandle RmlUiRenderInterfaceD3D11::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& /*source*/)
{
	texture_dimensions = {0, 0};
	return {};
}

Rml::TextureHandle RmlUiRenderInterfaceD3D11::GenerateTexture(Rml::Span<const Rml::byte> source,
	Rml::Vector2i source_dimensions)
{
	if (!pipeline_ok_ || !device_ || source.empty() || source_dimensions.x <= 0 || source_dimensions.y <= 0)
	{
		return {};
	}

	const UINT w = UINT(source_dimensions.x);
	const UINT h = UINT(source_dimensions.y);
	const UINT pitch = w * 4u;

	D3D11_TEXTURE2D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_IMMUTABLE;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = source.data();
	srd.SysMemPitch = pitch;

	ID3D11Texture2D* tex = nullptr;
	if (FAILED(device_->CreateTexture2D(&td, &srd, &tex)))
	{
		return {};
	}

	ID3D11ShaderResourceView* srv = nullptr;
	if (FAILED(device_->CreateShaderResourceView(tex, nullptr, &srv)))
	{
		tex->Release();
		return {};
	}
	tex->Release();

	const uint64_t id = next_texture_id_++;
	textures_[id] = srv;
	return static_cast<Rml::TextureHandle>(id);
}

void RmlUiRenderInterfaceD3D11::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	if (!texture_handle)
	{
		return;
	}
	auto it = textures_.find(static_cast<uint64_t>(texture_handle));
	if (it != textures_.end())
	{
		if (it->second)
		{
			it->second->Release();
		}
		textures_.erase(it);
	}
}

void RmlUiRenderInterfaceD3D11::EnableScissorRegion(bool enable)
{
	scissor_enabled_ = enable;
}

void RmlUiRenderInterfaceD3D11::SetScissorRegion(Rml::Rectanglei region)
{
	scissor_region_ = region;
}

void RmlUiRenderInterfaceD3D11::SetTransform(const Rml::Matrix4f* transform)
{
	element_transform_ = transform;
}

} // namespace EditorWorker
