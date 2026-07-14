#pragma once

#include <Manager/DataManager.h>
#include <game/GameApi.h>
#include <UI/GmDebugWindow.h>
#include <game/gas/CombatBeat.h>
#include <game/gas/SkillCatalog.h>

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

	Gas::CombatBeat& CombatBeatInstance() noexcept;
	Gas::CombatBeat const& CombatBeatInstance() const noexcept;

	Gas::SkillCatalog& SkillCatalogInstance() noexcept;
	Gas::SkillCatalog const& SkillCatalogInstance() const noexcept;

	void TickCombat(float dt_seconds);

private:
	GameContext() = default;

	bool started_ { false };
	DataManager data_manager_;
	GmDebugWindow gm_debug_window_;
	Gas::CombatBeat combat_beat_;
	Gas::SkillCatalog skill_catalog_;
};
