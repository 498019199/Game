#include <base/App3D.h>
#include <base/DevHelper.h>
#include <base/ResLoader.h>
#include <base/ZEngine.h>
#include <common/CpuInfo.h>
#include <common/Log.h>
#include <common/Profiler.h>
#include <common/Thread.h>
#include <render/RenderEngine.h>
#include <render/RenderFactory.h>
#include <render/RenderMaterial.h>
#include <render/TexCompression.h>
#include <render/Texture.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <vector>

namespace
{
constexpr char const* kDdsExt = ".dds";
constexpr char const* kMetaExt = ".kmeta";

struct ConvertOptions
{
	std::vector<std::string> inputs;
	std::vector<std::string> assets_dirs;
	std::string config_path;
	uint32_t jobs{0}; // 0 = auto (hardware concurrency)
	bool force{false};
	bool show_help{false};
	bool write_kmeta{true};
};

void PrintUsage()
{
	std::cout
		<< "ZEngine texture converter\n\n"
		<< "Usage:\n"
		<< "  ZENGINE_texture_convert [options] <tex|dir> [tex|dir...]\n\n"
		<< "Arguments:\n"
		<< "  tex/dir               Resource path (e.g. Models/a.tga), absolute file, or directory\n\n"
		<< "Options:\n"
		<< "  -h, --help            Show this help\n"
		<< "  -f, --force           Rebuild .dds even when up to date\n"
		<< "  -j, --jobs <N>        Parallel file conversions (default: HW threads)\n"
		<< "  --no-kmeta            Do not write missing .kmeta (slot stays default albedo)\n"
		<< "  --config <path>       KlayGE.cfg path (default: auto locate)\n"
		<< "  --assets-dir <dir>    Add resource search path (repeatable)\n\n"
		<< "Output:\n"
		<< "  foo.tga  -> foo.tga.dds (+ foo.tga.kmeta when missing)\n"
		<< "  Slot is inferred from filename (normal/height/metal/emissive/albedo).\n";
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
		else if ((arg == "-j") || (arg == "--jobs"))
		{
			if ((i + 1) >= argc)
			{
				throw std::runtime_error("--jobs requires a positive integer");
			}
			int const jobs = std::stoi(argv[++i]);
			if (jobs <= 0)
			{
				throw std::runtime_error("--jobs must be >= 1");
			}
			options.jobs = static_cast<uint32_t>(jobs);
		}
		else if (arg == "--no-kmeta")
		{
			options.write_kmeta = false;
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

std::string ToLowerAscii(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return value;
}

bool IsSourceTextureExt(std::filesystem::path const& path)
{
	auto const ext = ToLowerAscii(path.extension().string());
	static std::unordered_set<std::string> const kExts{
		".tga", ".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff", ".gif", ".hdr", ".exr"};
	return kExts.contains(ext);
}

RenderWorker::RenderMaterial::TextureSlot InferSlot(std::string_view name)
{
	std::string const lower = ToLowerAscii(std::string(name));
	if (lower.find("normal") != std::string::npos)
	{
		return RenderWorker::RenderMaterial::TS_Normal;
	}
	if ((lower.find("height") != std::string::npos) || (lower.find("disp") != std::string::npos))
	{
		return RenderWorker::RenderMaterial::TS_Height;
	}
	if ((lower.find("metal") != std::string::npos) || (lower.find("gloss") != std::string::npos)
		|| (lower.find("rough") != std::string::npos))
	{
		return RenderWorker::RenderMaterial::TS_MetalnessGlossiness;
	}
	if ((lower.find("emissive") != std::string::npos) || (lower.find("emit") != std::string::npos))
	{
		return RenderWorker::RenderMaterial::TS_Emissive;
	}
	return RenderWorker::RenderMaterial::TS_Albedo;
}

std::string RuntimeDdsName(std::string_view tex_name)
{
	std::string runtime_name(tex_name);
	if (std::filesystem::path(runtime_name).extension() != kDdsExt)
	{
		runtime_name += kDdsExt;
	}
	return runtime_name;
}

struct ResolvedInput
{
	std::string res_name;
	std::string source_path;
};

std::string GenericPathString(std::filesystem::path const& path)
{
	return path.generic_string();
}

// Map a filesystem path under .../Assets/ to a ResLoader name such as "Models/foo.tga".
bool TryMakeAssetsRelativeName(std::filesystem::path const& abs_path, std::string& out_res_name)
{
	std::error_code ec;
	std::filesystem::path cur = abs_path;
	while (!cur.empty())
	{
		if (ToLowerAscii(cur.filename().string()) == "assets")
		{
			std::filesystem::path const relative = abs_path.lexically_relative(cur);
			if (relative.empty() || (*relative.begin() == ".."))
			{
				return false;
			}
			out_res_name = GenericPathString(relative);
			return true;
		}
		auto const parent = cur.parent_path();
		if (parent == cur)
		{
			break;
		}
		cur = parent;
	}
	return false;
}

ResolvedInput ResolveInput(RenderWorker::ResLoader& res_loader, std::string const& input)
{
	std::error_code ec;
	std::filesystem::path input_path(input);

	// Prefer existing resource names (Models/...).
	if (input_path.is_relative())
	{
		std::string const located = res_loader.Locate(GenericPathString(input_path));
		if (!located.empty() && !std::filesystem::is_directory(located, ec))
		{
			return ResolvedInput{GenericPathString(input_path), located};
		}
	}

	std::filesystem::path abs_path = std::filesystem::absolute(input_path, ec);
	if (ec || !std::filesystem::exists(abs_path, ec) || std::filesystem::is_directory(abs_path, ec))
	{
		return {};
	}
	abs_path = std::filesystem::weakly_canonical(abs_path, ec);

	std::string res_name;
	if (!TryMakeAssetsRelativeName(abs_path, res_name))
	{
		std::string const parent_path = abs_path.parent_path().string();
		if (!res_loader.IsInPath(parent_path))
		{
			res_loader.AddPath(parent_path);
		}
		res_name = abs_path.filename().string();
	}

	return ResolvedInput{res_name, abs_path.string()};
}

void CollectInputs(RenderWorker::ResLoader& res_loader, std::string const& input, std::vector<std::string>& out)
{
	std::error_code ec;
	std::filesystem::path path(input);
	if (!std::filesystem::exists(path, ec))
	{
		// Resource-style dir: Models/chaos_knight -> locate a child if possible, else Assets/<input>
		std::filesystem::path assets_candidate;
		std::string const probe = res_loader.Locate("Models");
		if (!probe.empty())
		{
			assets_candidate = std::filesystem::path(probe).parent_path() / path;
		}
		if (!assets_candidate.empty() && std::filesystem::exists(assets_candidate, ec))
		{
			path = assets_candidate;
		}
	}

	if (std::filesystem::is_directory(path, ec))
	{
		for (auto const& entry : std::filesystem::recursive_directory_iterator(
				 std::filesystem::weakly_canonical(path, ec), ec))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}
			if (!IsSourceTextureExt(entry.path()))
			{
				continue;
			}
			out.push_back(entry.path().string());
		}
		if (out.empty())
		{
			CommonWorker::LogError() << "No source textures under: " << path.string() << std::endl;
		}
		return;
	}

	ResolvedInput const resolved = ResolveInput(res_loader, input);
	if (!resolved.res_name.empty())
	{
		out.push_back(resolved.source_path.empty() ? input : resolved.source_path);
	}
	else
	{
		CommonWorker::LogError() << "Could NOT find input texture/dir: " << input << std::endl;
	}
}

bool NeedsRebuild(RenderWorker::ResLoader& res_loader, std::string const& source_path, std::string const& runtime_name, bool force)
{
	if (force)
	{
		return true;
	}

	std::string const runtime_path = res_loader.Locate(runtime_name);
	if (runtime_path.empty())
	{
		return true;
	}

	std::error_code ec;
	auto const src_time = std::filesystem::last_write_time(source_path, ec);
	if (ec)
	{
		return true;
	}
	auto const dst_time = std::filesystem::last_write_time(runtime_path, ec);
	if (ec)
	{
		return true;
	}
	return src_time > dst_time;
}

char const* SlotToString(RenderWorker::RenderMaterial::TextureSlot slot)
{
	using Slot = RenderWorker::RenderMaterial::TextureSlot;
	switch (slot)
	{
	case Slot::TS_MetalnessGlossiness:
		return "metalness_glossiness";
	case Slot::TS_Emissive:
		return "emissive";
	case Slot::TS_Normal:
		return "normal";
	case Slot::TS_Height:
		return "height";
	case Slot::TS_Albedo:
	default:
		return "albedo";
	}
}

char const* SlotToPreferredFormat(RenderWorker::RenderMaterial::TextureSlot slot)
{
	using Slot = RenderWorker::RenderMaterial::TextureSlot;
	switch (slot)
	{
	case Slot::TS_Normal:
	case Slot::TS_MetalnessGlossiness:
		return "BC5";
	case Slot::TS_Height:
		return "BC4";
	case Slot::TS_Emissive:
	case Slot::TS_Albedo:
	default:
		// Prefer BC1 for offline cook speed; BC7 Quality/Balanced is too slow in Debug.
		return "BC1_SRGB";
	}
}

void EnsureKmeta(std::string const& source_path, std::string const& res_name, RenderWorker::RenderMaterial::TextureSlot slot)
{
	std::filesystem::path const kmeta_path = std::filesystem::path(source_path).string() + kMetaExt;
	if (std::filesystem::exists(kmeta_path))
	{
		return;
	}

	std::ofstream ofs(kmeta_path);
	if (!ofs)
	{
		throw std::runtime_error("Failed to write kmeta: " + kmeta_path.string());
	}
	ofs << "{\n"
		<< "  \"version\": 1,\n"
		<< "  \"type\": \"2D\",\n"
		<< "  \"slot\": \"" << SlotToString(slot) << "\",\n"
		<< "  \"prefered_format\": \"" << SlotToPreferredFormat(slot) << "\",\n"
		<< "  \"source\": \"" << res_name << "\"\n"
		<< "}\n";
	CommonWorker::LogInfo() << "Wrote " << kmeta_path.string() << std::endl;
}

class TextureConvertApp final : public RenderWorker::App3D
{
public:
	explicit TextureConvertApp(ConvertOptions options)
		: App3D("Texture Convert"), options_(std::move(options))
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

