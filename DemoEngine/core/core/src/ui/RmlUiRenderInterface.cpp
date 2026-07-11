#include <base/RmlUiRenderInterface.h>

#include <base/ZEngine.h>
#include <common/Log.h>
#include <math/math.h>
#include <render/ElementFormat.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/RenderLayout.h>

#include <RmlUi/Core/Matrix4.h>
#include <RmlUi/Core/Vertex.h>

#include <algorithm>
#include <cstring>
#include <vector>

namespace RenderWorker
{
namespace
{
#pragma pack(push, 1)
struct RmlVertexPacked
{
	float x;
	float y;
	uint32_t color;
	float u;
	float v;
};
#pragma pack(pop)

struct CompiledGeometryEffect
{
	RenderLayoutPtr layout;
};
} // namespace

RmlUiRenderInterfaceD3D11::RmlUiRenderInterfaceD3D11()
{
	UpdateProjection();
	pipeline_ok_ = BuildPipeline();
}

RmlUiRenderInterfaceD3D11::~RmlUiRenderInterfaceD3D11()
{
	textures_.clear();
	white_tex_.reset();
	effect_.reset();
}

void RmlUiRenderInterfaceD3D11::UpdateProjection()
{
	// Y-down UI ortho: top=0, bottom=height. MathWorker args: (l,r,b,t, far, near).
	projection_ = MathWorker::OrthoOffCenterLH(0.f, float(viewport_width_), float(viewport_height_), 0.f, 10000.f,
		-10000.f);
}

bool RmlUiRenderInterfaceD3D11::BuildPipeline()
{
	if (!Context::Instance().RenderFactoryValid())
	{
		LogError() << "RmlUiRenderInterface: RenderFactory unavailable." << std::endl;
		return false;
	}

	effect_ = SyncLoadRenderEffect("RmlUi.shader");
	if (!effect_)
	{
		LogError() << "RmlUiRenderInterface: failed to load RmlUi.shader." << std::endl;
		return false;
	}

	tech_ = effect_->TechniqueByName("RmlUiTech");
	tech_scissor_ = effect_->TechniqueByName("RmlUiScissorTech");
	transform_ep_ = effect_->ParameterByName("Transform");
	rml_tex_ep_ = effect_->ParameterByName("rml_tex");
	if (!tech_ || !tech_scissor_ || !transform_ep_ || !rml_tex_ep_)
	{
		LogError() << "RmlUiRenderInterface: missing technique or parameters in RmlUi.shader." << std::endl;
		return false;
	}
	if (!tech_->Validate() || !tech_scissor_->Validate())
	{
		LogError() << "RmlUiRenderInterface: RmlUi techniques are invalid." << std::endl;
		return false;
	}

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	uint32_t const white = 0xffffffffu;
	ElementInitData init {};
	init.data = &white;
	init.row_pitch = 4;
	init.slice_pitch = 4;
	white_tex_ = rf.MakeTexture2D(1, 1, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init));
	if (!white_tex_)
	{
		LogError() << "RmlUiRenderInterface: failed to create white texture." << std::endl;
		return false;
	}

	return true;
}

void RmlUiRenderInterfaceD3D11::SetViewportSize(int width, int height)
{
	viewport_width_ = (std::max)(1, width);
	viewport_height_ = (std::max)(1, height);
	UpdateProjection();
}

void RmlUiRenderInterfaceD3D11::SetTransformsForDraw(Rml::Vector2f translation)
{
	// Row-vector convention: oPos = mul(pos, Transform), Transform = T * Elem * Proj.
	float4x4 world = MathWorker::translation(translation.x, translation.y, 0.f);
	if (has_element_transform_)
	{
		world *= element_transform_;
	}
	*transform_ep_ = world * projection_;
}

