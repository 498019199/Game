#pragma once
#include <common/common.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#define PTR_PUT_VOID(ptr) reinterpret_cast<void**>(&ptr)
#include <render/ElementFormat.h>

namespace CoreWorker
{
class D3D11Mapping final
{
public:
    static DXGI_FORMAT MappingFormat(ElementFormat format);
    static ElementFormat MappingFormat(DXGI_FORMAT d3dfmt);
};
}