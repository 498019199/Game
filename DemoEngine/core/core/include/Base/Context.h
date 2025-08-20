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

class WinAPP;
class RenderEngine;
class RenderFactory;
class World;
class ResLoader;

class Context final
{
public:
    Context();
    ~Context() noexcept;

    void LoadConfig(const char* file_name);
    void SaveConfig();

    static Context& Instance();

    const ContextConfig& Config() const noexcept;
    void Config(const ContextConfig& cfg) noexcept;

    void AppInstance(WinAPP& app) noexcept;
    WinAPP& AppInstance() noexcept;
    RenderEngine& RenderEngineInstance() noexcept;
    RenderFactory& RenderFactoryInstance() noexcept;
    World& WorldInstance() noexcept;
    ResLoader& ResLoaderInstance() noexcept;
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}



