#include <base/Context.h>
#include <base/ResLoader.h>
#include <base/Thread.h>

#include <common/Util.h>
#include <common/ResIdentifier.h>
#include <common/XMLDom.h>
#include <common/XMLDom.h>

#include <render/ElementFormat.h>
#include <render/RenderFactory.h>
#include <world/World.h>

extern "C"
{
	void MakeRenderFactory(std::unique_ptr<RenderWorker::RenderFactory>& ptr);
	void MakeRenderWorld(std::unique_ptr<RenderWorker::World>& ptr);
}

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

    void AppInstance(WinAPP& app) noexcept
    {
        app_ = &app;
    }

    WinAPP& AppInstance() noexcept
    {
        COMMON_ASSERT(app_);
       // COMMON_ASSUME(app_);
        return *app_;
    }

    RenderEngine& RenderEngineInstance() noexcept
    {
        return render_factory_->RenderEngineInstance();
    }

    RenderFactory& RenderFactoryInstance() noexcept
    {
        if (!render_factory_)
        {
            std::lock_guard<std::mutex> lock(singleton_mutex_);
            if (!render_factory_)
            {
                this->LoadRenderFactory(cfg_.render_factory_name);
            }
        }
        return *render_factory_;
    }

    World& WorldInstance() noexcept
    {
        if (!render_world_)
        {
            std::lock_guard<std::mutex> lock(singleton_mutex_);
            if (!render_world_)
            {
                this->LoadRenderWorld(cfg_.render_world_name);
            }
        }
        return *render_world_;
    }

    ThreadPool& ThreadPoolInstance()
    {
        return global_thread_pool_;
    }

    ResLoader& ResLoaderInstance() noexcept
    {
        if (!res_loader_.Valid())
        {
            std::lock_guard<std::mutex> lock(singleton_mutex_);
            if (!res_loader_.Valid())
            {
                res_loader_.Init(global_thread_pool_);
            }
        }

        return res_loader_;
    }

    void LoadRenderFactory(const std::string& name)
    {
        MakeRenderFactory(render_factory_);
    }

    void LoadRenderWorld(const std::string& name)
    {
        MakeRenderWorld(render_world_);
    }

    void Config(const ContextConfig& cfg)
    {
        cfg_ = cfg;
    }

    const ContextConfig& Config() const
    {
        return cfg_;
    }

    void LoadConfig(const char* file_name)
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
        XMLNode const* context_node = cfg_root.FirstNode("context");
        XMLNode const* graphics_node = cfg_root.FirstNode("graphics");

        if (XMLNode const* perf_profiler_node = context_node->FirstNode("perf_profiler"))
        {
            perf_profiler = perf_profiler_node->Attrib("enabled")->ValueInt() ? true : false;
        }
        if (XMLNode const* location_sensor_node = context_node->FirstNode("location_sensor"))
        {
            location_sensor = location_sensor_node->Attrib("enabled")->ValueInt() ? true : false;
        }

        // 屏幕宽高
        XMLNode const* frame_node = graphics_node->FirstNode("frame");
        if (XMLAttribute const* attr = frame_node->Attrib("width"))
        {
            width = attr->ValueUInt();
        }
        if (XMLAttribute const* attr = frame_node->Attrib("height"))
        {
            height = attr->ValueUInt();
        }
        std::string color_fmt_str = "ARGB8";
        if (XMLAttribute const* attr = frame_node->Attrib("color_fmt"))
        {
            color_fmt_str = std::string(attr->ValueString());
        }
        std::string depth_stencil_fmt_str = "D16";
        if (XMLAttribute const* attr = frame_node->Attrib("depth_stencil_fmt"))
        {
            depth_stencil_fmt_str = std::string(attr->ValueString());
        }
        if (XMLAttribute const* attr = frame_node->Attrib("fullscreen"))
        {
            full_screen = attr->ValueBool();
        }
        if (XMLAttribute const* attr = frame_node->Attrib("keep_screen_on"))
        {
            keep_screen_on = attr->ValueBool();
        }

        size_t const color_fmt_str_hash = RtHash(color_fmt_str.c_str());
        if (CtHash("ARGB8") == color_fmt_str_hash)
        {
            color_fmt = EF_ARGB8;
        }
        else if (CtHash("ABGR8") == color_fmt_str_hash)
        {
            color_fmt = EF_ABGR8;
        }
        else if (CtHash("A2BGR10") == color_fmt_str_hash)
        {
            color_fmt = EF_A2BGR10;
        }
        else if (CtHash("ABGR16F") == color_fmt_str_hash)
        {
            color_fmt = EF_ABGR16F;
        }

        size_t const depth_stencil_fmt_str_hash = RtHash(depth_stencil_fmt_str.c_str());
        if (CtHash("D16") == depth_stencil_fmt_str_hash)
        {
            depth_stencil_fmt = EF_D16;
        }
        else if (CtHash("D24S8") == depth_stencil_fmt_str_hash)
        {
            depth_stencil_fmt = EF_D24S8;
        }
        else if (CtHash("D32F") == depth_stencil_fmt_str_hash)
        {
            depth_stencil_fmt = EF_D32F;
        }
        else
        {
            depth_stencil_fmt = EF_Unknown;
        }

        // 多重采样
        XMLNode const* sample_node = frame_node->FirstNode("sample");
        if (XMLAttribute const* attr = sample_node->Attrib("count"))
        {
            sample_count = attr->ValueUInt();
        }
        if (XMLAttribute const* attr = sample_node->Attrib("quality"))
        {
            sample_quality = attr->ValueUInt();
        }

        cfg_.graphics_cfg.left = cfg_.graphics_cfg.top = 0;
        cfg_.graphics_cfg.width = width;
        cfg_.graphics_cfg.height = height;
        cfg_.graphics_cfg.color_fmt = color_fmt;
        cfg_.graphics_cfg.depth_stencil_fmt = depth_stencil_fmt;
        cfg_.graphics_cfg.sample_count = sample_count;
        cfg_.graphics_cfg.sample_quality = sample_quality;
        cfg_.graphics_cfg.full_screen = full_screen;
        cfg_.graphics_cfg.sync_interval = sync_interval;
        cfg_.graphics_cfg.hdr = hdr;
        cfg_.graphics_cfg.ppaa = ppaa;
        cfg_.graphics_cfg.gamma = gamma;
        cfg_.graphics_cfg.color_grading = color_grading;
        cfg_.graphics_cfg.bloom = bloom;
        cfg_.graphics_cfg.blue_shift = blue_shift;
        cfg_.graphics_cfg.keep_screen_on = keep_screen_on;
        cfg_.graphics_cfg.stereo_method = stereo_method;
        cfg_.graphics_cfg.stereo_separation = stereo_separation;
        cfg_.graphics_cfg.display_output_method = display_output_method;
        cfg_.graphics_cfg.paper_white = paper_white;
        cfg_.graphics_cfg.display_max_luminance = display_max_luminance;
        cfg_.graphics_cfg.options = std::move(graphics_options);
        cfg_.graphics_cfg.debug_context = debug_context;

        cfg_.deferred_rendering = false;
        cfg_.perf_profiler = perf_profiler;
        cfg_.location_sensor = location_sensor;
    }

    void SaveConfig()
    {
        
    }
private:
	static std::mutex singleton_mutex_;
	static std::unique_ptr<Context> context_instance_;

    // 窗口实例
    WinAPP* app_;

    // 基础配置
    ContextConfig cfg_;

    // 分配渲染相关对象接口
    std::unique_ptr<RenderFactory> render_factory_;
    // 场景对象管理
    std::unique_ptr<World> render_world_;
    // 载入资源管理器
    ResLoader res_loader_;
    // 全局线程池
    ThreadPool global_thread_pool_;
};

std::mutex Context::Impl::singleton_mutex_;
std::unique_ptr<Context> Context::Impl::context_instance_;

Context& Context::Instance()
{
    return Impl::Instance();
}

void Context::AppInstance(WinAPP& app) noexcept
{
    pimpl_->AppInstance(app);
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

const ContextConfig& Context::Config() const noexcept
{
    return pimpl_->Config();
}

void Context::Config(const ContextConfig& cfg) noexcept
{
    pimpl_->Config(cfg);
}

void Context::LoadConfig(const char* file_name)
{
    pimpl_->LoadConfig(file_name);
}

void Context::SaveConfig()
{
    pimpl_->SaveConfig();
}

}