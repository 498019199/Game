#pragma once
#include <Base/Context.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#define PTR_PUT_VOID(ptr) reinterpret_cast<void**>(&ptr)

namespace RenderWorker
{
struct D3D11VideoMode 
{
    D3D11VideoMode(uint32_t width, uint32_t height, DXGI_FORMAT format)
        :width_(width), height_(height), format_(format)
    {}

    bool operator==(D3D11VideoMode const& rhs) const noexcept;
    bool operator<(D3D11VideoMode const& rhs) const noexcept;

    uint32_t		width_;
    uint32_t		height_;
    DXGI_FORMAT		format_;
};

class D3D11Adapter final
{
public:
    D3D11Adapter(uint32_t adapter_no, IDXGIAdapter2* adapter);

    void Enumerate();

    // 访问设备描述字符串
    std::wstring const Description() const;
    void ResetAdapter(IDXGIAdapter2* adapter);

private:
    // 这是第几块适配器
    uint32_t			adapter_no_;

    // 适配器信息
    IDXGIAdapter2* adapter_;
    DXGI_ADAPTER_DESC2 adapter_desc_{};
	
    // 显示模式列表
    std::vector<D3D11VideoMode> modes_;
};

using UniqueD3D11AdapterPtr = std::unique_ptr<D3D11Adapter>;
using D3D11AdapterPtr = std::shared_ptr<D3D11Adapter>;
}