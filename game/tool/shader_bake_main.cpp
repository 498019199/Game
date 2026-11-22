#include <base/DesktopBackend.h>
#include <base/App3D.h>
#include <base/ResLoader.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <render/BuiltInEffects.h>
#include <render/RenderEffect.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{

struct BakeOptions
{
    std::vector<std::string> effects;
    std::vector<std::string> assets_dirs;
    std::string config_path;
    std::string factory; // Empty selects the configured renderer.
    bool all_effects = false;
    bool force = false;
    bool show_help = false;
};

void PrintUsage()
{
    std::cout
        << "ZEngine C++ effect bake (platform-specific .kfx)\n\n"
        << "Usage:\n"
        << "  ZENGINE_shader_bake [options] --effect <name> [--effect <name>...]\n"
        << "  ZENGINE_shader_bake [options] --all-effects\n\n"
        << "Options:\n"
        << "  --effect <name>       Bake a built-in effect ID; repeatable\n"
        << "  --all-effects         Bake every registered C++ effect\n"
        << "  -h, --help            Show this help\n"
        << "  -f, --force           Delete the selected effect's platform cache before bake\n"
        << "  --factory <name>      D3D12 or D3D11 on Windows, Metal on macOS\n"
        << "  --config <path>       KlayGE.cfg path\n"
        << "  --assets-dir <dir>    Add resource search path (repeatable)\n\n"
        << "Outputs use EffectDesc::CacheName beside the effect's first HLSL source.\n"
        << "Unsupported techniques are skipped; every effect needs a valid technique.\n\n"
        << "Built-in effects:\n";
    for (auto const name : RenderWorker::BuiltInEffectNames())
    {
        std::cout << "  " << name << '\n';
    }
}

BakeOptions ParseArgs(int argc, char* argv[])
{
    BakeOptions options;
    for (int i = 1; i < argc; ++i)
    {
        std::string_view const arg = argv[i];
        auto value = [&]() -> std::string
        {
            if (i + 1 >= argc || std::string_view(argv[i + 1]).starts_with("--"))
            {
                throw std::runtime_error(std::string(arg) + " requires a value");
            }
            return argv[++i];
        };
        if (arg == "-h" || arg == "--help") options.show_help = true;
        else if (arg == "-f" || arg == "--force") options.force = true;
        else if (arg == "--all-effects") options.all_effects = true;
        else if (arg == "--effect") options.effects.push_back(value());
        else if (arg == "--factory") options.factory = value();
        else if (arg == "--config") options.config_path = value();
        else if (arg == "--assets-dir") options.assets_dirs.push_back(value());
        else throw std::runtime_error("Unexpected argument: " + std::string(arg) + ". Use --effect <name> or --all-effects.");
    }
    if (options.all_effects)
    {
        for (auto const name : RenderWorker::BuiltInEffectNames()) options.effects.emplace_back(name);
    }
    std::vector<std::string> unique_effects;
    for (auto& name : options.effects)
    {
        if (std::find(unique_effects.begin(), unique_effects.end(), name) == unique_effects.end())
        {
            unique_effects.push_back(std::move(name));
        }
    }
    options.effects = std::move(unique_effects);
    return options;
}

std::string_view UnsupportedReason(RenderWorker::TechniqueDesc const& technique,
    RenderWorker::RenderDeviceCaps const& caps)
{
    using RenderWorker::ShaderStage;
    if (technique.minimum_shader_model > caps.max_shader_model) return "shader model";
    for (auto const& pass : technique.passes)
    {
        auto uses = [&](ShaderStage stage) { return !pass.entry_points[std::to_underlying(stage)].empty(); };
        if (uses(ShaderStage::Geometry) && !caps.gs_support) return "geometry shaders";
        if (uses(ShaderStage::Compute) && !caps.cs_support) return "compute shaders";
        if (uses(ShaderStage::Hull) && !caps.hs_support) return "hull shaders";
        if (uses(ShaderStage::Domain) && !caps.ds_support) return "domain shaders";
    }
    return {};
}

void RemoveIfExists(std::filesystem::path const& path)
{
    std::error_code error;
    bool const removed = std::filesystem::remove(path, error);
    if (error) throw std::runtime_error("Cannot remove effect cache " + path.string() + ": " + error.message());
    if (removed) CommonWorker::LogInfo() << "Removed " << path.string() << std::endl;
}

class ShaderBakeApp final : public RenderWorker::App3D
{
public:
    explicit ShaderBakeApp(BakeOptions options)
        : App3D("Shader Bake"), options_(std::move(options))
    {
    }

    int ExitCode() const noexcept { return exit_code_; }

    void OnCreate() override
    {
        auto& context = RenderWorker::Context::Instance();
        auto& engine = context.RenderFactoryInstance().RenderEngineInstance();
        CommonWorker::LogInfo() << "Baking C++ effects for platform '" << engine.NativeShaderPlatformName()
            << "' (factory=" << context.Config().render_factory_name << ")" << std::endl;
        for (auto const& name : options_.effects)
        {
            try
            {
                BakeOne(name, context.ResLoaderInstance(), engine);
                ++succeeded_;
            }
            catch (std::exception const& error)
            {
                CommonWorker::LogError() << "Failed to bake " << name << ": " << error.what() << std::endl;
                exit_code_ = 1;
            }
        }
        CommonWorker::LogInfo() << "Bake summary: " << succeeded_ << '/' << options_.effects.size()
            << " effects succeeded, " << options_.effects.size() - succeeded_ << " failed; "
            << valid_techniques_ << " valid techniques, " << skipped_techniques_ << " skipped; "
            << unchanged_caches_ << " cache files unchanged, " << succeeded_ - unchanged_caches_
            << " written" << std::endl;
    }

