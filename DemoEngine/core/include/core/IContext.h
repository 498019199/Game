#pragma once
#include <core/define.h>
#include <core/IApp.h>
#include <render/IRenderDevice.h>

namespace CoreWorker
{
struct FConfigDesc
{
    int nWidth_ = 800;
    int nHeight_ = 400;

    bool full_screen = false;
    bool keep_screen_on = true;

    int render_type = 0;
    std::string title;

    FWindowDesc win_desc;
};

namespace Context
{
class IContext final 
{
public:
    bool LoadConfig(const std::string& filepath);
    std::string Locate(std::string_view name);

    const FConfigDesc& GetConfig() const { return ConfigDesc_; }
    void SetConfig(const FConfigDesc& config) { ConfigDesc_ = config; }

    IApp* CreateAppWindow();
    void SetMainApp(const ShareAppPtr& pApp);
    IApp* GetMainApp() const;

    IRenderDevice* GetRenderDevice();

    void Quit();
private:
    FConfigDesc ConfigDesc_;
    ShareAppPtr app_;
    RenderDevicePtr render_device_;
};


IContext* Instance();
}
}


