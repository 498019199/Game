#include <editor/RmlUiRenderInterfaceD3D11.h>

#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Vertex.h>

#include <d3dcompiler.h>

#include <algorithm>
#include <cstddef>
#include <cstring>

#ifndef NDEBUG
#define RML_D3D_COMPILE_FLAGS (D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_DEBUG)
#else
#define RML_D3D_COMPILE_FLAGS (D3DCOMPILE_OPTIMIZATION_LEVEL3)
#endif

namespace EditorWorker {
namespace {

static constexpr char kHlsl[] = R"(
cbuffer RmlCB : register(b0) {
	row_major float4x4 Transform;
};
Texture2D RmlTex : register(t0);
SamplerState RmlSamp : register(s0);

struct VS_IN {
	float2 pos : POSITION;
	float4 col : COLOR;
	float2 uv : TEXCOORD0;
};
struct VS_OUT {
	float4 pos : SV_Position;
	float4 col : COLOR;
	float2 uv : TEXCOORD0;
};
VS_OUT RmlVS(VS_IN vin) {
	VS_OUT vout;
	vout.pos = mul(float4(vin.pos, 0.f, 1.f), Transform);
	vout.col = vin.col;
	vout.uv = vin.uv;
	return vout;
}
float4 RmlPS(VS_OUT pin) : SV_Target {
	float4 s = RmlTex.Sample(RmlSamp, pin.uv);
	return pin.col * s;
}
)";

struct CompiledGeometryD3D11 {
	ID3D11Buffer* vb{};
	ID3D11Buffer* ib{};
	UINT index_count{};
};

bool RmlUICompileShader(char const* src, char const* entry, char const* target, ID3DBlob** out_blob, ID3DBlob** err_blob)
{
	UINT flags = RML_D3D_COMPILE_FLAGS;
	HRESULT hr = D3DCompile(src, std::strlen(src), nullptr, nullptr, nullptr, entry, target, flags, 0, out_blob, err_blob);
	if (FAILED(hr) && err_blob && *err_blob)
	{
		char const* msg = static_cast<char const*>(static_cast<void*>((*err_blob)->GetBufferPointer()));
		size_t len = (*err_blob)->GetBufferSize();
		int n = len > static_cast<size_t>(2047) ? 2047 : static_cast<int>(len);
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: HLSL compile failed (%s): %.*s", entry, n, msg);
	}
	return SUCCEEDED(hr);
}

} // namespace

