#include "SDL3RenderLayout.h"
#include "SDL3GraphicsBuffer.h"
#include <base/ZEngine.h>
#include <common/Log.h>

namespace RenderWorker
{

namespace
{

// Must stay in sync with D3D12_INTERNAL_ConvertVertexInputState (TEXCOORD+location)
// and RemapSdlVsInputSemantics() in SDL3ShaderObject.cpp.
Uint32 LocationFromUsage(VertexElementUsage usage, uint8_t usage_index)
{
	switch (usage)
	{
	case VEU_Position:		return 0;
	case VEU_Normal:		return 1;
	case VEU_Diffuse:		return 2;
	case VEU_Specular:		return 3;
	case VEU_BlendWeight:	return 4;
	case VEU_BlendIndex:	return 5;
	case VEU_TextureCoord:	return 6u + (usage_index > 7 ? 7u : static_cast<Uint32>(usage_index));
	case VEU_Tangent:		return 14;
	case VEU_Binormal:		return 15;
	default:				return 6;
	}
}

SDL_GPUVertexElementFormat MapVertexFormat(VertexElement const& ve)
{
	// Blend indices are integer IDs even when stored as EF_ABGR8 (same as D3D11).
	if (ve.usage == VEU_BlendIndex && ve.format == EF_ABGR8)
	{
		return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4;
	}

	switch (ve.format)
	{
	case EF_R32F:			return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
	case EF_GR32F:			return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	case EF_BGR32F:			return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	case EF_ABGR32F:		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	case EF_ABGR16F:		return SDL_GPU_VERTEXELEMENTFORMAT_HALF4;
	case EF_SIGNED_GR16:	return SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM;
	case EF_SIGNED_ABGR16:	return SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM;
	case EF_GR16:			return SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM;
	case EF_ABGR16:			return SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM;
	case EF_SIGNED_ABGR8:	return SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM;
	case EF_ABGR8:			return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
	case EF_ARGB8:			return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
	case EF_ABGR8UI:		return SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4;
	case EF_ABGR16UI:		return SDL_GPU_VERTEXELEMENTFORMAT_USHORT4;
	default:
		return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	}
}

} // namespace

void SDL3RenderLayout::Active() const
{
	uint32_t const num_streams = VertexStreamNum();

	// Delay-created VBs call CreateHWResource after an earlier Active(); always
	// refresh GPU pointers even when streams_dirty_ is false.
	std::vector<SDL_GPUBuffer*> new_vbs(num_streams, nullptr);
	for (uint32_t i = 0; i < num_streams; ++i)
	{
		auto const& gb = GetVertexStream(i);
		auto const* sdl_vb = gb ? dynamic_cast<SDL3GraphicsBuffer const*>(gb.get()) : nullptr;
		if (sdl_vb && sdl_vb->HWResourceReady())
		{
			new_vbs[i] = sdl_vb->GpuBuffer();
		}
	}

	bool const buffers_changed = (new_vbs != vbs_);
	if (streams_dirty_ || buffers_changed || attrs_.empty())
	{
		vb_descs_.resize(num_streams);
		attrs_.clear();

		for (uint32_t i = 0; i < num_streams; ++i)
		{
			vb_descs_[i] = {};
			vb_descs_[i].slot = i;
			vb_descs_[i].pitch = VertexSize(i);
			vb_descs_[i].input_rate = (VertexStreamType(i) == ST_Instance)
				? SDL_GPU_VERTEXINPUTRATE_INSTANCE
				: SDL_GPU_VERTEXINPUTRATE_VERTEX;
			// SDL_GPU currently requires instance_step_rate == 0 for all buffers.
			vb_descs_[i].instance_step_rate = 0;

			uint32_t offset = 0;
			for (auto const& ve : VertexStreamFormat(i))
			{
				SDL_GPUVertexAttribute attr{};
				attr.location = LocationFromUsage(ve.usage, ve.usage_index);
				attr.buffer_slot = i;
				attr.format = MapVertexFormat(ve);
				attr.offset = offset;
				attrs_.push_back(attr);
				offset += ve.element_size();
			}
		}

		bool has_position = false;
		for (auto const& attr : attrs_)
		{
			if (attr.location == 0)
			{
				has_position = true;
				break;
			}
		}
		if ((num_streams > 0) && attrs_.empty())
		{
			LogError() << "[SDL3] RenderLayout has " << num_streams
					   << " vertex stream(s) but 0 attributes after Active()" << std::endl;
		}
		else if ((num_streams > 0) && !has_position)
		{
			LogWarn() << "[SDL3] RenderLayout has no VEU_Position attribute (POSITION semantic missing)"
					  << std::endl;
		}

		streams_dirty_ = false;
	}

	vbs_ = std::move(new_vbs);
	vb_offsets_.assign(num_streams, 0);

	ib_ = nullptr;
	index_size_ = SDL_GPU_INDEXELEMENTSIZE_16BIT;
	if (UseIndices())
	{
		auto const& is = GetIndexStream();
		if (is)
		{
			auto const* sdl_ib = dynamic_cast<SDL3GraphicsBuffer const*>(is.get());
			if (sdl_ib && sdl_ib->HWResourceReady())
			{
				ib_ = sdl_ib->GpuBuffer();
			}
			index_size_ = (IndexStreamFormat() == EF_R32UI) ? SDL_GPU_INDEXELEMENTSIZE_32BIT
															: SDL_GPU_INDEXELEMENTSIZE_16BIT;
		}
	}
}

} // namespace RenderWorker
