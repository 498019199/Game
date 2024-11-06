#include <core/IContext.h>
#include <core/IApp.h>
#include <common/common.h>
#include <common/instance.h>

#include <mini/ini.h>

#ifdef DEMOENGINE_PLATFORM_WINDOWS
#include "app/windows/WindowApp.h"
#include "render/directX11/D3D11RenderDevice.h"
#endif//DEMOENGINE_PLATFORM_WINDOWS

namespace CoreWorker {
namespace Context{

IContext* Instance()
{
    return CommonWorker::Singleton<IContext>::Instance();
}

bool IContext::LoadConfig(const std::string& filepath)
{
    mINI::INIFile file(filepath.c_str());
    mINI::INIStructure ini;
    if(!file.read(ini))
    {
        return false;
    }

    ConfigDesc_.full_screen = "1" == ini["graphics"]["fullscreen"];
    ConfigDesc_.keep_screen_on = "1" == ini["graphics"]["keep_screen_on"];
    ConfigDesc_.title = ini["graphics"]["title"];

    ConfigDesc_.win_desc.hide_win = false;
    ConfigDesc_.win_desc.m_nRenderType = RenderType::RENDER_TYPE_TEXTURE;
    ConfigDesc_.win_desc.top = 0;
    ConfigDesc_.win_desc.left = 0;
    ConfigDesc_.win_desc.width = std::stoi(ini["graphics"]["width"]);
    ConfigDesc_.win_desc.height = std::stoi(ini["graphics"]["height"]);
    ConfigDesc_.win_desc.full_screen = "1" == ini["graphics"]["fullscreen"];
    ConfigDesc_.win_desc.keep_screen_on = "1" == ini["graphics"]["keep_screen_on"];
    std::string color_fmt_str = "ARGB8";
    if(!ini["graphics"]["color_fmt"].empty())
    {
        color_fmt_str = ini["graphics"]["color_fmt"];
    }
    if("ARGB8" == color_fmt_str)
    {
        ConfigDesc_.win_desc.color_fmt = ElementFormat::EF_ARGB8;
    }
    else if("ABGR8" == color_fmt_str)
    {
        ConfigDesc_.win_desc.color_fmt = ElementFormat::EF_ABGR8;
    }
    else if("A2BGR10" == color_fmt_str)
    {
        ConfigDesc_.win_desc.color_fmt = ElementFormat::EF_A2BGR10;
    }
    else if("ABGR16F" == color_fmt_str)
    {
        ConfigDesc_.win_desc.color_fmt = ElementFormat::EF_ABGR16F;
    }

    std::string depth_stencil_fmt_str = "D16";
    if(!ini["graphics"]["depth_stencil_fmt"].empty())
    {
        depth_stencil_fmt_str = ini["graphics"]["depth_stencil_fmt"];
    }
    if("D16" == depth_stencil_fmt_str)
    {
        ConfigDesc_.win_desc.depth_stencil_fmt = ElementFormat::EF_D16;
    }
    else if("D24S8" == depth_stencil_fmt_str)
    {
        ConfigDesc_.win_desc.depth_stencil_fmt = ElementFormat::EF_D24S8;
    }
    else if("D32F" == depth_stencil_fmt_str)
    {
        ConfigDesc_.win_desc.depth_stencil_fmt = ElementFormat::EF_D32F;
    }
    else 
    {
        ConfigDesc_.win_desc.depth_stencil_fmt = ElementFormat::EF_Unknown;
    }

    ConfigDesc_.win_desc.sample_count = std::stoi(ini["sample"]["count"]);
    ConfigDesc_.win_desc.sample_quality = std::stoi(ini["sample"]["quality"]);
    return true;
}

std::string IContext::Locate(std::string_view name)
{
    if (name.empty())
    {
        return "";
    }

    std::string res_name = "D://DEMOENGINE//DEMOENGINE//game//resource" + std::string(name);
    //std::filesystem::path res_path = 
    //if (std::filesystem::exists(res_path.c_str()))
    {
        return res_name;
    }
    //return "";
}

void IContext::SetMainApp(const ShareAppPtr& pApp)
{
    app_ = pApp;
}

IApp* IContext::GetMainApp() const
{
    return app_.get();
}

IApp* IContext::CreateAppWindow()
{
    std::string strName = ConfigDesc_.title;
    auto Settings = ConfigDesc_.win_desc;

#ifdef DEMOENGINE_PLATFORM_WINDOWS
    auto WinApp = CommonWorker::MakeSharedPtr<WindowApp>(strName, Settings, nullptr);
#endif// DEMOENGINE_PLATFORM_WINDOWS

    SetMainApp(WinApp);
    if(WinApp)
    {
        return WinApp.get();
    }
    return nullptr;
}

IRenderDevice* IContext::GetRenderDevice()
{
    if(render_device_.use_count() == 0)
    {
        render_device_ = CommonWorker::MakeSharedPtr<D3D11RenderDevice>(
                static_cast<WindowApp*>(GetMainApp())->GetHWnd(), ConfigDesc_.nWidth_, ConfigDesc_.nHeight_);
        render_device_->CreateDevice();
    }
    return render_device_.get();
}

void IContext:: Quit()
{
    app_->Close();
    render_device_->Destroy();
}
}
}