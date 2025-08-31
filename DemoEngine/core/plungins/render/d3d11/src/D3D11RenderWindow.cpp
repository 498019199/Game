#include <base/Context.h>
#include <base/WinApp.h>

#include "D3D11RenderWindow.h"

namespace RenderWorker
{
D3D11RenderWindow::D3D11RenderWindow(D3D11Adapter* adapter, const std::string& name, RenderSettings const& settings)
{
    // Store info
    name_				= name;
    is_full_screen_		= settings.full_screen;
    sync_interval_		= settings.sync_interval;

    ElementFormat format = settings.color_fmt;

    const auto& Cfg = Context::Instance().Config();
	HWND hwnd = Context::Instance().AppInstance().GetHWND();
    adapter_ = adapter;
}

D3D11RenderWindow::~D3D11RenderWindow()
{
    
}
}