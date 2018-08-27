#include "../Container/macro.h"
#if defined STX_PLATFORM_WIN
#include <windows.h>
#pragma warning(push)
#pragma warning(disable: 4471) // A forward declaration of an unscoped enumeration must have an underlying type
#endif//STX_PLATFORM_WIN

#if defined(STX_PLATFORM_WIN)
#pragma warning(pop)
#endif

#include "ResLoader.h"
#include "../Container/Hash.h"
#include "Package.h"
#include "../Container/C++17/filesystem.h"
#include "../Util/UtilTool.h"

#include <fstream>
//#include "../Core/ICore.h"
namespace stx
{
	std::mutex singleton_mutex;
}
std::unique_ptr<ResLoader> ResLoader::m_InstanceResLoaderPtr;
ResLoader::ResLoader(Context* pContext)
	:IEntityEx(pContext),m_bQuit(false)
{
	m_InstanceResLoaderPtr = std::unique_ptr<ResLoader>(this);
#if defined STX_PLATFORM_WIN
	char buf[MAX_PATH];
	::GetModuleFileNameA(nullptr, buf, sizeof(buf));
	m_strExePath = buf;
	m_strExePath = m_strExePath.substr(0, m_strExePath.rfind("\\"));
	std::filesystem::path last_fxml_path(m_strExePath);
	std::filesystem::path last_fxml_directory = last_fxml_path.parent_path();

	m_strLocalPath = last_fxml_directory.string() + "/";
#else
	m_strLocalPath = m_strExePath;
#endif

	m_PathDataVec.push_back(std::make_tuple(CT_HASH(""), 0, "", PackagePtr()));
	this->AddPath("");
#if defined(STX_PLATFORM_WIN) || defined(STX_PLATFORM_LINUX) || defined(STX_PLATFORM_DARWIN)
	this->AddPath("..");
	this->AddPath("Res/ini/obj");
	this->AddPath("Res/ini/sample");
	this->AddPath("Res/ini/struct");
	this->AddPath("Res/ini/tex");
#endif
//	loading_thread_ = MakeUniquePtr<joiner<void>>(ICore::Instance().ThreadPool()(
//		[this] { this->LoadingThreadFunc(); }));
}

ResLoader::~ResLoader()
{
	m_bQuit = true;
//	(*loading_thread_)();
}

ResLoader* ResLoader::Instance()
{
	if (nullptr != m_InstanceResLoaderPtr)
	{
		return m_InstanceResLoaderPtr.get();
	}

	return nullptr;
}

void ResLoader::Destroy()
{
}

void ResLoader::Suspend()
{
	// TODO
}

void ResLoader::Resume()
{
	// TODO
}

std::string ResLoader::AbsPath(std::string_view path)
{
	std::string path_str(path);
	std::filesystem::path new_path(path_str);
	if (!new_path.is_absolute())
	{
		std::filesystem::path full_path = std::filesystem::path(m_strLocalPath) / new_path;
#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
		std::error_code ec;
		if (!std::filesystem::exists(full_path, ec))
#else
		if (!std::filesystem::exists(full_path))
#endif
		{
#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
			if (!std::filesystem::exists(full_path, ec))
#else
			if (!std::filesystem::exists(full_path))
#endif
			{
				return "";
			}
		}

		new_path = full_path;
	}

	std::string ret = new_path.string();
#if defined STX_PLATFORM_WIN
	std::replace(ret.begin(), ret.end(), '\\', '/');
#endif
	return ret;
}

std::string ResLoader::RealPath(std::string_view path)
{
	std::string package_path;
	std::string password;
	std::string path_in_package;
	return this->RealPath(path, package_path, password, path_in_package);
}

std::string ResLoader::RealPath(std::string_view path,
	std::string& package_path, std::string& password, std::string& path_in_package)
{
	package_path = "";
	password = "";
	path_in_package = "";

	std::string abs_path = this->AbsPath(path);
	if (abs_path.empty())
	{
		this->DecomposePackageName(path, package_path, password, path_in_package);
		if (!package_path.empty())
		{
			std::string real_package_path = this->RealPath(package_path);
			real_package_path.pop_back();

			package_path = real_package_path;

			abs_path = real_package_path;
			if (!password.empty())
			{
				abs_path += "|" + password;
			}
			if (!path_in_package.empty())
			{
				abs_path += "/" + path_in_package;
			}
			if (abs_path.back() != '/')
			{
				abs_path.push_back('/');
			}
		}
	}
	else
	{
		this->DecomposePackageName(abs_path, package_path, password, path_in_package);

		if (abs_path.back() != '/')
		{
			abs_path.push_back('/');
		}
	}

	return abs_path;
}

