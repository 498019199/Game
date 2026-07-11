#pragma once

#include <game/DataManager.h>
#include <game/GameApi.h>
#include <game/GmDebugWindow.h>

class GAME_API GameContext
{
public:
	static GameContext& Instance();

	GameContext(GameContext const&) = delete;
	GameContext& operator=(GameContext const&) = delete;

	// Create / initialize all game managers. Safe to call multiple times.
	void Startup();
	// Tear down managers. Safe to call multiple times.
	void Shutdown();

	bool Started() const noexcept { return started_; }

	DataManager& DataManagerInstance() noexcept;
	DataManager const& DataManagerInstance() const noexcept;

	GmDebugWindow& GmDebugWindowInstance() noexcept;
	GmDebugWindow const& GmDebugWindowInstance() const noexcept;

private:
	GameContext() = default;

	bool started_ { false };
	DataManager data_manager_;
	GmDebugWindow gm_debug_window_;
};