RmlUiRenderInterfaceD3D11::RmlUiRenderInterfaceD3D11(ID3D11Device* device, ID3D11DeviceContext* imm_ctx)
	: device_(device)
	, ctx_(imm_ctx)
{
	EnsurePipeline();

	D3D11_TEXTURE2D_DESC td{};
	td.Width = 1;
	td.Height = 1;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_IMMUTABLE;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	Rml::byte px[4] = {255, 255, 255, 255};
	D3D11_SUBRESOURCE_DATA srd{};
	srd.pSysMem = px;
	srd.SysMemPitch = 4;
	if (FAILED(device_->CreateTexture2D(&td, &srd, &white_tex_)) || FAILED(device_->CreateShaderResourceView(white_tex_, nullptr, &white_srv_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: failed to create fallback white texture.");
		pipeline_ok_ = false;
	}
}

RmlUiRenderInterfaceD3D11::~RmlUiRenderInterfaceD3D11()
{
	for (auto& kv : textures_)
	{
		if (kv.second)
		{
			kv.second->Release();
		}
	}
	textures_.clear();

	if (white_srv_)
	{
		white_srv_->Release();
	}
	if (white_tex_)
	{
		white_tex_->Release();
	}
	if (dss_no_depth_)
	{
		dss_no_depth_->Release();
	}
	if (rs_scissor_)
	{
		rs_scissor_->Release();
	}
	if (rs_)
	{
		rs_->Release();
	}
	if (blend_)
	{
		blend_->Release();
	}
	if (sampler_)
	{
		sampler_->Release();
	}
	if (cb_)
	{
		cb_->Release();
	}
	if (layout_)
	{
		layout_->Release();
	}
	if (ps_)
	{
		ps_->Release();
	}
	if (vs_)
	{
		vs_->Release();
	}
}

void RmlUiRenderInterfaceD3D11::SetViewportSize(int width, int height)
{
	viewport_width_ = (std::max)(1, width);
	viewport_height_ = (std::max)(1, height);
	projection_ = Rml::Matrix4f::ProjectOrtho(0.f, float(viewport_width_), float(viewport_height_), 0.f, -1.f, 1.f);
}

void RmlUiRenderInterfaceD3D11::EnsurePipeline()
{
	if (vs_)
	{
		return;
	}

	pipeline_ok_ = false;

	if (!device_ || !ctx_)
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: device or context is null.");
		return;
	}

	ID3DBlob* vs_blob = nullptr;
	ID3DBlob* ps_blob = nullptr;
	ID3DBlob* err_blob = nullptr;
	if (!RmlUICompileShader(kHlsl, "RmlVS", "vs_5_0", &vs_blob, &err_blob))
	{
		if (err_blob)
		{
			err_blob->Release();
		}
		return;
	}
	if (!RmlUICompileShader(kHlsl, "RmlPS", "ps_5_0", &ps_blob, &err_blob))
	{
		if (err_blob)
		{
			err_blob->Release();
		}
		vs_blob->Release();
		return;
	}

	if (FAILED(device_->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vs_)) ||
		FAILED(device_->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &ps_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: CreateVertexShader / CreatePixelShader failed.");
		vs_blob->Release();
		ps_blob->Release();
		return;
	}

	// MSVC 下 Rml::Vertex 常在 colour 后插入对齐填充；APPEND_ALIGNED_ELEMENT 会把 TEXCOORD 对齐错，导致三角形无效。
	static_assert(sizeof(Rml::Vertex) <= 64, "unexpected vertex size");
	D3D11_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, UINT(offsetof(Rml::Vertex, position)), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, UINT(offsetof(Rml::Vertex, colour)), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, UINT(offsetof(Rml::Vertex, tex_coord)), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	if (FAILED(device_->CreateInputLayout(elements, 3, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &layout_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: CreateInputLayout failed.");
		vs_blob->Release();
		ps_blob->Release();
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	vs_blob->Release();
	ps_blob->Release();

	D3D11_BUFFER_DESC cbd{};
	cbd.ByteWidth = sizeof(float) * 16;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(device_->CreateBuffer(&cbd, nullptr, &cb_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: constant buffer create failed.");
		layout_->Release();
		layout_ = nullptr;
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	D3D11_SAMPLER_DESC sd{};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(device_->CreateSamplerState(&sd, &sampler_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: sampler create failed.");
		cb_->Release();
		cb_ = nullptr;
		layout_->Release();
		layout_ = nullptr;
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	D3D11_BLEND_DESC bd{};
	bd.RenderTarget[0].BlendEnable = TRUE;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(device_->CreateBlendState(&bd, &blend_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: blend state create failed.");
		sampler_->Release();
		sampler_ = nullptr;
		cb_->Release();
		cb_ = nullptr;
		layout_->Release();
		layout_ = nullptr;
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	D3D11_RASTERIZER_DESC rd{};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	rd.DepthClipEnable = TRUE;
	if (FAILED(device_->CreateRasterizerState(&rd, &rs_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: rasterizer create failed.");
		blend_->Release();
		blend_ = nullptr;
		sampler_->Release();
		sampler_ = nullptr;
		cb_->Release();
		cb_ = nullptr;
		layout_->Release();
		layout_ = nullptr;
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	rd.ScissorEnable = TRUE;
	if (FAILED(device_->CreateRasterizerState(&rd, &rs_scissor_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: rasterizer (scissor) create failed.");
		rs_->Release();
		rs_ = nullptr;
		blend_->Release();
		blend_ = nullptr;
		sampler_->Release();
		sampler_ = nullptr;
		cb_->Release();
		cb_ = nullptr;
		layout_->Release();
		layout_ = nullptr;
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = FALSE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	if (FAILED(device_->CreateDepthStencilState(&dsd, &dss_no_depth_)))
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "RmlUi D3D11: depth-stencil state create failed.");
		rs_scissor_->Release();
		rs_scissor_ = nullptr;
		rs_->Release();
		rs_ = nullptr;
		blend_->Release();
		blend_ = nullptr;
		sampler_->Release();
		sampler_ = nullptr;
		cb_->Release();
		cb_ = nullptr;
		layout_->Release();
		layout_ = nullptr;
		vs_->Release();
		vs_ = nullptr;
		ps_->Release();
		ps_ = nullptr;
		return;
	}

	pipeline_ok_ = true;
}

void RmlUiRenderInterfaceD3D11::UploadConstantBuffer(const float* row_major_m16)
{
	D3D11_MAPPED_SUBRESOURCE map{};
	if (FAILED(ctx_->Map(cb_, 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
	{
		return;
	}
	std::memcpy(map.pData, row_major_m16, sizeof(float) * 16);
	ctx_->Unmap(cb_, 0);
	ctx_->VSSetConstantBuffers(0, 1, &cb_);
}

void RmlUiRenderInterfaceD3D11::SetTransformsForDraw(Rml::Vector2f translation)
{
	Rml::Matrix4f world = Rml::Matrix4f::Translate(translation.x, translation.y, 0.f);
	if (element_transform_)
	{
		world = (*element_transform_) * world;
	}
	Rml::Matrix4f mvp = projection_ * world;
	UploadConstantBuffer(mvp.data());
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
