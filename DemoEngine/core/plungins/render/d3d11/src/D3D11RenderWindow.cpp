#include "D3D11RenderWindow.h"
namespace RenderWorker
{
D3D11RenderWindow::D3D11RenderWindow(D3D11Adapter* adapter, const std::string& name, RenderSettings const& settings)
{
    adapter_ = adapter;
}

D3D11RenderWindow::~D3D11RenderWindow()
{
    
}
}