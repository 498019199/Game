#pragma once
#include <memory>
#include <cstdint>
#include <common/common.h>
#include <base/RenderSettings.h>

namespace RenderWorker
{
using namespace CommonWorker;

struct ContextConfig
{
    RenderSettings graphics_cfg;

    bool deferred_rendering;
    bool perf_profiler;
    bool location_sensor;
};

class App3D;
class RenderEngine;
class RenderFactory;
class World;
class ResLoader;

class Context final
{
public:
    Context();
    ~Context() noexcept;

    static Context& Instance();
    static void Destroy() noexcept;
    void Suspend();
    
    void LoadConfig(const char* file_name);
    void SaveConfig();

    const ContextConfig& Config() const noexcept;
    void Config(const ContextConfig& cfg) noexcept;

    void AppInstance(App3D& app) noexcept;
    App3D& AppInstance() noexcept;
    RenderEngine& RenderEngineInstance() noexcept;
    RenderFactory& RenderFactoryInstance() noexcept;
    World& WorldInstance() noexcept;
    ResLoader& ResLoaderInstance() noexcept;

    bool RenderFactoryValid() const noexcept;
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}



