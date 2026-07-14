#pragma once

#include <game/GameApi.h>
#include <game/gas/SkillDef.h>

namespace Gas
{

class AbilitySystemComponent;
class SkillCatalog;

struct SkillLaunchContext
{
	AbilitySystemComponent* source {nullptr};
	AbilitySystemComponent* target {nullptr};
	int32_t rank {1};
};

// Active cast driven by flow phases (RulePlan → Timeline for P0).
class GAME_API SkillPipelineRunner
{
public:
	bool Start(SkillDef const& skill, SkillFlowDef const& flow, SkillLaunchContext ctx);
	void Cancel(ActivateFailReason reason = ActivateFailReason::None);
	void Step(TimeMs dt_ms);

	bool Running() const noexcept { return running_; }
	bool FinishedOk() const noexcept { return finished_ok_; }
	ActivateFailReason FailReason() const noexcept { return fail_reason_; }
	GasId SkillId() const noexcept { return skill_id_; }

private:
	bool EnterPhase(size_t index);
	bool ExecuteRulePlan(RulePlanPhaseDef const& plan);
	void StepTimeline(TimeMs dt_ms);
	void Finish(bool ok, ActivateFailReason reason);

	bool running_ {false};
	bool finished_ok_ {false};
	ActivateFailReason fail_reason_ {ActivateFailReason::None};
	GasId skill_id_ {kInvalidGasId};
	SkillLaunchContext ctx_ {};
	SkillDef skill_ {};
	SkillFlowDef flow_ {};
	size_t phase_index_ {0};
	TimeMs timeline_elapsed_ms_ {0};
	size_t next_event_index_ {0};
	bool in_timeline_ {false};
};

class GAME_API AbilityLauncher
{
public:
	// Starts a cast on source's pipeline runner. Returns false + fail reason on gate failure.
	static bool Launch(
		SkillCatalog const& catalog,
		SkillLaunchContext ctx,
		GasId skill_id,
		ActivateFailReason* out_fail = nullptr);
};

} // namespace Gas
