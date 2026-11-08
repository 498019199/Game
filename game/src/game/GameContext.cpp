#include <game/GameContext.h>
#include <GameSession.h>
#include <common/Log.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

GameContext::GameContext() = default;
GameContext::~GameContext() = default;
GameContext& GameContext::Instance()
{
    static GameContext instance;
    return instance;
}
void GameContext::Startup()
{
    if (started_) return;
    gameplay_ = Gameplay::GameSession::CreateDefault();
    try
    {
        if (!data_manager_.LoadGameConfig())
            CommonWorker::LogError() << "GameContext: failed to load presentation assets." << std::endl;
        pending_ms_ = 0;
        started_ = true;
        CommonWorker::LogInfo() << "[C#] gameplay session started (Protocol Buffers v1)." << std::endl;
    }
    catch (...) { data_manager_.Clear(); gameplay_.reset(); throw; }
}
void GameContext::Shutdown()
{
    if (!started_) return;
    gm_debug_window_.Shutdown();
    data_manager_.Clear();
    gameplay_.reset();
    pending_ms_ = 0;
    started_ = false;
}
DataManager& GameContext::DataManagerInstance() noexcept { return data_manager_; }
DataManager const& GameContext::DataManagerInstance() const noexcept { return data_manager_; }
GmDebugWindow& GameContext::GmDebugWindowInstance() noexcept { return gm_debug_window_; }
GmDebugWindow const& GameContext::GmDebugWindowInstance() const noexcept { return gm_debug_window_; }
Gameplay::GameSession& GameContext::GameplayInstance()
{
    if (!gameplay_) throw std::runtime_error("Gameplay session has not started");
    return *gameplay_;
}
void GameContext::TickCombat(float dt_seconds)
{
    if (!started_ || !std::isfinite(dt_seconds) || dt_seconds <= 0) return;
    // Retain fractional milliseconds; rendering passes must not advance twice.
    pending_ms_ += std::min(static_cast<double>(dt_seconds) * 1000.0, 60000.0);
    const int elapsed = static_cast<int>(pending_ms_);
    if (elapsed == 0) return;
    pending_ms_ -= elapsed;
    Gameplay::Protocol::Request request;
    request.mutable_tick()->set_delta_ms(elapsed);
    const auto response = gameplay_->Call(std::move(request));
    if (response.status() != Gameplay::Protocol::ERROR_CODE_OK)
        throw std::runtime_error("C# tick failed: " + response.error());
}
