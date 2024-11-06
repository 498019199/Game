#include "D3D11AdapterList.h"

namespace CoreWorker
{
D3D11AdapterList::D3D11AdapterList() noexcept = default;

void D3D11AdapterList::Destroy()
{
    adapters_.clear();
    current_adapter_ = 0;
}

// 获取系统显卡数目
/////////////////////////////////////////////////////////////////////////////////
size_t D3D11AdapterList::NumAdapter() const noexcept
{
    return adapters_.size();
}

// 获取当前显卡索引
/////////////////////////////////////////////////////////////////////////////////
uint32_t D3D11AdapterList::CurrentAdapterIndex() const noexcept
{
    return current_adapter_;
}

// 设置当前显卡索引
/////////////////////////////////////////////////////////////////////////////////
void D3D11AdapterList::CurrentAdapterIndex(uint32_t index) noexcept
{
    current_adapter_ = index;
}

// 获取显卡
/////////////////////////////////////////////////////////////////////////////////
D3D11Adapter* D3D11AdapterList::Adapter(size_t index) const
{
    COMMON_ASSERT(index < adapters_.size());
    return adapters_[index].get();
}

// 枚举系统显卡
/////////////////////////////////////////////////////////////////////////////////
void D3D11AdapterList::Enumerate(IDXGIFactory2* gi_factory)
{
    // 枚举系统中的适配器
    UINT adapter_no = 0;
    IDXGIAdapter1* dxgi_adapter1;
    while (gi_factory->EnumAdapters1(adapter_no, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND)
    {
        if (dxgi_adapter1 != nullptr)
        {
            IDXGIAdapter2* dxgi_adapter2;
            dxgi_adapter1->QueryInterface(__uuidof(IDXGIAdapter2), PTR_PUT_VOID(dxgi_adapter2));
            auto& adapter = adapters_.emplace_back(CommonWorker::MakeUniquePtr<D3D11Adapter>(adapter_no, dxgi_adapter2));
            adapter->Enumerate();
            
        }

        ++ adapter_no;
    }
}

void D3D11AdapterList::Enumerate(IDXGIFactory6* gi_factory)
{
    UINT adapter_no = 0;
	IDXGIAdapter2* dxgi_adapter;
	while (SUCCEEDED(gi_factory->EnumAdapterByGpuPreference(
        adapter_no, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter2), PTR_PUT_VOID(dxgi_adapter))))
	{
		if (dxgi_adapter != nullptr)
		{
            auto& adapter = adapters_.emplace_back(CommonWorker::MakeUniquePtr<D3D11Adapter>(adapter_no, dxgi_adapter));
            adapter->Enumerate();
        }
        ++ adapter_no;
    }
}
}