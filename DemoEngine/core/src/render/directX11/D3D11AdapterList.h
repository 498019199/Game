#pragma once

#include <vector>
#include "D3D11Adapter.h"
namespace CoreWorker
{

class D3D11AdapterList final
{
public:
    D3D11AdapterList() noexcept;
    void Destroy();

    size_t NumAdapter() const noexcept;
	D3D11Adapter* Adapter(size_t index) const;

	uint32_t CurrentAdapterIndex() const noexcept;
	void CurrentAdapterIndex(uint32_t index) noexcept;

    void Enumerate(IDXGIFactory2* gi_factory);
	void Enumerate(IDXGIFactory6* gi_factory);
private:
    uint32_t current_adapter_{0};
    std::vector<UniqueD3D11AdapterPtr> adapters_;
};
}