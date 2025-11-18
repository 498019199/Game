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
    std::string audio_factory_name;
    std::string audio_data_source_factory_name;
    // 图形配置
    RenderSettings graphics_cfg;
    
    bool deferred_rendering;
    bool perf_profiler;
    bool location_sensor;
};

class App3D;
class RenderEngine;
class RenderFactory;
class AudioFactory;
class AudioDataSourceFactory;
class World;
class ThreadPool;
class ResLoader;
class DevHelper;

class ZENGINE_CORE_API Context final
{
    Context(const Context& rhs) = delete; 
    Context& operator=(const Context& rhs) = delete;
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
    
	AudioFactory& AudioFactoryInstance();
    World& WorldInstance() noexcept;
    
    RenderFactory& RenderFactoryInstance() noexcept;
    bool RenderFactoryValid() const noexcept;

    bool AudioDataSourceFactoryValid() const noexcept;
    AudioDataSourceFactory& AudioDataSourceFactoryInstance();

#if ZENGINE_IS_DEV_PLATFORM
	bool DevHelperValid() const noexcept;
	DevHelper& DevHelperInstance();
#endif

    ThreadPool& ThreadPoolInstance();
    ResLoader& ResLoaderInstance();

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}



