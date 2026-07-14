#include <game/gas/EffectContainer.h>

#include <game/gas/AbilitySystemComponent.h>

namespace Gas
{

EffectContainer::EffectContainer(AbilitySystemComponent& owner)
	: owner_(owner)
{
}

GasId EffectContainer::Apply(EffectSpec const& spec)
{
	EffectInstance inst;
	inst.instance_id = next_instance_id_++;
	inst.effect_def_id = spec.effect_def_id;
	inst.duration_policy = spec.duration_policy;
	inst.period_ms = spec.period_ms;
	inst.granted_tags = spec.granted_tags;

	if (spec.duration_policy == EffectDurationPolicy::Duration)
	{
		inst.remaining_ms = spec.duration_ms;
	}
	else if (spec.duration_policy == EffectDurationPolicy::Infinite)
	{
		inst.remaining_ms = -1;
	}
	else
	{
		inst.remaining_ms = 0;
	}

	for (auto const& tag : inst.granted_tags)
	{
		owner_.Tags().AddTag(tag);
	}

	for (auto const& def : spec.modifiers)
	{
		AttributeModifier mod;
		mod.attribute = def.attribute;
		mod.op = def.op;
		mod.value = def.value;
		mod.source_effect_instance = inst.instance_id;
		owner_.Attributes().AddModifier(mod);
		inst.applied_modifiers.push_back(mod);
	}

	GasEvent applied;
	applied.type = GasEventType::EffectApplied;
	applied.effect_def_id = inst.effect_def_id;
	applied.effect_instance_id = inst.instance_id;
	applied.target = &owner_;
	owner_.Broadcast(applied);

	if (spec.duration_policy == EffectDurationPolicy::Instant)
	{
		// Instant: apply then immediately remove (reason Expired as completed).
		active_.push_back(std::move(inst));
		RemoveInternal(active_.size() - 1, EffectRemoveReason::Expired);
		return kInvalidGasId;
	}

	active_.push_back(std::move(inst));
	return active_.back().instance_id;
}

void EffectContainer::Step(TimeMs dt_ms)
{
	if (dt_ms <= 0)
	{
		return;
	}

	for (auto& inst : active_)
	{
		if (inst.pending_remove)
		{
			continue;
		}

		if (inst.period_ms > 0)
		{
			inst.period_elapsed_ms += dt_ms;
			while (inst.period_elapsed_ms >= inst.period_ms)
			{
				inst.period_elapsed_ms -= inst.period_ms;
				GasEvent tick;
				tick.type = GasEventType::EffectTicked;
				tick.effect_def_id = inst.effect_def_id;
				tick.effect_instance_id = inst.instance_id;
				tick.target = &owner_;
				owner_.Broadcast(tick);
			}
		}

		if (inst.duration_policy == EffectDurationPolicy::Duration)
		{
			inst.remaining_ms -= dt_ms;
			if (inst.remaining_ms <= 0)
			{
				inst.pending_remove = true;
				inst.pending_reason = EffectRemoveReason::Expired;
			}
		}
	}
}

bool EffectContainer::Remove(GasId instance_id, EffectRemoveReason reason)
{
	for (size_t i = 0; i < active_.size(); ++i)
	{
		if (active_[i].instance_id == instance_id)
		{
			RemoveInternal(i, reason);
			return true;
		}
	}
	return false;
}

void EffectContainer::FlushPendingRemoves()
{
	for (size_t i = 0; i < active_.size();)
	{
		if (active_[i].pending_remove)
		{
			RemoveInternal(i, active_[i].pending_reason);
		}
		else
		{
			++i;
		}
	}
}

EffectInstance const* EffectContainer::Find(GasId instance_id) const
{
	for (auto const& inst : active_)
	{
		if (inst.instance_id == instance_id)
		{
			return &inst;
		}
	}
	return nullptr;
}

void EffectContainer::RemoveInternal(size_t index, EffectRemoveReason reason)
{
	if (index >= active_.size())
	{
		return;
	}

	EffectInstance inst = std::move(active_[index]);
	active_.erase(active_.begin() + static_cast<std::ptrdiff_t>(index));

	owner_.Attributes().RemoveModifiersFromSource(inst.instance_id);
	for (auto const& tag : inst.granted_tags)
	{
		owner_.Tags().RemoveTag(tag);
	}

	GasEvent removed;
	removed.type = GasEventType::EffectRemoved;
	removed.effect_def_id = inst.effect_def_id;
	removed.effect_instance_id = inst.instance_id;
	removed.remove_reason = reason;
	removed.target = &owner_;
	owner_.Broadcast(removed);
}

} // namespace Gas
