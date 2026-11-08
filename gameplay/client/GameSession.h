#pragma once
#include "DotnetHost.h"
#include "gameplay.pb.h"
#include <memory>
#include <string>

namespace Gameplay
{
namespace Protocol = zengine::gameplay::v1;
inline constexpr std::size_t MaxFrameBytes = 4 * 1024 * 1024;
// Owns one managed world. Requests on one instance must be sent by one thread.
class GameSession
{
public:
    GameSession(const std::filesystem::path& managed_directory, const std::filesystem::path& dotnet_root);
    ~GameSession();
    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;
    Protocol::Response Call(Protocol::Request request);
    std::string Exchange(const std::string& protobuf_request);
    static std::unique_ptr<GameSession> CreateDefault();
private:
    RuntimeApi api_;
    std::int64_t handle_{};
    std::uint64_t next_request_{1};
};
}
