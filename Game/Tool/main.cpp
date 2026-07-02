#include <base/App3D.h>
#include <base/ResLoader.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <render/Mesh.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
constexpr char const* kModelBinExt = ".model_bin";

struct ConvertOptions
{
	std::vector<std::string> inputs;
	std::vector<std::string> assets_dirs;
	std::string config_path;
	bool force { false };
	bool show_help { false };
};

void PrintUsage()
{
	std::cout
		<< "ZEngine model converter\n\n"
		<< "Usage:\n"
		<< "  ZENGINE_model_convert [options] <model> [model...]\n\n"
		<< "Arguments:\n"
		<< "  model                 Resource path (e.g. Models/Spring.obj) or absolute file path\n\n"
		<< "Options:\n"
		<< "  -h, --help            Show this help\n"
		<< "  -f, --force           Rebuild .model_bin even when up to date\n"
		<< "  --config <path>       KlayGE.cfg path (default: auto locate)\n"
		<< "  --assets-dir <dir>    Add resource search path (repeatable)\n\n"
		<< "Output:\n"
		<< "  Spring.obj   -> Spring.obj.model_bin\n"
		<< "  model.glb    -> model.glb.model_bin\n";
}

ConvertOptions ParseArgs(int argc, char* argv[])
{
	ConvertOptions options;
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
		else if (arg == "--config")
		{
			if ((i + 1) >= argc)
			{
				throw std::runtime_error("--config requires a path argument");
			}
			options.config_path = argv[++i];
		}
		else if (arg == "--assets-dir")
		{
			if ((i + 1) >= argc)
			{
				throw std::runtime_error("--assets-dir requires a path argument");
			}
			options.assets_dirs.push_back(argv[++i]);
		}
		else if (!arg.empty() && (arg[0] == '-'))
		{
			throw std::runtime_error(std::string("Unknown option: ") + std::string(arg));
		}
		else
		{
			options.inputs.emplace_back(arg);
		}
	}
	return options;
}

std::string RuntimeModelBinName(std::string_view model_name)
{
	std::string runtime_name(model_name);
	if (std::filesystem::path(runtime_name).extension() != kModelBinExt)
	{
		runtime_name += kModelBinExt;
	}
	return runtime_name;
}

struct ResolvedInput
{
	std::string res_name;
	std::string source_path;
};

ResolvedInput ResolveInput(RenderWorker::ResLoader& res_loader, std::string const& input)
{
	std::filesystem::path const input_path(input);
	if (input_path.is_relative())
	{
		std::string const located = res_loader.Locate(input);
		if (!located.empty())
		{
			return ResolvedInput { input, located };
		}
	}

	std::error_code ec;
	std::filesystem::path abs_path = std::filesystem::absolute(input_path, ec);
	if (ec || !std::filesystem::exists(abs_path, ec))
	{
		return {};
	}

	abs_path = std::filesystem::weakly_canonical(abs_path, ec);
	std::string const parent_path = abs_path.parent_path().string();
	if (!res_loader.IsInPath(parent_path))
	{
		res_loader.AddPath(parent_path);
	}

	return ResolvedInput { abs_path.filename().string(), abs_path.string() };
}

class ModelConvertApp final : public RenderWorker::App3D
{
public:
	explicit ModelConvertApp(ConvertOptions options)
		: App3D("Model Convert")
		, options_(std::move(options))
	{
	}

	int ExitCode() const noexcept
	{
		return exit_code_;
	}

	void OnCreate() override
	{
		auto& res_loader = RenderWorker::Context::Instance().ResLoaderInstance();

		if (!RenderWorker::Context::Instance().EnsureDevHelper())
		{
			CommonWorker::LogError() << "DevHelper is unavailable." << std::endl;
			exit_code_ = 1;
			return;
		}

		for (auto const& assets_dir : options_.assets_dirs)
		{
			res_loader.AddPath(assets_dir);
		}

		for (auto const& input : options_.inputs)
		{
			ConvertOne(res_loader, input);
		}
	}

	uint32_t DoUpdate(uint32_t /*pass*/) override
	{
		return URV_Finished;
	}

private:
	void ConvertOne(RenderWorker::ResLoader& res_loader, std::string const& input)
	{
		ResolvedInput const resolved = ResolveInput(res_loader, input);
		if (resolved.res_name.empty())
		{
			CommonWorker::LogError() << "Could NOT find input model: " << input << std::endl;
			exit_code_ = 1;
			return;
		}

		std::string const runtime_name = RuntimeModelBinName(resolved.res_name);
		if (options_.force)
		{
			std::string const runtime_path = res_loader.Locate(runtime_name);
			if (!runtime_path.empty())
			{
				std::error_code ec;
				std::filesystem::remove(runtime_path, ec);
			}
		}

		CommonWorker::LogInfo() << "Converting " << resolved.source_path << " -> " << runtime_name << std::endl;

		RenderWorker::RenderModelPtr model = RenderWorker::LoadSoftwareModel(resolved.res_name);
		if (!model)
		{
			CommonWorker::LogError() << "Conversion failed: " << resolved.source_path << std::endl;
			exit_code_ = 1;
			return;
		}

		std::string const output_path = res_loader.Locate(runtime_name);
		if (output_path.empty())
		{
			CommonWorker::LogError() << "Conversion finished but output was NOT found: " << runtime_name << std::endl;
			exit_code_ = 1;
			return;
		}

		CommonWorker::LogInfo() << "Saved " << output_path << std::endl;
	}

private:
	ConvertOptions options_;
	int exit_code_ { 0 };
};
} // namespace

int main(int argc, char* argv[])
{
	try
	{
		ConvertOptions options = ParseArgs(argc, argv);
		if (options.show_help)
		{
			PrintUsage();
			return 0;
		}
		if (options.inputs.empty())
		{
			PrintUsage();
			return 2;
		}

		auto& context = RenderWorker::Context::Instance();
		if (options.config_path.empty())
		{
			options.config_path = context.ResLoaderInstance().Locate("KlayGE.cfg");
		}
		if (options.config_path.empty())
		{
			std::cerr << "Could NOT locate KlayGE.cfg. Use --config to specify it." << std::endl;
			return 2;
		}

		context.LoadConfig(options.config_path.c_str());
		RenderWorker::ContextConfig config = context.Config();
		config.graphics_cfg.width = 64;
		config.graphics_cfg.height = 64;
		config.graphics_cfg.full_screen = false;
		context.Config(config);

		ModelConvertApp app(std::move(options));
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
