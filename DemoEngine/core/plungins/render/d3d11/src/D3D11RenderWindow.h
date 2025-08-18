#pragma once
#include <render/FrameBuffer.h>
#include "D3D11AdapterList.h"

namespace RenderWorker
{
class D3D11RenderWindow:public FrameBuffer
{
public:
    D3D11RenderWindow(D3D11Adapter* adapter, const std::string& name, RenderSettings const& settings);
    ~D3D11RenderWindow();
};
}