    uint32_t DoUpdate(uint32_t /*pass*/) override { return URV_Finished; }

private:
    void BakeOne(std::string const& name, RenderWorker::ResLoader& loader, RenderWorker::RenderEngine& engine)
    {
#if !ZENGINE_IS_DEV_PLATFORM
        throw std::runtime_error("Effect baking requires a development platform build");
#endif
        auto const description = RenderWorker::DescribeBuiltInEffect(name);
        description.Validate();
        auto const source = loader.Locate(description.shader_files.front());
        if (source.empty()) throw std::runtime_error("Missing HLSL: " + description.shader_files.front());
        auto const output = std::filesystem::path(source).parent_path() / description.CacheName(engine.NativeShaderPlatformName());
        if (options_.force) RemoveIfExists(output);
        bool const had_cache = std::filesystem::exists(output);
        auto const previous_write = had_cache ? std::filesystem::last_write_time(output) : std::filesystem::file_time_type{};

        // A fresh instance also rebakes effects preloaded while App3D creates its window.
        RenderWorker::RenderEffect effect;
        effect.Load(description);
#if ZENGINE_IS_DEV_PLATFORM
        effect.CompileShaders();
#endif
        effect.CreateHwShaders();
        uint32_t valid = 0;
        uint32_t skipped = 0;
        for (auto const& technique : description.techniques)
        {
            auto const reason = UnsupportedReason(technique, engine.DeviceCaps());
            if (!reason.empty())
            {
                ++skipped;
                CommonWorker::LogInfo() << "Skipped " << name << '/' << technique.name
                    << ": device does not support " << reason << std::endl;
                continue;
            }
            auto const* compiled = effect.TechniqueByName(technique.name);
            if (!compiled || !compiled->Validate()) throw std::runtime_error("Invalid technique: " + technique.name);
            ++valid;
        }
        if (valid == 0) throw std::runtime_error("No valid technique for the selected renderer");
        if (!std::filesystem::exists(output)) throw std::runtime_error("Expected effect cache missing: " + output.string());
        bool const unchanged = had_cache && std::filesystem::last_write_time(output) == previous_write;
        valid_techniques_ += valid;
        skipped_techniques_ += skipped;
        if (unchanged) ++unchanged_caches_;
        CommonWorker::LogInfo() << "Baked C++ effect " << name << " (" << valid << " valid, " << skipped
            << " skipped, cache " << (unchanged ? "unchanged" : "written") << "): " << output.string() << std::endl;
    }

    BakeOptions options_;
    int exit_code_ = 0;
    size_t succeeded_ = 0;
    size_t unchanged_caches_ = 0;
    uint32_t valid_techniques_ = 0;
    uint32_t skipped_techniques_ = 0;
};

} // namespace

int main(int argc, char* argv[])
{
    try
    {
        BakeOptions options = ParseArgs(argc, argv);
        if (options.show_help) { PrintUsage(); return 0; }
        if (options.effects.empty()) { PrintUsage(); return 2; }

#if !defined(ZENGINE_PLATFORM_WINDOWS)
        if (!options.factory.empty() && options.factory != RenderWorker::DesktopRendererName)
        {
            std::cerr << "Only --factory Metal is supported on this platform." << std::endl;
            return 2;
        }
#else
        if (!options.factory.empty() && options.factory != "D3D11" && options.factory != RenderWorker::DesktopRendererName)
        {
            std::cerr << "--factory must be D3D11 or D3D12." << std::endl;
            return 2;
        }
#endif
        // Reject unknown IDs before creating a device or changing any cache files.
        for (auto const& name : options.effects) RenderWorker::DescribeBuiltInEffect(name).Validate();
        auto& context = RenderWorker::Context::Instance();
        for (auto const& path : options.assets_dirs) context.ResLoaderInstance().AddPath(path);
        if (options.config_path.empty()) options.config_path = context.ResLoaderInstance().Locate("KlayGE.cfg");
        if (options.config_path.empty())
        {
            std::cerr << "Could NOT locate KlayGE.cfg. Use --config." << std::endl;
            return 2;
        }

        context.LoadConfig(options.config_path.c_str());
        RenderWorker::ContextConfig config = context.Config();
        if (!options.factory.empty()) config.render_factory_name = options.factory;
        config.graphics_cfg.width = 64;
        config.graphics_cfg.height = 64;
        config.graphics_cfg.full_screen = false;
        context.Config(config);

        ShaderBakeApp app(std::move(options));
        app.Create();
        app.Destroy();
        return app.ExitCode();
    }
    catch (std::exception const& error)
    {
        std::cerr << error.what() << std::endl;
        return 2;
    }
}
