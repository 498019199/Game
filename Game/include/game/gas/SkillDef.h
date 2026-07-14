#pragma once

#include <game/GameApi.h>
#include <game/gas/GasConventions.h>

#include <cstdint>
#include <string>
#include <vector>

namespace Gas
{

enum class FlowPhaseType : uint8_t
{
	RulePlan = 0,
	Timeline = 1,
	WaitUntil = 2, // reserved; P0 loader rejects or skips
};

enum class TimelineAction : uint8_t
{
	Cue = 0,
	EmitHit = 1,
	End = 2,
};

struct SkillDef
{
	GasId id {kInvalidGasId};
	std::string name; // debug / editor only
	SkillSlot slot {SkillSlot::Slot0};
	GasId cast_flow_id {kInvalidGasId};
	float cost_mp {0.f};
	TimeMs cooldown_ms {0};
	std::vector<std::string> blocked_by_tags;
};

struct RulePlanPhaseDef
{
	bool commit_cost {true};
};

struct TimelineEventDef
{
	TimeMs t_ms {0};
	TimelineAction action {TimelineAction::Cue};
	std::string cue;
	float damage_scale {1.f}; // normalized multiplier on ATK
};

struct TimelinePhaseDef
{
	std::vector<TimelineEventDef> events;
};

struct FlowPhaseDef
{
	FlowPhaseType type {FlowPhaseType::RulePlan};
	RulePlanPhaseDef rule_plan;
	TimelinePhaseDef timeline;
};

struct SkillFlowDef
{
	GasId id {kInvalidGasId};
	std::string name;
	std::vector<FlowPhaseDef> phases;
};

enum class ActivateFailReason : uint8_t
{
	None = 0,
	UnknownSkill = 1,
	UnknownFlow = 2,
	BlockedByTag = 3,
	CannotCast = 4,
	InsufficientMp = 5,
	Busy = 6,
	InvalidTarget = 7,
};

} // namespace Gas
