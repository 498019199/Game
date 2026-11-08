#pragma once

#include <manager/DataManager.h>
#include <game/GameApi.h>
#include <ui/GmDebugWindow.h>
#include <memory>

namespace Gameplay { class GameSession; }

// Native engine/UI adapter. All gameplay state and rules live in the C# session.
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
    Gameplay::GameSession& GameplayInstance();
    void TickCombat(float dt_seconds);
private:
    GameContext();
    ~GameContext();
    bool started_ = false;
    double pending_ms_ = 0;
    std::unique_ptr<Gameplay::GameSession> gameplay_;
    DataManager data_manager_;
    GmDebugWindow gm_debug_window_;
};
