#include <base/App3D.h>
#include <base/ResLoader.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <render/RenderEffect.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>

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
	std::vector<std::string> shaders;
	std::vector<std::string> assets_dirs;
	std::string config_path;
	std::string factory; // D3D11 or SDL3; empty = use cfg
	bool force {false};
	bool show_help {false};
};

void PrintUsage()
{
	std::cout
		<< "ZEngine shader bake (platform-specific .kfx)\n\n"
		<< "Usage:\n"
		<< "  ZENGINE_shader_bake [options] <shader> [shader...]\n\n"
		<< "Arguments:\n"
		<< "  shader                e.g. SimpleAlbedoNormal.shader or Shaders/SkyBox.shader\n\n"
		<< "Options:\n"
		<< "  -h, --help            Show this help\n"
		<< "  -f, --force           Delete platform .kfx (and legacy .kfx) before bake\n"
		<< "  --factory <name>      Render factory: D3D11 or SDL3 (Windows)\n"
		<< "  --config <path>       KlayGE.cfg path\n"
		<< "  --assets-dir <dir>    Add resource search path (repeatable)\n\n"
		<< "Output (next to source):\n"
		<< "  Foo.shader + D3D11 -> Foo.d3d_11_0.kfx\n"
		<< "  Foo.shader + SDL3  -> Foo.d3d_12.kfx (Windows) / Foo.metal_spirv.kfx (macOS)\n";
}

BakeOptions ParseArgs(int argc, char* argv[])
{
	BakeOptions options;
	for (int i = 1; i < argc; ++i)
	{
		std::string_view arg = argv[i];
		if ((arg == "-h") || (arg == "--help"))
		{
			options.show_help = true;
		}
		else if ((arg == "-f") || (arg == "--force"))
		{
			options.force = true;
		}
		else if (arg == "--factory")
		{
			if ((i + 1) >= argc)
			{
				throw std::runtime_error("--factory requires D3D11 or SDL3");
			}
			options.factory = argv[++i];
		}
		else if (arg == "--config")
		{
			if ((i + 1) >= argc)
			{
				throw std::runtime_error("--config requires a path");
			}
			options.config_path = argv[++i];
		}
		else if (arg == "--assets-dir")
		{
			if ((i + 1) >= argc)
			{
				throw std::runtime_error("--assets-dir requires a directory");
			}
			options.assets_dirs.push_back(argv[++i]);
		}
		else if (!arg.empty() && (arg[0] == '-'))
		{
			throw std::runtime_error(std::string("Unknown option: ") + std::string(arg));
		}
		else
		{
			options.shaders.emplace_back(arg);
		}
	}
	return options;
}

void RemoveIfExists(std::filesystem::path const& path)
{
	std::error_code ec;
	if (std::filesystem::exists(path, ec))
	{
		std::filesystem::remove(path, ec);
		if (!ec)
		{
			CommonWorker::LogInfo() << "Removed " << path.string() << std::endl;
		}
	}
}

void ForceDeleteKfx(RenderWorker::ResLoader& res_loader, std::string const& shader_name,
	std::string_view platform_name)
{
	std::string const located = res_loader.Locate(shader_name);
	std::filesystem::path const base =
		located.empty() ? std::filesystem::path(shader_name) : std::filesystem::path(located);
	std::filesystem::path const dir = base.parent_path();
	std::string const stem = base.stem().string();

	RemoveIfExists(dir / (stem + "." + std::string(platform_name) + ".kfx"));
	RemoveIfExists(dir / (stem + ".kfx"));
}

class ShaderBakeApp final : public RenderWorker::App3D
{
public:
	explicit ShaderBakeApp(BakeOptions options)
		: App3D("Shader Bake"), options_(std::move(options))
	{
	}

	int ExitCode() const noexcept
	{
		return exit_code_;
	}

	void OnCreate() override
	{
		auto& context = RenderWorker::Context::Instance();
		auto& res_loader = context.ResLoaderInstance();
		auto& re = context.RenderFactoryInstance().RenderEngineInstance();
		std::string_view const platform = re.NativeShaderPlatformName();

		CommonWorker::LogInfo() << "Baking kfx for platform '" << platform << "' (factory="
								<< context.Config().render_factory_name << ")" << std::endl;

		for (auto const& assets_dir : options_.assets_dirs)
		{
			res_loader.AddPath(assets_dir);
		}

		for (auto const& shader : options_.shaders)
		{
			BakeOne(res_loader, platform, shader);
		}
	}

	uint32_t DoUpdate(uint32_t /*pass*/) override
	{
		return URV_Finished;
	}

private:
	void BakeOne(RenderWorker::ResLoader& res_loader, std::string_view platform, std::string const& shader)
	{
		std::string const located = res_loader.Locate(shader);
		if (located.empty())
		{
			CommonWorker::LogError() << "Could NOT find shader: " << shader << std::endl;
			exit_code_ = 1;
			return;
		}

		if (options_.force)
		{
			ForceDeleteKfx(res_loader, shader, platform);
		}

		CommonWorker::LogInfo() << "Loading/compiling " << located << std::endl;
		auto effect = RenderWorker::SyncLoadRenderEffect(shader);
		if (!effect)
		{
			CommonWorker::LogError() << "SyncLoadRenderEffect failed: " << shader << std::endl;
			exit_code_ = 1;
			return;
		}

		std::filesystem::path const dir = std::filesystem::path(located).parent_path();
		std::string const stem = std::filesystem::path(located).stem().string();
		std::filesystem::path const out = dir / (stem + "." + std::string(platform) + ".kfx");
		std::error_code ec;
		if (!std::filesystem::exists(out, ec))
		{
			CommonWorker::LogError() << "Expected kfx missing after bake: " << out.string()
									 << " (compile may have failed; check logs)" << std::endl;
			exit_code_ = 1;
			return;
		}

		CommonWorker::LogInfo() << "Wrote " << out.string() << std::endl;
	}

	BakeOptions options_;
	int exit_code_ {0};
};

} // namespace

int main(int argc, char* argv[])
{
	try
	{
		BakeOptions options = ParseArgs(argc, argv);
		if (options.show_help)
		{
			PrintUsage();
			return 0;
		}
		if (options.shaders.empty())
		{
			PrintUsage();
			return 2;
		}

#if !defined(ZENGINE_PLATFORM_WINDOWS)
		if (!options.factory.empty() && (options.factory != "SDL3"))
		{
			std::cerr << "Only --factory SDL3 is supported on this platform." << std::endl;
			return 2;
		}
#else
		if (!options.factory.empty() && (options.factory != "D3D11") && (options.factory != "SDL3"))
		{
			std::cerr << "--factory must be D3D11 or SDL3." << std::endl;
			return 2;
		}
#endif

		auto& context = RenderWorker::Context::Instance();
		if (options.config_path.empty())
		{
			options.config_path = context.ResLoaderInstance().Locate("KlayGE.cfg");
		}
		if (options.config_path.empty())
		{
			std::cerr << "Could NOT locate KlayGE.cfg. Use --config." << std::endl;
			return 2;
		}

		context.LoadConfig(options.config_path.c_str());
		RenderWorker::ContextConfig config = context.Config();
		if (!options.factory.empty())
		{
			config.render_factory_name = options.factory;
		}
		config.graphics_cfg.width = 64;
		config.graphics_cfg.height = 64;
		config.graphics_cfg.full_screen = false;
		context.Config(config);

		ShaderBakeApp app(std::move(options));
		app.Create();
		app.Destroy();
		return app.ExitCode();
	}
	catch (std::exception const& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 2;
	}
}