		if (!context.EnsureDevHelper())
		{
			CommonWorker::LogError() << "DevHelper is unavailable." << std::endl;
			exit_code_ = 1;
			return;
		}

		for (auto const& assets_dir : options_.assets_dirs)
		{
			res_loader.AddPath(assets_dir);
		}

		std::vector<std::string> textures;
		for (auto const& input : options_.inputs)
		{
			CollectInputs(res_loader, input, textures);
		}
		if (textures.empty())
		{
			CommonWorker::LogError() << "No source textures found." << std::endl;
			exit_code_ = 1;
			return;
		}

		CommonWorker::CpuInfo cpu;
		uint32_t jobs = options_.jobs;
		if (jobs == 0)
		{
			jobs = cpu.NumHWThreads();
			if (jobs <= 1)
			{
				jobs = std::max<uint32_t>(1, static_cast<uint32_t>(std::thread::hardware_concurrency()));
			}
		}
		jobs = std::max<uint32_t>(1, std::min<uint32_t>(jobs, static_cast<uint32_t>(textures.size())));

		// Parallelize across files; keep per-texture encode single-threaded to avoid oversubscription.
		RenderWorker::SetTexEncodeThreadHint(1);

		auto& caps = context.RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
		auto& helper = context.DevHelperInstance();

