#pragma once

#include <game/GameApi.h>
#include <game/gas/AbilitySystemComponent.h>
#include <game/gas/CombatBeat.h>
#include <game/gas/EffectTypes.h>

namespace Gas
{

class SkillCatalog;

class GAME_API CombatService
{
public:
	static void SetSkillCatalog(SkillCatalog const* catalog);
	static SkillCatalog const* GetSkillCatalog();

	static float ApplyDamage(AbilitySystemComponent& source, AbilitySystemComponent& target, float amount);
	static void HandleCommand(CombatCommand& cmd);
};

// Builds a sample Atk PercentAdd buff for smoke / GM.
GAME_API EffectSpec MakeSampleAtkBuff(TimeMs duration_ms, float percent_add);

// Runs ASC + Effect + Damage checks; returns true if all assertions pass.
GAME_API bool RunGasSmokeTest();

// Loads skills/flows (or uses injected catalog) and runs BasicStrike via pipeline.
GAME_API bool RunGasSkillConfigSmokeTest();
// Builds a sample Atk PercentAdd buff for smoke / GM.
GAME_API EffectSpec MakeSampleAtkBuff(TimeMs duration_ms, float percent_add);

// Runs ASC + Effect + Damage checks; returns true if all assertions pass.
GAME_API bool RunGasSmokeTest();

// Loads skills/flows (or uses injected catalog) and runs BasicStrike via pipeline.
GAME_API bool RunGasSkillConfigSmokeTest();

} // namespace Gas
