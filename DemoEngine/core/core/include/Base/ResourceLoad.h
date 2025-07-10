#pragma once
#include <memory>
#include <string_view>
#include <common/common.h>

namespace RenderWorker
{
class ResourceLoad final
{
public:
    ResourceLoad() noexcept;
    ~ResourceLoad() noexcept;

    void AddPath(std::string_view phy_path);
	void DelPath(std::string_view phy_path);

    void Mount(std::string_view virtual_path, std::string_view phy_path);
    void Unmount(std::string_view virtual_path, std::string_view phy_path);

    ResIdentifierPtr Open(std::string_view name);
    std::string Locate(std::string_view name);
    uint64_t Timestamp(std::string_view name);
    std::string AbsPath(std::string_view path);
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

}