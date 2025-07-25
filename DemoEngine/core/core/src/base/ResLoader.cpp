/**
 * @file ResLoader.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */
#include <common/common.h>
#include <common/Util.h>
#include <base/Package.h>
#include <base/Thread.h>

#if defined ZENGINE_PLATFORM_LINUX
#include <cstring>
#endif
#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include <vector>

#if defined ZENGINE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#elif defined ZENGINE_PLATFORM_WINDOWS_STORE
#if defined(ZENGINE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(push)
#pragma warning(disable : 5246) // Turn of warnings of _Elems
#endif
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Storage.h>
#if defined(ZENGINE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(pop)
#endif

namespace uwp
{
	using winrt::hstring;

	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::ApplicationModel;
	using namespace winrt::Windows::Storage;
}

#include <KFL/ErrorHandling.hpp>
#elif defined ZENGINE_PLATFORM_LINUX
#elif defined ZENGINE_PLATFORM_ANDROID
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <KFL/CustomizedStreamBuf.hpp>
#elif defined ZENGINE_PLATFORM_DARWIN
#include <mach-o/dyld.h>
#elif defined ZENGINE_PLATFORM_IOS
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <base/ResLoader.h>

#if defined(ZENGINE_PLATFORM_ANDROID)
struct AAsset;
#endif

namespace
{
#ifdef ZENGINE_PLATFORM_ANDROID
	class AAssetStreamBuf : public KlayGE::MemInputStreamBuf
	{
	public:
		explicit AAssetStreamBuf(AAsset* asset)
			: MemInputStreamBuf(AAsset_getBuffer(asset), AAsset_getLength(asset)),
				asset_(asset)
		{
			BOOST_ASSERT(asset_ != nullptr);
		}

		~AAssetStreamBuf()
		{
			AAsset_close(asset_);
		}

	private:
		AAsset* asset_;
	};
#endif
}

namespace RenderWorker
{
    using namespace CommonWorker;
    
	class ResLoader::Impl final
	{
	public:
		explicit Impl(ThreadPool& tp)
		{
#if defined ZENGINE_PLATFORM_WINDOWS
#if defined ZENGINE_PLATFORM_WINDOWS_DESKTOP
			char buf[MAX_PATH];
			::GetModuleFileNameA(nullptr, buf, sizeof(buf));
			exe_path_ = std::filesystem::path(buf).parent_path().generic_string();
			local_path_ = exe_path_ + "/";
#else
			auto package = uwp::Package::Current();
			auto installed_loc_storage_item = package.InstalledLocation().as<uwp::IStorageItem>();
			auto const installed_loc_folder_name = installed_loc_storage_item.Path();
			Convert(exe_path_, installed_loc_folder_name);

			auto app_data = uwp::ApplicationData::Current();
			auto local_folder_storage_item = app_data.LocalFolder().as<uwp::IStorageItem>();
			auto const local_folder_name = local_folder_storage_item.Path();
			Convert(local_path_, local_folder_name);
			local_path_ += "\\";
#endif
#elif defined ZENGINE_PLATFORM_LINUX
			{
				FILE* fp = fopen("/proc/self/maps", "r");
				if (fp != nullptr)
				{
					while (!feof(fp))
					{
						char line[1024];
						unsigned long start, end;
						if (!fgets(line, sizeof(line), fp))
						{
							continue;
						}
						if (!strstr(line, " r-xp ") || !strchr(line, '/'))
						{
							continue;
						}

						void const* symbol = "";
						sscanf(line, "%lx-%lx ", &start, &end);
						if ((symbol >= reinterpret_cast<void const*>(start)) && (symbol < reinterpret_cast<void const*>(end)))
						{
							exe_path_ = strchr(line, '/');
							exe_path_ = exe_path_.substr(0, exe_path_.rfind("/"));
						}
					}
					fclose(fp);
				}

#ifdef ZENGINE_PLATFORM_ANDROID
				exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("/"));
				exe_path_ = exe_path_.substr(exe_path_.find_last_of("/") + 1);
				exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("-"));
				exe_path_ = "/data/data/" + exe_path_;
#endif

