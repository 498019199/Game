#include <game/GameContext.h>
#include <common/Log.h>

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
    try
    {
        if (!data_manager_.LoadGameConfig())
            CommonWorker::LogError() << "GameContext: failed to load presentation assets." << std::endl;
        started_ = true;
    }
    catch (...)
    {
        data_manager_.Clear();
        throw;
    }
}
void GameContext::Shutdown()
{
    if (!started_) return;
    gm_debug_window_.Shutdown();
    data_manager_.Clear();
    started_ = false;
}
DataManager& GameContext::DataManagerInstance() noexcept { return data_manager_; }
DataManager const& GameContext::DataManagerInstance() const noexcept { return data_manager_; }
GmDebugWindow& GameContext::GmDebugWindowInstance() noexcept { return gm_debug_window_; }
GmDebugWindow const& GameContext::GmDebugWindowInstance() const noexcept { return gm_debug_window_; }
