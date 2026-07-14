#include <game/gas/AbilitySystemComponent.h>

#include <game/gas/GameplayTag.h>

namespace Gas
{

AbilitySystemComponent::AbilitySystemComponent()
	: effects_(*this)
{
}

AbilitySystemComponent::~AbilitySystemComponent() = default;

bool AbilitySystemComponent::CanMove() const
{
	return !tags_.HasTag(Tags::StateStunned) && !tags_.HasTag(Tags::StateRooted);
}

bool AbilitySystemComponent::CanCast() const
{
	return !tags_.HasTag(Tags::StateStunned) && !tags_.HasTag(Tags::StateSilenced);
}

GasId AbilitySystemComponent::ApplyEffect(EffectSpec const& spec)
{
	return effects_.Apply(spec);
}

void AbilitySystemComponent::StepEffects(TimeMs dt_ms)
{
	effects_.Step(dt_ms);
}

void AbilitySystemComponent::StepPipeline(TimeMs dt_ms)
{
	pipeline_.Step(dt_ms);
}

void AbilitySystemComponent::FlushEffects()
{
	effects_.FlushPendingRemoves();
}

void AbilitySystemComponent::SpendMp(float amount)
{
	if (amount <= 0.f)
	{
		return;
	}
	float const mp = attributes_.GetCurrent(AttributeId::MP);
	attributes_.SetBase(AttributeId::MP, mp - amount);
	attributes_.Recalculate();
}

bool AbilitySystemComponent::HasMp(float amount) const
{
	return attributes_.GetCurrent(AttributeId::MP) + 0.001f >= amount;
}

void AbilitySystemComponent::AddEventHandler(GasEventHandler handler)
{
	handlers_.push_back(std::move(handler));
}

void AbilitySystemComponent::Broadcast(GasEvent const& evt)
{
	for (auto const& h : handlers_)
	{
		if (h)
		{
			h(evt);
		}
	}
}

void AbilitySystemComponent::InitDefaultCombatStats(float max_hp, float atk, float max_mp)
{
	attributes_.SetBase(AttributeId::MaxHP, max_hp);
	attributes_.SetBase(AttributeId::HP, max_hp);
	attributes_.SetBase(AttributeId::ATK, atk);
	attributes_.SetBase(AttributeId::MaxMP, max_mp);
	attributes_.SetBase(AttributeId::MP, max_mp);
	attributes_.Recalculate();
}

} // namespace Gas
