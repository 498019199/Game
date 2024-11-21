#pragma once

#include <vector>

namespace CoreWorker
{
class D3D9Adapter;
class D3D9AdapterList final
{
public:
private:
    uint8_t CurrentAdapter_;
    std::vector<D3D9Adapter> Adapters_;
};
}