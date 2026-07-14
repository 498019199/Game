#include <game/GameContext.h>

#include <base/UIManager.h>
#include <base/ZEngine.h>
#include <common/Log.h>
#include <game/gas/CombatService.h>
#include <game/gas/GasConventions.h>

GameContext& GameContext::Instance()
{
	static GameContext instance;
	return instance;
}

void GameContext::Startup()
{
	if (started_)
	{
		return;
	}

	if (!data_manager_.LoadNpcConfig())
	{
		CommonWorker::LogError() << "GameContext: failed to load NPC config." << std::endl;
	}

	skill_catalog_.Clear();
	if (!skill_catalog_.LoadSkillsJson("Config/skills.json")
		|| !skill_catalog_.LoadFlowsJson("Config/skill_flows.json"))
	{
		CommonWorker::LogError() << "GameContext: failed to load skill configs." << std::endl;
	}
	else
	{
		Gas::CombatService::SetSkillCatalog(&skill_catalog_);
	}

	started_ = true;
}

void GameContext::Shutdown()
{
	if (!started_)
	{
		return;
	}

	Gas::CombatService::SetSkillCatalog(nullptr);
	skill_catalog_.Clear();
	gm_debug_window_.Shutdown();
	data_manager_.Clear();
	started_ = false;
}

DataManager& GameContext::DataManagerInstance() noexcept
{
	return data_manager_;
}

DataManager const& GameContext::DataManagerInstance() const noexcept
{
	return data_manager_;
}

GmDebugWindow& GameContext::GmDebugWindowInstance() noexcept
{
	return gm_debug_window_;
}

GmDebugWindow const& GameContext::GmDebugWindowInstance() const noexcept
{
	return gm_debug_window_;
}

Gas::CombatBeat& GameContext::CombatBeatInstance() noexcept
{
	return combat_beat_;
}

Gas::CombatBeat const& GameContext::CombatBeatInstance() const noexcept
{
	return combat_beat_;
}

Gas::SkillCatalog& GameContext::SkillCatalogInstance() noexcept
{
	return skill_catalog_;
}

Gas::SkillCatalog const& GameContext::SkillCatalogInstance() const noexcept
{
	return skill_catalog_;
}

void GameContext::TickCombat(float dt_seconds)
{
	if (!started_ || dt_seconds <= 0.f)
	{
		return;
	}
	Gas::TimeMs const dt_ms = static_cast<Gas::TimeMs>(dt_seconds * 1000.f);
	if (dt_ms > 0)
	{
		combat_beat_.Step(dt_ms);
	}
}
