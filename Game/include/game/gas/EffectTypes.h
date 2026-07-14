#pragma once

#include <game/gas/AttributeSet.h>
#include <game/gas/GasConventions.h>

#include <functional>
#include <string>
#include <vector>

namespace Gas
{

enum class EffectDurationPolicy : uint8_t
{
	Instant = 0,  // apply tags/mods momentarily then remove (or one-shot actions only)
	Duration = 1, // timed
	Infinite = 2,
};

enum class EffectRemoveReason : uint8_t
{
	None = 0,
	Expired = 1,
	Manual = 2,
	Dispelled = 3,
	Death = 4,
	Replaced = 5,
};

struct EffectModifierDef
{
	AttributeId attribute {AttributeId::None};
	ModifierOp op {ModifierOp::Add};
	float value {0.f};
};

// Definition / apply request (data-driven later; P0 can construct in code).
struct EffectSpec
{
	GasId effect_def_id {kInvalidGasId};
	EffectDurationPolicy duration_policy {EffectDurationPolicy::Duration};
	TimeMs duration_ms {0}; // ignored if Infinite; Instant uses 0
	TimeMs period_ms {0};   // 0 = no periodic tick
	std::vector<std::string> granted_tags;
	std::vector<EffectModifierDef> modifiers;
};

struct EffectInstance
{
	GasId instance_id {kInvalidGasId};
	GasId effect_def_id {kInvalidGasId};
	EffectDurationPolicy duration_policy {EffectDurationPolicy::Duration};
	TimeMs remaining_ms {0};
	TimeMs period_ms {0};
	TimeMs period_elapsed_ms {0};
	std::vector<std::string> granted_tags;
	std::vector<AttributeModifier> applied_modifiers;
	bool pending_remove {false};
	EffectRemoveReason pending_reason {EffectRemoveReason::None};
};

enum class GasEventType : uint8_t
{
	EffectApplied = 0,
	EffectTicked = 1,
	EffectRemoved = 2,
	DamageApplied = 3,
};

struct GasEvent
{
	GasEventType type {GasEventType::EffectApplied};
	GasId effect_def_id {kInvalidGasId};
	GasId effect_instance_id {kInvalidGasId};
	EffectRemoveReason remove_reason {EffectRemoveReason::None};
	float damage {0.f};
	class AbilitySystemComponent* source {nullptr};
	class AbilitySystemComponent* target {nullptr};
};

using GasEventHandler = std::function<void(GasEvent const&)>;

} // namespace Gas
