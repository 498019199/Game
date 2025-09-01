#include <base/Context.h>
#include <base/WinApp.h>

#include "D3D11RenderWindow.h"
#include "D3D11RenderEngine.h"

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

    RenderFactory& rf = Context::Instance().RenderFactoryInstance();
    auto& d3d11_re = checked_cast<D3D11RenderEngine&>(rf.RenderEngineInstance());
    ID3D11Device1Ptr d3d_device = d3d11_re.D3DDevice1();
    ID3D11DeviceContext1Ptr d3d_imm_ctx;
}

D3D11RenderWindow::~D3D11RenderWindow()
{
    
}
}