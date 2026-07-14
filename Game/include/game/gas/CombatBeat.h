#pragma once

#include <game/GameApi.h>
#include <game/gas/EffectTypes.h>
#include <game/gas/GasConventions.h>

#include <cstdint>
#include <deque>
#include <functional>
#include <vector>

namespace Gas
{

class AbilitySystemComponent;

enum class CombatCommandType : uint8_t
{
	None = 0,
	CastSkill = 1, // payload for later Pipeline; P0 may map to direct damage
	ApplyDamage = 2,
	ApplyEffect = 3,
};

struct CombatCommand
{
	CombatCommandType type {CombatCommandType::None};
	AbilitySystemComponent* source {nullptr};
	AbilitySystemComponent* target {nullptr};
	GasId skill_id {kInvalidGasId};
	GasId effect_def_id {kInvalidGasId};
	float damage {0.f};
	EffectSpec effect_spec {};
};

// Thin combat beat: ① time → ② effects → ③ commands → ④ execute → ⑤ cleanup.
class GAME_API CombatBeat
{
public:
	void Register(AbilitySystemComponent* asc);
	void Unregister(AbilitySystemComponent* asc);

	void Enqueue(CombatCommand cmd);
	void Step(TimeMs dt_ms);

	using ExecuteHook = std::function<void(CombatCommand&)>;
	void SetExecuteHook(ExecuteHook hook) { execute_hook_ = std::move(hook); }

	uint64_t BeatIndex() const noexcept { return beat_index_; }
	TimeMs AccumulatedTimeMs() const noexcept { return accumulated_ms_; }

private:
	void PhaseTimeAdvanced(TimeMs dt_ms);
	void PhaseContinuous(TimeMs dt_ms);
	void PhaseSelect();
	void PhaseExecute();
	void PhaseCleanup();

	std::vector<AbilitySystemComponent*> ascs_;
	std::deque<CombatCommand> incoming_;
	std::vector<CombatCommand> selected_;
	ExecuteHook execute_hook_;
	uint64_t beat_index_ {0};
	TimeMs accumulated_ms_ {0};
};

} // namespace Gas
