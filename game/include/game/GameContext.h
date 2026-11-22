#pragma once

#include <manager/DataManager.h>
#include <game/GameApi.h>
#include <ui/GmDebugWindow.h>

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
private:
    GameContext();
    ~GameContext();
    bool started_ = false;
    DataManager data_manager_;
    GmDebugWindow gm_debug_window_;
};
