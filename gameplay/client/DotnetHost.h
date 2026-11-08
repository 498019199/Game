#pragma once
#include <cstdint>
#include <filesystem>

#ifdef _WIN32
#define GAMEPLAY_CALL __cdecl
#else
#define GAMEPLAY_CALL
#endif
namespace Gameplay
{
struct RuntimeApi
{
    using Create = std::int64_t(GAMEPLAY_CALL*)(const std::uint8_t*, std::int32_t);
    using Exchange = std::int32_t(GAMEPLAY_CALL*)(std::int64_t, const std::uint8_t*, std::int32_t, std::uint8_t**, std::int32_t*);
    using Release = void(GAMEPLAY_CALL*)(std::uint8_t*);
    using Destroy = std::int32_t(GAMEPLAY_CALL*)(std::int64_t);
    Create create{}; Exchange exchange{}; Release release{}; Destroy destroy{};
};
RuntimeApi LoadGameRuntime(const std::filesystem::path& managed_directory,
                          const std::filesystem::path& dotnet_root);
}
