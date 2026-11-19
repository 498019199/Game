#pragma once

#include <manager/DataManager.h>
#include <game/GameApi.h>
#include <ui/GmDebugWindow.h>

#ifdef ZENGINE_ENABLE_MANAGED_GAMEPLAY
#include <memory>

namespace Gameplay { class GameSession; }
#endif

class GAME_API GameContext
{
public:
    static GameContext& Instance();
    GameContext(GameContext const&) = delete;
    GameContext& operator=(GameContext const&) = delete;
    void Startup();
    void Shutdown();
    bool Started() const noexcept { return started_; }
    DataManager& DataManagerInstance() noexcept;
    DataManager const& DataManagerInstance() const noexcept;
    GmDebugWindow& GmDebugWindowInstance() noexcept;
    GmDebugWindow const& GmDebugWindowInstance() const noexcept;
#ifdef ZENGINE_ENABLE_MANAGED_GAMEPLAY
    Gameplay::GameSession& GameplayInstance();
    void TickCombat(float dt_seconds);
#endif
private:
    GameContext();
    ~GameContext();
    bool started_ = false;
#ifdef ZENGINE_ENABLE_MANAGED_GAMEPLAY
    double pending_ms_ = 0;
    std::unique_ptr<Gameplay::GameSession> gameplay_;
#endif
    DataManager data_manager_;
    GmDebugWindow gm_debug_window_;
};