void ResLoader::DecomposePackageName(std::string_view path,
	std::string& package_path, std::string& password, std::string& path_in_package)
{
	package_path = "";
	password = "";
	path_in_package = "";

	size_t start_offset = 0;
	for (;;)
	{
		auto const pkt_offset = path.find(".7z", start_offset);
		if (pkt_offset != std::string_view::npos)
		{
			package_path = std::string(path.substr(0, pkt_offset + 3));
			std::filesystem::path pkt_path(package_path);
#if defined(STX__CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX__TS_LIBRARY_FILESYSTEM_SUPPORT)
			std::error_code ec;
			if (std::filesystem::exists(pkt_path, ec)
#else
			if (std::filesystem::exists(pkt_path)
#endif
				&& (std::filesystem::is_regular_file(pkt_path) || std::filesystem::is_symlink(pkt_path)))
			{
				auto const next_slash_offset = path.find('/', pkt_offset + 3);
				if ((path.size() > pkt_offset + 3) && (path[pkt_offset + 3] == '|'))
				{
					auto const password_start_offset = pkt_offset + 4;
					if (next_slash_offset != std::string_view::npos)
					{
						password = std::string(path.substr(password_start_offset, next_slash_offset - password_start_offset));
					}
					else
					{
						password = std::string(path.substr(password_start_offset));
					}
				}
				if (next_slash_offset != std::string_view::npos)
				{
					path_in_package = std::string(path.substr(next_slash_offset + 1));
				}
				break;
			}
			else
			{
				start_offset = pkt_offset + 3;
			}
		}
		else
		{
			break;
		}
	}
}

void ResLoader::AddPath(std::string_view phy_path)
{
	this->Mount("", phy_path);
}

void ResLoader::DelPath(std::string_view phy_path)
{
	this->Unmount("", phy_path);
}

const std::string& ResLoader::LocalFolder() const
{
	return m_strLocalPath;
}

void ResLoader::Mount(std::string_view virtual_path, std::string_view phy_path)
{
	std::lock_guard<std::mutex> lock(m_PathsMutex);

	std::string package_path;
	std::string password;
	std::string path_in_package;
	std::string real_path = this->RealPath(phy_path,
		package_path, password, path_in_package);
	if (!real_path.empty())
	{
		std::string virtual_path_str(virtual_path);
		if (!virtual_path.empty() && (virtual_path.back() != '/'))
		{
			virtual_path_str.push_back('/');
		}
		uint64_t const virtual_path_hash = HashRange(virtual_path_str.begin(), virtual_path_str.end());

		bool found = false;
		for (auto const & path : m_PathDataVec)
		{
			if ((std::get<0>(path) == virtual_path_hash) && (std::get<2>(path) == real_path))
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			PackagePtr package;
			if (!package_path.empty())
			{
				for (auto const & path : m_PathDataVec)
				{
					auto const & p = std::get<3>(path);
					if (p && package_path == p->ArchiveStream()->ResName())
					{
						package = p;
						break;
					}
				}
				if (!package)
				{
#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
					uint64_t timestamp = std::filesystem::last_write_time(package_path).time_since_epoch().count();
#else
					uint64_t timestamp = std::filesystem::last_write_time(package_path);
#endif
					// The static_cast is a workaround for a bug in clang/c2
					auto package_res = MakeSharedPtr<ResIdentifier>(package_path, timestamp,
						MakeSharedPtr<std::ifstream>(package_path.c_str(),
							static_cast<std::ios_base::openmode>(std::ios_base::binary)));

					package = MakeSharedPtr<Package>(package_res, password);
				}
			}

			m_PathDataVec.push_back(std::make_tuple(virtual_path_hash, static_cast<uint32_t>(virtual_path_str.size()), real_path, package));
		}
	}
}

void ResLoader::Unmount(std::string_view virtual_path, std::string_view phy_path)
{
	std::lock_guard<std::mutex> lock(m_PathsMutex);

	std::string real_path = this->RealPath(phy_path);
	if (!real_path.empty())
	{
		std::string virtual_path_str(virtual_path);
		if (!virtual_path.empty() && (virtual_path.back() != '/'))
		{
			virtual_path_str.push_back('/');
		}
		uint64_t const virtual_path_hash = HashRange(virtual_path_str.begin(), virtual_path_str.end());

		for (auto iter = m_PathDataVec.begin(); iter != m_PathDataVec.end(); ++iter)
		{
			if ((std::get<0>(*iter) == virtual_path_hash) && (std::get<2>(*iter) == real_path))
			{
				m_PathDataVec.erase(iter);
				break;
			}
		}
	}
}

std::string ResLoader::Locate(std::string_view name)
{
#if defined(STX_PLATFORM_ANDROID)
#elif defined(STX_PLATFORM_IOS)
#else
	{
		std::lock_guard<std::mutex> lock(m_PathsMutex);
		for (auto const & path : m_PathDataVec)
		{
			if ((std::get<1>(path) != 0) || (HashRange(name.begin(), name.begin() + std::get<1>(path)) == std::get<0>(path)))
			{
				std::string res_name(std::get<2>(path) + std::string(name.substr(std::get<1>(path))));
#if defined STX_PLATFORM_WIN
				std::replace(res_name.begin(), res_name.end(), '\\', '/');
#endif

#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
				std::error_code ec;
				if (std::filesystem::exists(std::filesystem::path(res_name), ec))
#else
				if (std::filesystem::exists(std::filesystem::path(res_name)))
#endif
				{
					return res_name;
				}
				else
				{
					std::string package_path;
					std::string password;
					std::string path_in_package;
					this->DecomposePackageName(res_name, package_path, password, path_in_package);
					auto const & package = std::get<3>(path);
					if (!package_path.empty() && package && (package_path == package->ArchiveStream()->ResName()))
					{
						if (package->Locate(path_in_package))
						{
							return res_name;
						}
					}
				}
			}

			if ((std::get<1>(path) == 0) && std::filesystem::path(name.begin(), name.end()).is_absolute())
			{
				break;
			}
		}
	}

#endif

	return "";
}

ResIdentifierPtr ResLoader::Open(std::string_view strName)
{
	{
		std::lock_guard<std::mutex> lock(m_PathsMutex);
		for (auto const & var : m_PathDataVec)
		{
			if ((std::get<1>(var) != 0) || (HashRange(strName.begin(), strName.begin() + std::get<1>(var)) == std::get<0>(var)))
			{
				std::string strNameRes(std::get<2>(var) + std::string(strName.substr(std::get<1>(var))));
#if defined STX_PLATFORM_WIN
				std::replace(strNameRes.begin(), strNameRes.end(), '\\', '/');
#endif

				std::filesystem::path strPathRes(strNameRes);
#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
				std::error_code ec;
				if (std::filesystem::exists(strPathRes, ec))
#else
				if (std::filesystem::exists(strPathRes))
#endif
				{
#if defined(STX_CXX17_LIBRARY_FILESYSTEM_SUPPORT) || defined(STX_TS_LIBRARY_FILESYSTEM_SUPPORT)
					uint64_t timestamp = std::filesystem::last_write_time(strPathRes).time_since_epoch().count();
#else
					uint64_t timestamp = std::filesystem::last_write_time(strPathRes);
#endif
					return MakeSharedPtr<ResIdentifier>(strName, timestamp,
						MakeSharedPtr<std::ifstream>(strNameRes.c_str(), std::ios_base::binary));
				}
				else
				{
					std::string package_path;
					std::string password;
					std::string path_in_package;
					this->DecomposePackageName(strNameRes, package_path, password, path_in_package);
					auto const & package = std::get<3>(var);
					if (!package_path.empty() && package && (package_path == package->ArchiveStream()->ResName()))
					{
						auto res = package->Extract(strName, strName);
						if (res)
						{
							return res;
						}
					}
				}
			}

			if ((std::get<1>(var) == 0) && std::filesystem::path(strName.begin(), strName.end()).is_absolute())
			{
				break;
			}
		}
	}

	return ResIdentifierPtr();
}

std::shared_ptr<void> ResLoader::SyncQuery(ResLoadingDescPtr const & res_desc)
{
	this->RemoveUnrefResources();
	std::shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
	std::shared_ptr<void> res;
	if (loaded_res)
	{
		if (res_desc->StateLess())
		{
			res = loaded_res;
		}
		else
		{
			res = res_desc->CloneResourceFrom(loaded_res);
			if (res != loaded_res)
			{
				this->AddLoadedResource(res_desc, res);
			}
		}
	}
	else
	{
		std::shared_ptr<volatile LoadingStatus> async_is_done;
		bool found = false;
		{
			std::lock_guard<std::mutex> lock(m_LoadingMutex);
			for (auto const & lrq : m_LoadingRes)
			{
				if (lrq.first->Match(*res_desc))
				{
					res_desc->CopyDataFrom(*lrq.first);
					res = lrq.first->Resource();
					async_is_done = lrq.second;
					found = true;
					break;
				}
			}
		}

		if (found)
		{
			*async_is_done = LS_Complete;
		}
		else
		{
			res = res_desc->CreateResource();
		}

		if (res_desc->HasSubThreadStage())
		{
			res_desc->SubThreadStage();
		}

		res_desc->MainThreadStage();
		res = res_desc->Resource();
		this->AddLoadedResource(res_desc, res);
	}

	return res;
}

std::shared_ptr<void> ResLoader::ASyncQuery(ResLoadingDescPtr const & res_desc)
{
	this->RemoveUnrefResources();

	std::shared_ptr<void> res;
	std::shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
	if (loaded_res)
	{
		if (res_desc->StateLess())
		{
			res = loaded_res;
		}
		else
		{
			res = res_desc->CloneResourceFrom(loaded_res);
			if (res != loaded_res)
			{
				this->AddLoadedResource(res_desc, res);
			}
		}
	}
	else
	{
		std::shared_ptr<volatile LoadingStatus> async_is_done;
		bool found = false;
		{
			std::lock_guard<std::mutex> lock(m_LoadingMutex);

			for (auto const & lrq : m_LoadingRes)
			{
				if (lrq.first->Match(*res_desc))
				{
					res_desc->CopyDataFrom(*lrq.first);
					res = lrq.first->Resource();
					async_is_done = lrq.second;
					found = true;
					break;
				}
			}
		}

		if (found)
		{
			if (!res_desc->StateLess())
			{
				std::lock_guard<std::mutex> lock(m_LoadingMutex);
				m_LoadingRes.emplace_back(res_desc, async_is_done);
			}
		}
		else
		{
			if (res_desc->HasSubThreadStage())
			{
				res = res_desc->CreateResource();

				async_is_done = MakeSharedPtr<LoadingStatus>(LS_Loading);

				{
					std::lock_guard<std::mutex> lock(m_LoadingMutex);
					m_LoadingRes.emplace_back(res_desc, async_is_done);
				}
				//loading_res_queue_.push(std::make_pair(res_desc, async_is_done));
			}
			else
			{
				res_desc->MainThreadStage();
				res = res_desc->Resource();
				this->AddLoadedResource(res_desc, res);
			}
		}
	}

	return res;
}

void ResLoader::Unload(std::shared_ptr<void> const & res)
{
	std::lock_guard<std::mutex> lock(m_LoadedMutex);

	for (auto iter = m_LoadedResVec.begin(); iter != m_LoadedResVec.end(); ++iter)
	{
		if (res == iter->second.lock())
		{
			m_LoadedResVec.erase(iter);
			break;
		}
	}
}

bool ResLoader::OnInit()
{
	return true;
}

bool ResLoader::OnShut()
{
	return true;
}

void ResLoader::Update()
{
	std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> tmp_loading_res;
	{
		std::lock_guard<std::mutex> lock(m_LoadingMutex);
		tmp_loading_res = m_LoadingRes;
	}

	for (auto& lrq : tmp_loading_res)
	{
		if (LS_Complete == *lrq.second)
		{
			ResLoadingDescPtr const & res_desc = lrq.first;

			std::shared_ptr<void> res;
			std::shared_ptr<void> loaded_res = this->FindMatchLoadedResource(res_desc);
			if (loaded_res)
			{
				if (!res_desc->StateLess())
				{
					res = res_desc->CloneResourceFrom(loaded_res);
					if (res != loaded_res)
					{
						this->AddLoadedResource(res_desc, res);
					}
				}
			}
			else
			{
				res_desc->MainThreadStage();
				res = res_desc->Resource();
				this->AddLoadedResource(res_desc, res);
			}

			*lrq.second = LS_CanBeRemoved;
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_LoadingMutex);
		for (auto iter = m_LoadingRes.begin(); iter != m_LoadingRes.end();)
		{
			if (LS_CanBeRemoved == *(iter->second))
			{
				iter = m_LoadingRes.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}

void ResLoader::LoadingThreadFunc()
{
	//while (!m_bQuit)
	//{
	//	std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>> res_pair;
	//	while (loading_res_queue_.pop(res_pair))
	//	{
	//		if (LS_Loading == *res_pair.second)
	//		{
	//			res_pair.first->SubThreadStage();
	//			*res_pair.second = LS_Complete;
	//		}
	//	}

	//	Sleep(10);
	//}
}

#if defined(STX_PLATFORM_ANDROID)
	AAsset* ResLoader::LocateFileAndroid(std::string_view name)
	{
		android_app* state = Context::Instance()->AppState();
		AAssetManager* am = state->activity->assetManager;
		return AAssetManager_open(am, std::string(name).c_str(), AASSET_MODE_UNKNOWN);
	}
#elif defined(STX_PLATFORM_IOS)
	std::string ResLoader::LocateFileIOS(std::string_view name)
	{
		std::string res_name;
		std::string::size_type found = name.find_last_of(".");
		if (found != std::string::npos)
		{
			std::string::size_type found2 = name.find_last_of("/");
			CFBundleRef main_bundle = CFBundleGetMainBundle();
			CFStringRef file_name = CFStringCreateWithCString(kCFAllocatorDefault,
				std::string(name.substr(found2 + 1, found - found2 - 1)).c_str(), kCFStringEncodingASCII);
			CFStringRef file_ext = CFStringCreateWithCString(kCFAllocatorDefault,
				std::string(name.substr(found + 1)).c_str(), kCFStringEncodingASCII);
			CFURLRef file_url = CFBundleCopyResourceURL(main_bundle, file_name, file_ext, NULL);
			CFRelease(file_name);
			CFRelease(file_ext);
			if (file_url != nullptr)
			{
				CFStringRef file_path = CFURLCopyFileSystemPath(file_url, kCFURLPOSIXPathStyle);

				res_name = CFStringGetCStringPtr(file_path, CFStringGetSystemEncoding());

				CFRelease(file_url);
				CFRelease(file_path);
			}
		}
		return res_name;
	}
#endif

void ResLoader::AddLoadedResource(ResLoadingDescPtr const & res_desc, std::shared_ptr<void> const & res)
{
	std::lock_guard<std::mutex> lock(m_LoadedMutex);

	bool found = false;
	for (auto& c_desc : m_LoadedResVec)
	{
		if (c_desc.first == res_desc)
		{
			c_desc.second = std::weak_ptr<void>(res);
			found = true;
			break;
		}
	}
	if (!found)
	{
		m_LoadedResVec.emplace_back(res_desc, std::weak_ptr<void>(res));
	}
}

std::shared_ptr<void> ResLoader::FindMatchLoadedResource(ResLoadingDescPtr const & res_desc)
{
	std::lock_guard<std::mutex> lock(m_LoadedMutex);
	std::shared_ptr<void> loaded_res;
	for (auto const & lr : m_LoadedResVec)
	{
		if (lr.first->Match(*res_desc))
		{
			loaded_res = lr.second.lock();
			break;
		}
	}
	return loaded_res;
}

void ResLoader::RemoveUnrefResources()
{
	std::lock_guard<std::mutex> lock(m_LoadedMutex);
	for (auto iter = m_LoadedResVec.begin(); iter != m_LoadedResVec.end();)
	{
		if (iter->second.lock())
		{
			++iter;
		}
		else
		{
			iter = m_LoadedResVec.erase(iter);
		}
	}
}
