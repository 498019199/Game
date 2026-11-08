#include "GameSession.h"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace Gameplay
{
GameSession::GameSession(const std::filesystem::path& directory, const std::filesystem::path& root)
    : api_(LoadGameRuntime(directory, root))
{
    std::ifstream file(directory / "game.catalog.pb", std::ios::binary);
    if (!file) throw std::runtime_error("Missing game.catalog.pb; rebuild managed gameplay assets");
    std::string bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (bytes.empty() || bytes.size() > MaxFrameBytes) throw std::runtime_error("Invalid catalog size");
    handle_ = api_.create(reinterpret_cast<const std::uint8_t*>(bytes.data()), static_cast<std::int32_t>(bytes.size()));
    if (!handle_) throw std::runtime_error("C# rejected the gameplay catalog");
}
GameSession::~GameSession() { if (handle_) api_.destroy(handle_); }
std::string GameSession::Exchange(const std::string& request)
{
    if (request.empty() || request.size() > MaxFrameBytes) throw std::invalid_argument("Invalid protobuf request size");
    std::uint8_t* response = nullptr;
    std::int32_t length = 0;
    int code = api_.exchange(handle_, reinterpret_cast<const std::uint8_t*>(request.data()),
        static_cast<std::int32_t>(request.size()), &response, &length);
    // Release even if constructing the std::string throws.
    std::unique_ptr<std::uint8_t, RuntimeApi::Release> owned(response, api_.release);
    if (code != 0 || !response || length <= 0 || static_cast<std::size_t>(length) > MaxFrameBytes)
        throw std::runtime_error("C# protobuf exchange failed: " + std::to_string(code));
    return std::string(reinterpret_cast<const char*>(response), static_cast<std::size_t>(length));
}
Protocol::Response GameSession::Call(Protocol::Request request)
{
    request.set_version(1);
    request.set_request_id(next_request_++);
    std::string bytes = Exchange(request.SerializeAsString());
    Protocol::Response response;
    if (!response.ParseFromString(bytes) || response.version() != 1 || response.request_id() != request.request_id())
        throw std::runtime_error("Invalid protobuf response/version/request ID");
    return response;
}
std::unique_ptr<GameSession> GameSession::CreateDefault()
{
    const char* root = std::getenv("DOTNET_ROOT");
    const char* managed = std::getenv("GAMEPLAY_MANAGED_DIR");
    return std::make_unique<GameSession>(managed && *managed ? managed : GAMEPLAY_MANAGED_DIR,
                                         root && *root ? root : GAMEPLAY_DOTNET_ROOT);
}
}
