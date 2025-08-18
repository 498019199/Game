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
    std::string render_factory_name;
    std::string render_engine_name;
    std::string render_world_name;

    RenderSettings graphics_cfg;

    bool deferred_rendering;
    bool perf_profiler;
};

class WinAPP;
class RenderEngine;
class RenderFactory;
class World;
class ResLoader;

class Context final
{
public:
    Context() = default;
    ~Context() = default;

    void LoadConfig(const char* file_name);
    void SaveConfig();

    static Context& Instance();

    const ContextConfig& Config() const noexcept;
    void Config(const ContextConfig& cfg) noexcept;

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



