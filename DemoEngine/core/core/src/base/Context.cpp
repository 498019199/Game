#include <base/Context.h>
#include <base/ResLoader.h>

#include <common/Util.h>
#include <common/ResIdentifier.h>
#include <common/XMLDom.h>

#include <render/ElementFormat.h>

#include <filesystem>
#include <fstream>
namespace RenderWorker
{
using namespace CommonWorker;

class Context::Impl final
{
public:
    static Context& Instance()
    {
        if (!context_instance_)
        {
            std::lock_guard<std::mutex> lock(singleton_mutex_);
            if (!context_instance_)
            {
                context_instance_ = MakeUniquePtr<Context>();
            }
        }
        return *context_instance_;
    }

    WinAPP& AppInstance() noexcept;
    RenderEngine& RenderEngineInstance() noexcept;
    RenderFactory& RenderFactoryInstance() noexcept;
    World& WorldInstance() noexcept;
    ResLoader& ResLoaderInstance() noexcept;

private:
	static std::mutex singleton_mutex_;
	static std::unique_ptr<Context> context_instance_;
};
std::mutex Context::Impl::singleton_mutex_;
std::unique_ptr<Context> Context::Impl::context_instance_;

Context& Context::Instance()
{
    return Impl::Instance();
}

WinAPP& Context::AppInstance() noexcept
{
    return pimpl_->AppInstance();
}

RenderEngine& Context::RenderEngineInstance() noexcept
{
    return pimpl_->RenderEngineInstance();
}

RenderFactory& Context::RenderFactoryInstance() noexcept
{
    return pimpl_->RenderFactoryInstance();
}

World& Context::WorldInstance() noexcept
{
    return pimpl_->WorldInstance();
}

ResLoader& Context::ResLoaderInstance() noexcept
{
    return pimpl_->ResLoaderInstance();
}

void Context::LoadConfig(const char* file_name)
{
    uint32_t width = 800;
    uint32_t height = 600;
    ElementFormat color_fmt = EF_ARGB8;
    ElementFormat depth_stencil_fmt = EF_D16;
    uint32_t sample_count = 1;
    uint32_t sample_quality = 0;
    bool full_screen = false;
    uint32_t sync_interval = 0;
    bool hdr = false;
    bool ppaa = false;
    bool gamma = false;
    bool color_grading = false;
    float bloom = 0.25f;
    bool blue_shift = true;
    bool keep_screen_on = true;
    StereoMethod stereo_method = STM_None;
    float stereo_separation = 0;
    DisplayOutputMethod display_output_method = DOM_sRGB;
    uint32_t paper_white = 100;
    uint32_t display_max_luminance = 100;
    std::vector<std::pair<std::string, std::string>> graphics_options;
    bool debug_context = false;
    bool perf_profiler = false;
    bool location_sensor = false;

    auto& res_loader = ResLoaderInstance();
    ResIdentifierPtr file = res_loader.Open(file_name);
    if(!file)
    {
        return;
    }

    XMLNode cfg_root = LoadXml(*file);
}

void Context::SaveConfig(const char* file_name)
{

}
}