				local_path_ = exe_path_;
			}
#elif defined ZENGINE_PLATFORM_DARWIN
			uint32_t size = 0;
			_NSGetExecutablePath(nullptr, &size);
			auto buffer = MakeUniquePtr<char[]>(size + 1);
			_NSGetExecutablePath(buffer.get(), &size);
			buffer[size] = '\0';
			exe_path_ = buffer.get();
			exe_path_ = exe_path_.substr(0, exe_path_.find_last_of("/") + 1);
			local_path_ = exe_path_;
#endif

			paths_.emplace_back(PathInfo{CtHash(""), 0U, std::filesystem::path(), PackagePtr()});

			loading_thread_ = tp.QueueThread([this] { this->LoadingThreadFunc(); });
		}
		~Impl()
		{
			quit_ = true;

			{
				std::unique_lock<std::mutex> lock(loading_res_queue_mutex_, std::try_to_lock);
				non_empty_loading_res_queue_ = true;
				loading_res_queue_cv_.notify_one();
			}

			loading_thread_.wait();
		}

		void Suspend()
		{
			// TODO
		}
		void Resume()
		{
			// TODO
		}

		void AddPath(std::string_view phy_path)
		{
			this->Mount("", std::move(phy_path));
		}
		void DelPath(std::string_view phy_path)
		{
			this->Unmount("", std::move(phy_path));
		}
		bool IsInPath(std::string_view phy_path)
		{
			std::string_view virtual_path = "";

			std::lock_guard<std::mutex> lock(paths_mutex_);

			std::filesystem::path const real_path = this->RealPath(std::move(phy_path));
			if (!real_path.empty())
			{
				std::string virtual_path_str(virtual_path);
				if (!virtual_path.empty() && (virtual_path.back() != '/'))
				{
					virtual_path_str.push_back('/');
				}
				uint64_t const virtual_path_hash = HashValue(virtual_path_str);

				bool found = false;
				for (auto const& path : paths_)
				{
					if ((path.virtual_path_hash == virtual_path_hash) && (path.real_path == real_path))
					{
						found = true;
						break;
					}
				}

				return found;
			}
			else
			{
				return false;
			}
		}
		std::string const& LocalFolder() const noexcept
		{
			return local_path_;
		}

		void Mount(std::string_view virtual_path, std::string_view phy_path)
		{
			std::lock_guard<std::mutex> lock(paths_mutex_);

			std::string package_path;
			std::string password;
			std::string path_in_package;
			std::filesystem::path real_path = this->RealPath(std::move(phy_path), package_path, password, path_in_package);
			if (!real_path.empty())
			{
				std::string virtual_path_str(virtual_path);
				if (!virtual_path.empty() && (virtual_path.back() != '/'))
				{
					virtual_path_str.push_back('/');
				}
				uint64_t const virtual_path_hash = HashValue(virtual_path_str);

				bool found = false;
				for (auto const& path : paths_)
				{
					if ((path.virtual_path_hash == virtual_path_hash) && (path.real_path == real_path))
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
						for (auto const& path : paths_)
						{
							auto const& p = path.package;
							if (p && package_path == p->ArchiveStream()->ResName())
							{
								package = p;
								break;
							}
						}
						if (!package)
						{
							uint64_t const timestamp = std::filesystem::last_write_time(package_path).time_since_epoch().count();
							auto package_res = MakeSharedPtr<ResIdentifier>(
								package_path, timestamp, MakeSharedPtr<std::ifstream>(package_path.c_str(), std::ios_base::binary));

							package = MakeSharedPtr<Package>(package_res, password);
						}
					}

					paths_.emplace_back(PathInfo{
						virtual_path_hash, static_cast<uint32_t>(virtual_path_str.size()), std::move(real_path), std::move(package)});
				}
			}
		}
		void Unmount(std::string_view virtual_path, std::string_view phy_path)
		{
			std::lock_guard<std::mutex> lock(paths_mutex_);

			std::filesystem::path const real_path = this->RealPath(std::move(phy_path));
			if (!real_path.empty())
			{
				std::string virtual_path_str(std::move(virtual_path));
				if (!virtual_path.empty() && (virtual_path.back() != '/'))
				{
					virtual_path_str.push_back('/');
				}
				uint64_t const virtual_path_hash = HashValue(virtual_path_str);

				for (auto iter = paths_.begin(); iter != paths_.end(); ++iter)
				{
					if ((iter->virtual_path_hash == virtual_path_hash) && (iter->real_path == real_path))
					{
						paths_.erase(iter);
						break;
					}
				}
			}
		}

		ResIdentifierPtr Open(std::string_view name)
		{
			if (name.empty())
			{
				return ResIdentifierPtr();
			}

#if defined(ZENGINE_PLATFORM_ANDROID)
			AAsset* asset = this->LocateFileAndroid(name);
			if (asset != nullptr)
			{
				std::shared_ptr<AAssetStreamBuf> asb = MakeSharedPtr<AAssetStreamBuf>(asset);
				std::shared_ptr<std::istream> asset_file = MakeSharedPtr<std::istream>(asb.get());
				return MakeSharedPtr<ResIdentifier>(name, 0, asset_file, asb);
			}
#elif defined(ZENGINE_PLATFORM_IOS)
			std::string const& res_name = this->LocateFileIOS(name);
			if (!res_name.empty())
			{
				FILESYSTEM_NS::path res_path(res_name);
				uint64_t const timestamp = FILESYSTEM_NS::last_write_time(res_path).time_since_epoch().count();
				return MakeSharedPtr<ResIdentifier>(name, timestamp,
					MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
			}
#else
			{
				std::lock_guard<std::mutex> lock(paths_mutex_);
				for (auto const& path : paths_)
				{
					if ((path.virtual_path_size != 0) ||
						(HashRange(name.begin(), name.begin() + path.virtual_path_size) == path.virtual_path_hash))
					{
						std::filesystem::path const res_path = path.real_path / name.substr(path.virtual_path_size);
						std::string const res_name = res_path.generic_string();
						if (std::filesystem::exists(res_path))
						{
							uint64_t const timestamp = std::filesystem::last_write_time(res_path).time_since_epoch().count();
							return MakeSharedPtr<ResIdentifier>(
								name, timestamp, MakeSharedPtr<std::ifstream>(res_name.c_str(), std::ios_base::binary));
						}
						else
						{
							std::string package_path;
							std::string password;
							std::string path_in_package;
							this->DecomposePackageName(res_name, package_path, password, path_in_package);
							auto const& package = path.package;
							if (!package_path.empty() && package && (package_path == package->ArchiveStream()->ResName()))
							{
								auto res = package->Extract(path_in_package, name);
								if (res)
								{
									return res;
								}
							}
						}
					}

					if ((path.virtual_path_size == 0) && std::filesystem::path(name).is_absolute())
					{
						break;
					}
				}
			}
#if defined(ZENGINE_PLATFORM_WINDOWS_STORE)
			std::string const& res_name = this->LocateFileWinRT(name);
			if (!res_name.empty())
			{
				return this->Open(res_name);
			}
#endif
#endif

			return ResIdentifierPtr();
		}
		std::string Locate(std::string_view name)
		{
			if (name.empty())
			{
				return "";
			}

#if defined(ZENGINE_PLATFORM_ANDROID)
			AAsset* asset = this->LocateFileAndroid(name);
			if (asset != nullptr)
			{
				AAsset_close(asset);
				return std::string(name);
			}
#elif defined(ZENGINE_PLATFORM_IOS)
			return this->LocateFileIOS(name);
#else
			{
				std::lock_guard<std::mutex> lock(paths_mutex_);
				for (auto const& path : paths_)
				{
					if ((path.virtual_path_size != 0) || (HashRange(name.begin(), name.begin() + path.virtual_path_size) == path.virtual_path_hash))
					{
						std::filesystem::path const res_path = path.real_path / name.substr(path.virtual_path_size);
						std::string const res_name = res_path.generic_string();
						if (std::filesystem::exists(res_path))
						{
							return res_name;
						}
						else
						{
							std::string package_path;
							std::string password;
							std::string path_in_package;
							this->DecomposePackageName(res_name, package_path, password, path_in_package);
							auto const& package = path.package;
							if (!package_path.empty() && package && (package_path == package->ArchiveStream()->ResName()))
							{
								if (package->Locate(path_in_package))
								{
									return res_name;
								}
							}
						}
					}

					if ((path.virtual_path_size == 0) && std::filesystem::path(name).is_absolute())
					{
						break;
					}
				}
			}
#if defined ZENGINE_PLATFORM_WINDOWS_STORE
			std::string const& res_name = this->LocateFileWinRT(name);
			if (!res_name.empty())
			{
				return this->Locate(res_name);
			}
#endif
#endif

			return "";
		}
		uint64_t Timestamp(std::string_view name)
		{
			uint64_t timestamp = 0;
			auto res_path = this->Locate(std::move(name));
			if (!res_path.empty())
			{
#if !defined(ZENGINE_PLATFORM_ANDROID)
				timestamp = std::filesystem::last_write_time(res_path).time_since_epoch().count();
#endif
			}

			return timestamp;
		}
		std::string AbsPath(std::string_view path)
		{
			std::filesystem::path new_path(std::move(path));
			if (!new_path.is_absolute())
			{
				std::filesystem::path full_path = std::filesystem::path(exe_path_) / new_path;
				if (!std::filesystem::exists(full_path))
				{
#ifndef ZENGINE_PLATFORM_ANDROID
					try
					{
						full_path = std::filesystem::current_path() / new_path;
					}
					catch (...)
					{
						full_path = new_path;
					}
					if (!std::filesystem::exists(full_path))
					{
						return "";
					}
#else
					return "";
#endif
				}
				new_path = full_path;
			}
			return new_path.generic_string();
		}

		std::shared_ptr<void> SyncQuery(ResLoadingDescPtr const& res_desc)
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
					std::lock_guard<std::mutex> lock(loading_mutex_);

					for (auto const& lrq : loading_res_)
					{
						if (lrq.first->Match(*res_desc))
						{
							res_desc->CopyDataFrom(*lrq.first);
							async_is_done = lrq.second;
							found = true;
							break;
						}
					}
				}

				if (found)
				{
					*async_is_done = LoadingStatus::Complete;
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
		std::shared_ptr<void> ASyncQuery(ResLoadingDescPtr const& res_desc)
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
					std::lock_guard<std::mutex> lock(loading_mutex_);

					for (auto const& lrq : loading_res_)
					{
						if (lrq.first->Match(*res_desc))
						{
							res_desc->CopyDataFrom(*lrq.first);
							async_is_done = lrq.second;
							found = true;
							break;
						}
					}
				}

				if (found)
				{
					res = res_desc->Resource();

					if (!res_desc->StateLess())
					{
						std::lock_guard<std::mutex> lock(loading_mutex_);
						loading_res_.emplace_back(res_desc, async_is_done);
					}
				}
				else
				{
					if (res_desc->HasSubThreadStage())
					{
						res = res_desc->CreateResource();

						async_is_done = MakeSharedPtr<LoadingStatus>(LoadingStatus::Loading);

						{
							std::lock_guard<std::mutex> lock(loading_mutex_);
							loading_res_.emplace_back(res_desc, async_is_done);
						}
						{
							std::unique_lock<std::mutex> lock(loading_res_queue_mutex_, std::try_to_lock);
							loading_res_queue_.emplace_back(res_desc, async_is_done);
							non_empty_loading_res_queue_ = true;
							loading_res_queue_cv_.notify_one();
						}
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
		void Unload(std::shared_ptr<void> const& res)
		{
			std::lock_guard<std::mutex> lock(loaded_mutex_);

			for (auto iter = loaded_res_.begin(); iter != loaded_res_.end(); ++iter)
			{
				if (res == iter->second.lock())
				{
					loaded_res_.erase(iter);
					break;
				}
			}
		}

		void Update()
		{
			std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> tmp_loading_res;
			{
				std::lock_guard<std::mutex> lock(loading_mutex_);
				tmp_loading_res = loading_res_;
			}

			for (auto& lrq : tmp_loading_res)
			{
				if (LoadingStatus::Complete == *lrq.second)
				{
					ResLoadingDescPtr const& res_desc = lrq.first;

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
				}
			}
			for (auto& lrq : tmp_loading_res)
			{
				if (LoadingStatus::Complete == *lrq.second)
				{
					*lrq.second = LoadingStatus::CanBeRemoved;
				}
			}

			{
				std::lock_guard<std::mutex> lock(loading_mutex_);
				for (auto iter = loading_res_.begin(); iter != loading_res_.end();)
				{
					if (LoadingStatus::CanBeRemoved == *(iter->second))
					{
						iter = loading_res_.erase(iter);
					}
					else
					{
						++iter;
					}
				}
			}
		}

		uint32_t NumLoadingResources() const noexcept
		{
			return static_cast<uint32_t>(loading_res_.size());
		}

	private:
		std::filesystem::path RealPath(std::string_view path)
		{
			std::string package_path;
			std::string password;
			std::string path_in_package;
			return this->RealPath(std::move(path), package_path, password, path_in_package);
		}
		std::filesystem::path RealPath(std::string_view path,
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
					std::string real_package_path = this->RealPath(package_path).string();
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
		void DecomposePackageName(std::string_view path,
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
					std::filesystem::path pkt_path(this->AbsPath(package_path));
					if (std::filesystem::exists(pkt_path) &&
						(std::filesystem::is_regular_file(pkt_path) || std::filesystem::is_symlink(pkt_path)))
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

		void AddLoadedResource(ResLoadingDescPtr const& res_desc, std::shared_ptr<void> const& res)
		{
			std::lock_guard<std::mutex> lock(loaded_mutex_);

			bool found = false;
			for (auto& c_desc : loaded_res_)
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
				loaded_res_.emplace_back(res_desc, std::weak_ptr<void>(res));
			}
		}
		std::shared_ptr<void> FindMatchLoadedResource(ResLoadingDescPtr const& res_desc)
		{
			std::lock_guard<std::mutex> lock(loaded_mutex_);

			std::shared_ptr<void> loaded_res;
			for (auto const& lr : loaded_res_)
			{
				if (lr.first->Match(*res_desc))
				{
					loaded_res = lr.second.lock();
					break;
				}
			}
			return loaded_res;
		}
		void RemoveUnrefResources()
		{
			std::lock_guard<std::mutex> lock(loaded_mutex_);

			for (auto iter = loaded_res_.begin(); iter != loaded_res_.end();)
			{
				if (iter->second.lock())
				{
					++iter;
				}
				else
				{
					iter = loaded_res_.erase(iter);
				}
			}
		}

		void LoadingThreadFunc()
		{
			while (!quit_)
			{
				std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> loading_res_queue_copy;

				{
					std::unique_lock<std::mutex> lock(loading_res_queue_mutex_);
					loading_res_queue_cv_.wait(lock, [this] { return non_empty_loading_res_queue_; });

					loading_res_queue_copy.swap(loading_res_queue_);
					non_empty_loading_res_queue_ = false;
				}

				for (auto& res_pair : loading_res_queue_copy)
				{
					if (LoadingStatus::Loading == *res_pair.second)
					{
						res_pair.first->SubThreadStage();
						*res_pair.second = LoadingStatus::Complete;
					}
				}

				Sleep(10);
			}
		}

#if defined(ZENGINE_PLATFORM_ANDROID)
		AAsset* LocateFileAndroid(std::string_view name)
		{
			android_app* state = Context::Instance().AppState();
			AAssetManager* am = state->activity->assetManager;
			return AAssetManager_open(am, std::string(std::move(name)).c_str(), AASSET_MODE_UNKNOWN);
		}
#elif defined(ZENGINE_PLATFORM_IOS)
		std::string LocateFileIOS(std::string_view name)
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
#elif defined(ZENGINE_PLATFORM_WINDOWS_STORE)
		std::string LocateFileWinRT(std::string_view name)
		{
			std::string res_name;
			std::string::size_type pos = name.rfind('/');
			if (std::string::npos == pos)
			{
				pos = name.rfind('\\');
			}
			if (pos != std::string::npos)
			{
				res_name = name.substr(pos + 1);
			}
			return res_name;
		}
#endif

	private:
		enum class LoadingStatus
		{
			Loading,
			Complete,
			CanBeRemoved
		};

		std::string exe_path_;
		std::string local_path_;

		struct PathInfo
		{
			uint64_t virtual_path_hash;
			uint32_t virtual_path_size;
			std::filesystem::path real_path;
			PackagePtr package;
		};
		std::vector<PathInfo> paths_;
		std::mutex paths_mutex_;

		std::mutex loaded_mutex_;
		std::mutex loading_mutex_;
		std::vector<std::pair<ResLoadingDescPtr, std::weak_ptr<void>>> loaded_res_;
		std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> loading_res_;

		bool non_empty_loading_res_queue_ = false;
		std::condition_variable loading_res_queue_cv_;
		std::mutex loading_res_queue_mutex_;
		std::vector<std::pair<ResLoadingDescPtr, std::shared_ptr<volatile LoadingStatus>>> loading_res_queue_;

		std::future<void> loading_thread_;
		volatile bool quit_{false};
	};

	ResLoadingDesc::ResLoadingDesc() noexcept = default;
	ResLoadingDesc::~ResLoadingDesc() noexcept = default;

	ResLoader::ResLoader() noexcept = default;
	ResLoader::~ResLoader() noexcept = default;

	void ResLoader::Init(ThreadPool& tp)
	{
		pimpl_ = MakeUniquePtr<Impl>(tp);
	}

	void ResLoader::Destroy() noexcept
	{
		pimpl_.reset();
	}

	bool ResLoader::Valid() const noexcept
	{
		return static_cast<bool>(pimpl_);
	}

	void ResLoader::Suspend()
	{
		pimpl_->Suspend();
	}

	void ResLoader::Resume()
	{
		pimpl_->Resume();
	}

	std::string ResLoader::AbsPath(std::string_view path)
	{
		return pimpl_->AbsPath(std::move(path));
	}

	void ResLoader::AddPath(std::string_view phy_path)
	{
		pimpl_->AddPath(std::move(phy_path));
	}

	void ResLoader::DelPath(std::string_view phy_path)
	{
		pimpl_->DelPath(std::move(phy_path));
	}

	bool ResLoader::IsInPath(std::string_view phy_path)
	{
		return pimpl_->IsInPath(std::move(phy_path));
	}

	std::string const& ResLoader::LocalFolder() const noexcept
	{
		return pimpl_->LocalFolder();
	}

	void ResLoader::Mount(std::string_view virtual_path, std::string_view phy_path)
	{
		pimpl_->Mount(std::move(virtual_path), std::move(phy_path));
	}

	void ResLoader::Unmount(std::string_view virtual_path, std::string_view phy_path)
	{
		pimpl_->Unmount(std::move(virtual_path), std::move(phy_path));
	}

	std::string ResLoader::Locate(std::string_view name)
	{
		return pimpl_->Locate(std::move(name));
	}

	ResIdentifierPtr ResLoader::Open(std::string_view name)
	{
		return pimpl_->Open(std::move(name));
	}

	uint64_t ResLoader::Timestamp(std::string_view name)
	{
		return pimpl_->Timestamp(std::move(name));
	}

	std::shared_ptr<void> ResLoader::SyncQuery(ResLoadingDescPtr const & res_desc)
	{
		return pimpl_->SyncQuery(res_desc);
	}

	std::shared_ptr<void> ResLoader::ASyncQuery(ResLoadingDescPtr const & res_desc)
	{
		return pimpl_->ASyncQuery(res_desc);
	}

	void ResLoader::Unload(std::shared_ptr<void> const & res)
	{
		pimpl_->Unload(res);
	}

	void ResLoader::Update()
	{
		pimpl_->Update();
	}

	uint32_t ResLoader::NumLoadingResources() const noexcept
	{
		return pimpl_->NumLoadingResources();
	}
}