		std::atomic<uint32_t> next_index{0};
		std::atomic<uint32_t> converted{0};
		std::atomic<uint32_t> skipped{0};
		std::atomic<uint32_t> failed{0};

		CommonWorker::LogInfo() << "Converting " << textures.size() << " texture(s) with " << jobs << " job(s)"
								<< std::endl;

		auto worker = [&]() {
			for (;;)
			{
				uint32_t const index = next_index.fetch_add(1, std::memory_order_relaxed);
				if (index >= textures.size())
				{
					break;
				}
				if (!ConvertOne(res_loader, helper, caps, textures[index], converted, skipped))
				{
					failed.fetch_add(1, std::memory_order_relaxed);
				}
			}
		};

		if (jobs == 1)
		{
			worker();
		}
		else
		{
			std::vector<std::future<void>> joiners(jobs);
			for (uint32_t i = 0; i < jobs; ++i)
			{
				joiners[i] = CommonWorker::CreateThread(worker);
			}
			for (auto& joiner : joiners)
			{
				joiner.wait();
			}
		}

		RenderWorker::SetTexEncodeThreadHint(0);

		CommonWorker::LogInfo() << "Done. converted=" << converted.load() << " skipped=" << skipped.load()
								<< " failed=" << failed.load() << std::endl;
		if (failed.load() > 0)
		{
			exit_code_ = 1;
		}
	}

