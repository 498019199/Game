
#include <base/ResourceLoad.h>

#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include <vector>

#if defined ZENGINE_PLATFORM_WINDOWS_DESKTOP
#include <windows.h>
#elif defined ZENGINE_PLATFORM_WINDOWS_STORE
#if defined(KLAYGE_COMPILER_MSVC) && (_MSC_VER >= 1929)
#pragma warning(push)
#pragma warning(disable : 5246) // Turn of warnings of _Elems
#endif

namespace RenderWorker
{
using namespace CommonWorker;

class ResourceLoad::Impl
{
public:
    void AddPath(std::string_view phy_path)
    {
        this->Mount("", std::move(phy_path));
    }

	void DelPath(std::string_view phy_path)
    {
        this->Unmount("", std::move(phy_path));
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
        return ResIdentifierPtr();
    }

    std::string Locate(std::string_view name);

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
#ifndef KLAYGE_PLATFORM_ANDROID
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

private:
    struct PathInfo
    {
        uint64_t virtual_path_hash;
        uint32_t virtual_path_size;
        std::filesystem::path real_path;
        PackagePtr package;
    };
    std::vector<PathInfo> paths_;
    std::mutex paths_mutex_;
};

ResourceLoad::ResourceLoad() noexcept = default;
ResourceLoad::~ResourceLoad() noexcept = default;

std::string ResourceLoad::AbsPath(std::string_view path)
{
    return pimpl_->AbsPath(std::move(path));
}

void ResourceLoad::AddPath(std::string_view phy_path)
{
    pimpl_->AddPath(std::move(phy_path));
}

void ResourceLoad::DelPath(std::string_view phy_path)
{
    pimpl_->DelPath(std::move(phy_path));
}

void ResourceLoad::Mount(std::string_view virtual_path, std::string_view phy_path)
{
    pimpl_->Mount(std::move(virtual_path), std::move(phy_path));
}

void ResourceLoad::Unmount(std::string_view virtual_path, std::string_view phy_path)
{
    pimpl_->Unmount(std::move(virtual_path), std::move(phy_path));
}

ResIdentifierPtr ResourceLoad::Open(std::string_view name)
{
    return pimpl_->Open(std::move(name));
}

std::string ResourceLoad::Locate(std::string_view name)
{
    return pimpl_->Locate(std::move(name));
}

uint64_t ResourceLoad::Timestamp(std::string_view name)
{
    return pimpl_->Timestamp(std::move(name));
}


}
