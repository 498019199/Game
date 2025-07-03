#pragma once
#include <render/RenderLayout.h>
#include <render/ShaderObject.h>

#include "D3D11Util.h"

namespace RenderWorker
{
class D3D11RenderLayout final: public RenderLayout
{
public:
    D3D11RenderLayout();

    void Active() const;
    ID3D11InputLayout* InputLayout(const ShaderObject* so) const;

    const std::vector<ID3D11Buffer*> & VBs() const
    {
        return vbs_;
    }
    const std::vector<UINT> & Strides() const
    {
        return strides_;
    }
    const std::vector<UINT> & Offsets() const
    {
        return offsets_;
    }
private:
    mutable std::vector<D3D11_INPUT_ELEMENT_DESC> vertex_elems_;
    mutable std::vector<std::pair<uint32_t, ID3D11InputLayoutPtr>> input_layouts_;
    
    mutable std::vector<ID3D11Buffer*> vbs_;
    mutable std::vector<UINT> strides_;
    mutable std::vector<UINT> offsets_;
};
}