Rml::CompiledGeometryHandle RmlUiRenderInterfaceD3D11::CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
	Rml::Span<const int> indices)
{
	if (!pipeline_ok_ || vertices.empty() || indices.empty())
	{
		return {};
	}

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	std::vector<RmlVertexPacked> packed(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		auto const& v = vertices[i];
		packed[i].x = v.position.x;
		packed[i].y = v.position.y;
		std::memcpy(&packed[i].color, &v.colour, sizeof(uint32_t));
		packed[i].u = v.tex_coord.x;
		packed[i].v = v.tex_coord.y;
	}

	auto* geo = new CompiledGeometryEffect {};
	geo->layout = rf.MakeRenderLayout();
	geo->layout->TopologyType(RenderLayout::TT_TriangleList);

	GraphicsBufferPtr const vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
		static_cast<uint32_t>(packed.size() * sizeof(RmlVertexPacked)), packed.data());
	GraphicsBufferPtr const ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable,
		static_cast<uint32_t>(indices.size() * sizeof(int)), indices.data());
	if (!vb || !ib)
	{
		delete geo;
		return {};
	}

	geo->layout->BindVertexStream(vb,
		MakeSpan({VertexElement(VEU_Position, 0, EF_GR32F), VertexElement(VEU_Diffuse, 0, EF_ABGR8),
			VertexElement(VEU_TextureCoord, 0, EF_GR32F)}));
	geo->layout->BindIndexStream(ib, EF_R32UI);

	return reinterpret_cast<Rml::CompiledGeometryHandle>(geo);
}

void RmlUiRenderInterfaceD3D11::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
	Rml::TextureHandle texture)
{
	if (!pipeline_ok_ || !effect_ || !transform_ep_ || !rml_tex_ep_ || geometry == 0)
	{
		return;
	}

	auto* geo = reinterpret_cast<CompiledGeometryEffect*>(geometry);
	if (!geo->layout)
	{
		return;
	}

	SetTransformsForDraw(translation);

	TexturePtr tex = white_tex_;
	if (texture)
	{
		auto const it = textures_.find(static_cast<uint64_t>(texture));
		if (it != textures_.end() && it->second)
		{
			tex = it->second;
		}
	}
	*rml_tex_ep_ = tex;

	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderTechnique* tech = tech_;
	if (scissor_enabled_)
	{
		tech = tech_scissor_;
		re.ScissorRect(static_cast<uint32_t>(scissor_region_.Left()), static_cast<uint32_t>(scissor_region_.Top()),
			static_cast<uint32_t>(scissor_region_.Right() + 1), static_cast<uint32_t>(scissor_region_.Bottom() + 1));
	}
	else
	{
		re.ScissorRect(0, 0, static_cast<uint32_t>(viewport_width_), static_cast<uint32_t>(viewport_height_));
	}

	re.Render(*effect_, *tech, *geo->layout);
}

void RmlUiRenderInterfaceD3D11::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
	if (geometry == 0)
	{
		return;
	}
	delete reinterpret_cast<CompiledGeometryEffect*>(geometry);
}

Rml::TextureHandle RmlUiRenderInterfaceD3D11::LoadTexture(Rml::Vector2i& texture_dimensions, Rml::String const& /*source*/)
{
	texture_dimensions = {0, 0};
	return {};
}

Rml::TextureHandle RmlUiRenderInterfaceD3D11::GenerateTexture(Rml::Span<const Rml::byte> source,
	Rml::Vector2i source_dimensions)
{
	if (!pipeline_ok_ || source.empty() || source_dimensions.x <= 0 || source_dimensions.y <= 0)
	{
		return {};
	}

	uint32_t const w = static_cast<uint32_t>(source_dimensions.x);
	uint32_t const h = static_cast<uint32_t>(source_dimensions.y);

	ElementInitData init {};
	init.data = source.data();
	init.row_pitch = w * 4u;
	init.slice_pitch = init.row_pitch * h;

	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	TexturePtr tex = rf.MakeTexture2D(w, h, 1, 1, EF_ABGR8, 1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init));
	if (!tex)
	{
		return {};
	}

	uint64_t const id = next_texture_id_++;
	textures_[id] = std::move(tex);
	return static_cast<Rml::TextureHandle>(id);
}

void RmlUiRenderInterfaceD3D11::ReleaseTexture(Rml::TextureHandle texture_handle)
{
	if (!texture_handle)
	{
		return;
	}
	textures_.erase(static_cast<uint64_t>(texture_handle));
}

void RmlUiRenderInterfaceD3D11::EnableScissorRegion(bool enable)
{
	scissor_enabled_ = enable;
}

void RmlUiRenderInterfaceD3D11::SetScissorRegion(Rml::Rectanglei region)
{
	scissor_region_ = region;
}

void RmlUiRenderInterfaceD3D11::SetTransform(Rml::Matrix4f const* transform)
{
	if (transform)
	{
		has_element_transform_ = true;
		// Rml Matrix4f is column-major; float4x4(float const*) reads as row-major → loads M^T.
		element_transform_ = float4x4(transform->data());
	}
	else
	{
		has_element_transform_ = false;
		element_transform_ = float4x4::Identity();
	}
}

} // namespace RenderWorker
