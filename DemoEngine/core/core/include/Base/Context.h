#pragma once
#include <Base/WinApp.h>
#include <World/World.h>
#include <render/RenderFactory.h>
#include <render/RenderEngine.h>

namespace RenderWorker
{
struct RenderSettings
{
    bool    full_screen = false;
    int		left = 0;
    int		top = 0;

    int		width;
    int		height;

    uint32_t sample_count = 1;
    uint32_t sample_quality = 0;
};

class Context
{
public:
    Context();
    ~Context() = default;

    static Context& Instance();

    void AppInstance(WinAPP& app);
    WinAPP& AppInstance();

    void RenderEngineInstance(RenderEngine& render_engine);
    RenderEngine& RenderEngineInstance() const;

    RenderFactory& RenderFactoryInstance();
    World& WorldInstance();
    
    void LoadConfig(const char* file_name);

    const std::string& GetWorkPath() const;
    void AddResource(const std::string& Path);
    const std::string& GetResourcePath() const;

    static ResIdentifierPtr OpenFile(std::string_view FileName);
    static ResIdentifierPtr OpenFile(const std::string& FileName);

    static std::string Locate(std::string_view name);
private:
    static std::unique_ptr<Context> instance_;
    WinAPP* app_;

    RenderEngine* render_engine_;
    RenderFactoryPtr render_factory_;
    WorldPtr world_;

    std::string work_path_;
    std::string resource_path_;
};












}



