
#include <base/Context.h>

#include "D3D11RenderLayout.h"
#include "D3D11GraphicsBuffer.h"
#include "D3D11RenderEngine.h"
#include "D3D11ShaderObject.h"

namespace RenderWorker
{
using namespace CommonWorker;

D3D11RenderLayout::D3D11RenderLayout() = default;

void D3D11RenderLayout::Active() const
{
    if (streams_dirty_)
    {
        uint32_t const num_vertex_streams = VertexStreamNum();
        vertex_elems_.clear();
        vertex_elems_.reserve(num_vertex_streams);

        vbs_.resize(num_vertex_streams);
        strides_.resize(num_vertex_streams);
        offsets_.resize(num_vertex_streams);
        for (uint32_t i = 0; i < num_vertex_streams; ++ i)
        {
            std::vector<D3D11_INPUT_ELEMENT_DESC> stream_elems;
            D3D11Mapping::Mapping(stream_elems, i, VertexStreamFormat(i), vertex_streams_[i].type, vertex_streams_[i].freq);
            vertex_elems_.insert(vertex_elems_.end(), stream_elems.begin(), stream_elems.end());

            const D3D11GraphicsBuffer& d3dvb = checked_cast<const D3D11GraphicsBuffer&>(*GetVertexStream(i));
            vbs_[i] = d3dvb.D3DBuffer();
            strides_[i] = VertexSize(i);
            offsets_[i] = 0;
        }

        streams_dirty_ = false;
    }
}

ID3D11InputLayout* D3D11RenderLayout::InputLayout(const ShaderObject* so) const
{
    if (!vertex_elems_.empty())
    {
        const D3D11ShaderObject& shader = checked_cast<const D3D11ShaderObject&>(*so);
        auto const signature = shader.VsSignature();

        for (auto const & il : input_layouts_)
        {
            if (il.first == signature)
            {
                return il.second.get();
            }
        }

        auto vs_code = shader.VsCode();
        const auto& d3d11_re = checked_cast<const D3D11RenderEngine&>(Context::Instance().RenderEngineInstance());
        auto d3d_device = d3d11_re.D3DDevice();
        ID3D11InputLayoutPtr new_layout;
        TIFHR(d3d_device->CreateInputLayout(&vertex_elems_[0], static_cast<UINT>(vertex_elems_.size()),
            vs_code.data(), vs_code.size(), new_layout.put()));
        auto* new_layout_raw = new_layout.get();
        input_layouts_.emplace_back(signature, std::move(new_layout)); // The emplace_back in VS2015 doesn't have a return value
		return new_layout_raw;
    }

    return nullptr;
}

}