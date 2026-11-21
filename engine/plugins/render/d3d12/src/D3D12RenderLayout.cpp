#include "D3D12RenderLayout.h"
#include "D3D12GraphicsBuffer.h"
#include <base/ZEngine.h>
#include <common/Log.h>

namespace RenderWorker
{

namespace
{

// Must stay in sync with D3D12_INTERNAL_ConvertVertexInputState (TEXCOORD+location)
// and RemapVertexInputSemantics() in D3D12ShaderObject.cpp.
uint32_t LocationFromUsage(VertexElementUsage usage, uint8_t usage_index)
{
	switch (usage)
	{
	case VEU_Position:		return 0;
	case VEU_Normal:		return 1;
	case VEU_Diffuse:		return 2;
	case VEU_Specular:		return 3;
	case VEU_BlendWeight:	return 4;
	case VEU_BlendIndex:	return 5;
	case VEU_TextureCoord:	return 6u + (usage_index > 7 ? 7u : static_cast<uint32_t>(usage_index));
	case VEU_Tangent:		return 14;
	case VEU_Binormal:		return 15;
	default:				return 6;
	}
}

d3d12::VertexElementFormat MapVertexFormat(VertexElement const& ve)
{
	// Blend indices are integer IDs even when stored as EF_ABGR8 (same as D3D11).
	if (ve.usage == VEU_BlendIndex && ve.format == EF_ABGR8)
	{
		return d3d12::VERTEXELEMENTFORMAT_UBYTE4;
	}

	switch (ve.format)
	{
	case EF_R32F:			return d3d12::VERTEXELEMENTFORMAT_FLOAT;
	case EF_GR32F:			return d3d12::VERTEXELEMENTFORMAT_FLOAT2;
	case EF_BGR32F:			return d3d12::VERTEXELEMENTFORMAT_FLOAT3;
	case EF_ABGR32F:		return d3d12::VERTEXELEMENTFORMAT_FLOAT4;
	case EF_ABGR16F:		return d3d12::VERTEXELEMENTFORMAT_HALF4;
	case EF_SIGNED_GR16:	return d3d12::VERTEXELEMENTFORMAT_SHORT2_NORM;
	case EF_SIGNED_ABGR16:	return d3d12::VERTEXELEMENTFORMAT_SHORT4_NORM;
	case EF_GR16:			return d3d12::VERTEXELEMENTFORMAT_USHORT2_NORM;
	case EF_ABGR16:			return d3d12::VERTEXELEMENTFORMAT_USHORT4_NORM;
	case EF_SIGNED_ABGR8:	return d3d12::VERTEXELEMENTFORMAT_BYTE4_NORM;
	case EF_ABGR8:			return d3d12::VERTEXELEMENTFORMAT_UBYTE4_NORM;
	case EF_ARGB8:			return d3d12::VERTEXELEMENTFORMAT_UBYTE4_NORM;
	case EF_ABGR8UI:		return d3d12::VERTEXELEMENTFORMAT_UBYTE4;
	case EF_ABGR16UI:		return d3d12::VERTEXELEMENTFORMAT_USHORT4;
	default:
		return d3d12::VERTEXELEMENTFORMAT_FLOAT4;
	}
}

} // namespace

void D3D12RenderLayout::Active() const
{
	uint32_t const num_streams = VertexStreamNum();

	// Delay-created VBs call CreateHWResource after an earlier Active(); always
	// refresh GPU pointers even when streams_dirty_ is false.
	std::vector<d3d12::Buffer*> new_vbs(num_streams, nullptr);
	for (uint32_t i = 0; i < num_streams; ++i)
	{
		auto const& gb = GetVertexStream(i);
		auto const* native_vb = gb ? dynamic_cast<D3D12GraphicsBuffer const*>(gb.get()) : nullptr;
		if (native_vb && native_vb->HWResourceReady())
		{
			new_vbs[i] = native_vb->GpuBuffer();
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
				? d3d12::VERTEXINPUTRATE_INSTANCE
				: d3d12::VERTEXINPUTRATE_VERTEX;
			// d3d12:: currently requires instance_step_rate == 0 for all buffers.
			vb_descs_[i].instance_step_rate = 0;

			uint32_t offset = 0;
			for (auto const& ve : VertexStreamFormat(i))
			{
				d3d12::VertexAttribute attr{};
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
			LogError() << "[D3D12] RenderLayout has " << num_streams
					   << " vertex stream(s) but 0 attributes after Active()" << std::endl;
		}
		else if ((num_streams > 0) && !has_position)
		{
			LogWarn() << "[D3D12] RenderLayout has no VEU_Position attribute (POSITION semantic missing)"
					  << std::endl;
		}

		streams_dirty_ = false;
	}

	vbs_ = std::move(new_vbs);
	vb_offsets_.assign(num_streams, 0);

	ib_ = nullptr;
	index_size_ = d3d12::INDEXELEMENTSIZE_16BIT;
	if (UseIndices())
	{
		auto const& is = GetIndexStream();
		if (is)
		{
			auto const* native_ib = dynamic_cast<D3D12GraphicsBuffer const*>(is.get());
			if (native_ib && native_ib->HWResourceReady())
			{
				ib_ = native_ib->GpuBuffer();
			}
			index_size_ = (IndexStreamFormat() == EF_R32UI) ? d3d12::INDEXELEMENTSIZE_32BIT
															: d3d12::INDEXELEMENTSIZE_16BIT;
		}
	}
}

} // namespace RenderWorker