	uint32_t DoUpdate(uint32_t /*pass*/) override
	{
		return URV_Finished;
	}

private:
	bool ConvertOne(RenderWorker::ResLoader& res_loader, RenderWorker::DevHelper& helper,
		RenderWorker::RenderDeviceCaps const& caps, std::string const& input, std::atomic<uint32_t>& converted,
		std::atomic<uint32_t>& skipped)
	{
		ZENGINE_ZONE("texture_convert.ConvertOne");
		ResolvedInput const resolved = ResolveInput(res_loader, input);
		if (resolved.res_name.empty())
		{
			CommonWorker::LogError() << "Could NOT find input texture: " << input << std::endl;
			return false;
		}

		std::string const runtime_name = RuntimeDdsName(resolved.res_name);
		if (!NeedsRebuild(res_loader, resolved.source_path, runtime_name, options_.force))
		{
			CommonWorker::LogInfo() << "Up to date: " << runtime_name << std::endl;
			skipped.fetch_add(1, std::memory_order_relaxed);
			return true;
		}

		if (options_.force)
		{
			std::string const runtime_path = res_loader.Locate(runtime_name);
			if (!runtime_path.empty())
			{
				std::error_code ec;
				std::filesystem::remove(runtime_path, ec);
			}
		}

		auto const slot = InferSlot(resolved.res_name);
		std::string const metadata_name = resolved.res_name + kMetaExt;
		if (options_.write_kmeta)
		{
			EnsureKmeta(resolved.source_path, resolved.res_name, slot);
		}

		CommonWorker::LogInfo() << "Converting " << resolved.source_path << " -> " << runtime_name
								<< " (slot=" << static_cast<int>(slot) << ")" << std::endl;

		ZoneNamedN(tracyConvertTexture, "texture_convert.ConvertTexture", true);
		auto texture = helper.ConvertTexture(resolved.res_name, metadata_name, runtime_name, &caps);
		if (!texture)
		{
			CommonWorker::LogError() << "Conversion failed: " << resolved.source_path << std::endl;
			return false;
		}

		std::string const output_path = res_loader.Locate(runtime_name);
		if (output_path.empty())
		{
			CommonWorker::LogError() << "Conversion finished but output was NOT found: " << runtime_name << std::endl;
			return false;
		}

		CommonWorker::LogInfo() << "Saved " << output_path << std::endl;
		converted.fetch_add(1, std::memory_order_relaxed);
		return true;
	}

private:
	ConvertOptions options_;
	int exit_code_{0};
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

		TextureConvertApp app(std::move(options));
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
