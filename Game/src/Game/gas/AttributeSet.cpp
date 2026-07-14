#include <game/gas/AttributeSet.h>

#include <algorithm>
#include <cmath>

namespace Gas
{

void AttributeSet::SetBase(AttributeId id, float value)
{
	bases_[id] = value;
	if (id == AttributeId::HP)
	{
		hp_ = value;
	}
	dirty_ = true;
}

float AttributeSet::GetBase(AttributeId id) const
{
	auto it = bases_.find(id);
	return it == bases_.end() ? 0.f : it->second;
}

void AttributeSet::AddModifier(AttributeModifier const& mod)
{
	if (mod.attribute == AttributeId::None || mod.attribute == AttributeId::HP)
	{
		// HP current is not modifier-driven in P0.
		return;
	}
	modifiers_.push_back(mod);
	dirty_ = true;
	Recalculate();
}

void AttributeSet::RemoveModifiersFromSource(GasId effect_instance_id)
{
	auto const before = modifiers_.size();
	modifiers_.erase(
		std::remove_if(
			modifiers_.begin(),
			modifiers_.end(),
			[effect_instance_id](AttributeModifier const& m)
			{
				return m.source_effect_instance == effect_instance_id;
			}),
		modifiers_.end());
	if (modifiers_.size() != before)
	{
		dirty_ = true;
		Recalculate();
	}
}

void AttributeSet::ClearModifiers()
{
	modifiers_.clear();
	dirty_ = true;
	Recalculate();
}

float AttributeSet::ComputeMerged(AttributeId id) const
{
	float const base = GetBase(id);
	float add = 0.f;
	float percent_add = 0.f;
	for (auto const& m : modifiers_)
	{
		if (m.attribute != id)
		{
			continue;
		}
		if (m.op == ModifierOp::Add)
		{
			add += m.value;
		}
		else
		{
			percent_add += m.value;
		}
	}
	return (base + add) * (1.f + percent_add);
}

void AttributeSet::Recalculate()
{
	cache_.clear();
	for (auto const& [id, base] : bases_)
	{
		(void)base;
		if (id == AttributeId::HP)
		{
			continue;
		}
		cache_[id] = ComputeMerged(id);
	}
	float const max_hp = GetCurrent(AttributeId::MaxHP);
	hp_ = std::clamp(hp_, 0.f, max_hp > 0.f ? max_hp : hp_);
	dirty_ = false;
}

float AttributeSet::GetCurrent(AttributeId id) const
{
	if (id == AttributeId::HP)
	{
		return hp_;
	}
	if (dirty_)
	{
		const_cast<AttributeSet*>(this)->Recalculate();
	}
	auto it = cache_.find(id);
	if (it != cache_.end())
	{
		return it->second;
	}
	return ComputeMerged(id);
}

void AttributeSet::SetHP(float hp)
{
	hp_ = hp;
	float const max_hp = GetCurrent(AttributeId::MaxHP);
	if (max_hp > 0.f)
	{
		hp_ = std::clamp(hp_, 0.f, max_hp);
	}
	bases_[AttributeId::HP] = hp_;
}

void AttributeSet::ApplyHPDelta(float delta)
{
	SetHP(hp_ + delta);
}

} // namespace Gas
