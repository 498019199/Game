#pragma once
#include "D3D11Util.h"

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

    uint32_t AdapterNo() const noexcept
    {
        return adapter_no_;
    }

    IDXGIAdapter2* DXGIAdapter() const noexcept
    {
        return adapter_.get();
    }

    DXGI_FORMAT DesktopFormat() const noexcept
    {
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    }
private:
    // 这是第几块适配器
    uint32_t			adapter_no_;

    // 适配器信息
    IDXGIAdapter2Ptr adapter_;
    DXGI_ADAPTER_DESC2 adapter_desc_{};
	
    // 显示模式列表
    std::vector<D3D11VideoMode> modes_;
};

using UniqueD3D11AdapterPtr = std::unique_ptr<D3D11Adapter>;
using D3D11AdapterPtr = std::shared_ptr<D3D11Adapter>;
}