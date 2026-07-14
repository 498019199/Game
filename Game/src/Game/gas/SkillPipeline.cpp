#include <game/gas/SkillPipeline.h>

#include <game/gas/AbilitySystemComponent.h>
#include <game/gas/CombatService.h>
#include <game/gas/SkillCatalog.h>

#include <common/Log.h>

#include <string>

namespace Gas
{

bool SkillPipelineRunner::Start(SkillDef const& skill, SkillFlowDef const& flow, SkillLaunchContext ctx)
{
	if (!ctx.source || !ctx.target)
	{
		fail_reason_ = ActivateFailReason::InvalidTarget;
		return false;
	}
	if (running_)
	{
		fail_reason_ = ActivateFailReason::Busy;
		return false;
	}

	skill_ = skill;
	flow_ = flow;
	ctx_ = ctx;
	skill_id_ = skill.id;
	phase_index_ = 0;
	timeline_elapsed_ms_ = 0;
	next_event_index_ = 0;
	in_timeline_ = false;
	running_ = true;
	finished_ok_ = false;
	fail_reason_ = ActivateFailReason::None;

	if (!EnterPhase(0))
	{
		return false;
	}
	return running_ || finished_ok_;
}

void SkillPipelineRunner::Cancel(ActivateFailReason reason)
{
	if (!running_)
	{
		return;
	}
	if (ctx_.source)
	{
		ctx_.source->Tags().RemoveTag(Tags::AbilityActive);
	}
	Finish(false, reason == ActivateFailReason::None ? ActivateFailReason::Busy : reason);
}

void SkillPipelineRunner::Step(TimeMs dt_ms)
{
	if (!running_)
	{
		return;
	}
	if (in_timeline_)
	{
		StepTimeline(dt_ms);
	}
}

bool SkillPipelineRunner::EnterPhase(size_t index)
{
	phase_index_ = index;
	in_timeline_ = false;
	if (phase_index_ >= flow_.phases.size())
	{
		Finish(true, ActivateFailReason::None);
		return true;
	}

	FlowPhaseDef const& phase = flow_.phases[phase_index_];
	switch (phase.type)
	{
	case FlowPhaseType::RulePlan:
		if (!ExecuteRulePlan(phase.rule_plan))
		{
			return false;
		}
		return EnterPhase(phase_index_ + 1);
	case FlowPhaseType::Timeline:
		in_timeline_ = true;
		timeline_elapsed_ms_ = 0;
		next_event_index_ = 0;
		if (ctx_.source)
		{
			ctx_.source->Tags().AddTag(Tags::AbilityActive);
		}
		// Process any t_ms == 0 events immediately.
		StepTimeline(0);
		return true;
	default:
		return EnterPhase(phase_index_ + 1);
	}
}

bool SkillPipelineRunner::ExecuteRulePlan(RulePlanPhaseDef const& plan)
{
	AbilitySystemComponent* src = ctx_.source;
	if (!src)
	{
		Finish(false, ActivateFailReason::InvalidTarget);
		return false;
	}
	if (!src->CanCast())
	{
		Finish(false, ActivateFailReason::CannotCast);
		return false;
	}
	for (auto const& tag : skill_.blocked_by_tags)
	{
		if (src->HasTag(tag))
		{
			Finish(false, ActivateFailReason::BlockedByTag);
			return false;
		}
	}
	if (plan.commit_cost)
	{
		if (!src->HasMp(skill_.cost_mp))
		{
			Finish(false, ActivateFailReason::InsufficientMp);
			return false;
		}
		src->SpendMp(skill_.cost_mp);
		// CD placeholder: grant a cooldown tag for debugging when cooldown_ms > 0.
		if (skill_.cooldown_ms > 0)
		{
			EffectSpec cd;
			cd.effect_def_id = skill_.id;
			cd.duration_policy = EffectDurationPolicy::Duration;
			cd.duration_ms = skill_.cooldown_ms;
			cd.granted_tags.push_back("Cooldown.Skill." + std::to_string(skill_.id));
			src->ApplyEffect(cd);
		}
	}
	return true;
}

void SkillPipelineRunner::StepTimeline(TimeMs dt_ms)
{
	if (!in_timeline_ || phase_index_ >= flow_.phases.size())
	{
		return;
	}

	timeline_elapsed_ms_ += dt_ms;
	TimelinePhaseDef const& timeline = flow_.phases[phase_index_].timeline;

	while (next_event_index_ < timeline.events.size())
	{
		TimelineEventDef const& ev = timeline.events[next_event_index_];
		if (ev.t_ms > timeline_elapsed_ms_)
		{
			break;
		}

		switch (ev.action)
		{
		case TimelineAction::Cue:
			LogInfo() << "[GAS] Cue skill=" << skill_id_ << " " << ev.cue << std::endl;
			break;
		case TimelineAction::EmitHit:
			if (ctx_.source && ctx_.target)
			{
				float const dmg = ctx_.source->GetAttribute(AttributeId::ATK) * ev.damage_scale;
				CombatService::ApplyDamage(*ctx_.source, *ctx_.target, dmg);
			}
			break;
		case TimelineAction::End:
			if (ctx_.source)
			{
				ctx_.source->Tags().RemoveTag(Tags::AbilityActive);
			}
			++next_event_index_;
			in_timeline_ = false;
			EnterPhase(phase_index_ + 1);
			return;
		}
		++next_event_index_;
	}

	if (next_event_index_ >= timeline.events.size())
	{
		if (ctx_.source)
		{
			ctx_.source->Tags().RemoveTag(Tags::AbilityActive);
		}
		in_timeline_ = false;
		EnterPhase(phase_index_ + 1);
	}
}

void SkillPipelineRunner::Finish(bool ok, ActivateFailReason reason)
{
	running_ = false;
	finished_ok_ = ok;
	fail_reason_ = reason;
	in_timeline_ = false;
}

bool AbilityLauncher::Launch(
	SkillCatalog const& catalog,
	SkillLaunchContext ctx,
	GasId skill_id,
	ActivateFailReason* out_fail)
{
	auto set_fail = [&](ActivateFailReason r)
	{
		if (out_fail)
		{
			*out_fail = r;
		}
		return false;
	};

	if (!ctx.source || !ctx.target)
	{
		return set_fail(ActivateFailReason::InvalidTarget);
	}
	if (ctx.source->IsCasting())
	{
		return set_fail(ActivateFailReason::Busy);
	}

	SkillDef const* skill = catalog.FindSkill(skill_id);
	if (!skill)
	{
		return set_fail(ActivateFailReason::UnknownSkill);
	}
	SkillFlowDef const* flow = catalog.FindFlow(skill->cast_flow_id);
	if (!flow)
	{
		return set_fail(ActivateFailReason::UnknownFlow);
	}

	bool const ok = ctx.source->Pipeline().Start(*skill, *flow, ctx);
	if (!ok)
	{
		return set_fail(ctx.source->Pipeline().FailReason());
	}
	if (out_fail)
	{
		*out_fail = ActivateFailReason::None;
	}
	return true;
}

} // namespace Gas
