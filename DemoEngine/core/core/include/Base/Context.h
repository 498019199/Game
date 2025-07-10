#pragma once
#include <memory>
#include <cstdint>
#include <common/common.h>

namespace RenderWorker
{
using namespace CommonWorker;

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

class WinAPP;
class RenderEngine;
class RenderFactory;
class World;
class ResourceLoad;

class Context final
{
public:
    Context() = default;
    ~Context() = default;

    void LoadConfig(const char* file_name);
    
    static Context& Instance();

    WinAPP& AppInstance() noexcept;
    RenderEngine& RenderEngineInstance() noexcept;
    RenderFactory& RenderFactoryInstance() noexcept;
    World& WorldInstance() noexcept;
    ResourceLoad& ResourceLoadInstance() noexcept;
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};
}



