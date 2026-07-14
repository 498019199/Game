#include <game/gas/CombatBeat.h>

#include <game/gas/AbilitySystemComponent.h>
#include <game/gas/CombatService.h>

#include <algorithm>

namespace Gas
{

void CombatBeat::Register(AbilitySystemComponent* asc)
{
	if (!asc)
	{
		return;
	}
	if (std::find(ascs_.begin(), ascs_.end(), asc) == ascs_.end())
	{
		ascs_.push_back(asc);
	}
}

void CombatBeat::Unregister(AbilitySystemComponent* asc)
{
	ascs_.erase(std::remove(ascs_.begin(), ascs_.end(), asc), ascs_.end());
}

void CombatBeat::Enqueue(CombatCommand cmd)
{
	incoming_.push_back(std::move(cmd));
}

void CombatBeat::Step(TimeMs dt_ms)
{
	PhaseTimeAdvanced(dt_ms);
	PhaseContinuous(dt_ms);
	PhaseSelect();
	PhaseExecute();
	PhaseCleanup();
	++beat_index_;
}

void CombatBeat::PhaseTimeAdvanced(TimeMs dt_ms)
{
	if (dt_ms > 0)
	{
		accumulated_ms_ += dt_ms;
	}
}

void CombatBeat::PhaseContinuous(TimeMs dt_ms)
{
	for (auto* asc : ascs_)
	{
		if (asc)
		{
			asc->StepEffects(dt_ms);
			asc->StepPipeline(dt_ms);
		}
	}
}

void CombatBeat::PhaseSelect()
{
	selected_.clear();
	while (!incoming_.empty())
	{
		selected_.push_back(std::move(incoming_.front()));
		incoming_.pop_front();
	}
}

void CombatBeat::PhaseExecute()
{
	for (auto& cmd : selected_)
	{
		if (execute_hook_)
		{
			execute_hook_(cmd);
		}
		else
		{
			CombatService::HandleCommand(cmd);
		}
	}
	selected_.clear();
}

void CombatBeat::PhaseCleanup()
{
	for (auto* asc : ascs_)
	{
		if (asc)
		{
			asc->FlushEffects();
		}
	}
}

} // namespace Gas
