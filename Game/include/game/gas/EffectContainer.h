#pragma once

#include <game/GameApi.h>
#include <game/gas/EffectTypes.h>

#include <vector>

namespace Gas
{

class AbilitySystemComponent;

class GAME_API EffectContainer
{
public:
	explicit EffectContainer(AbilitySystemComponent& owner);

	GasId Apply(EffectSpec const& spec);
	void Step(TimeMs dt_ms);
	bool Remove(GasId instance_id, EffectRemoveReason reason);
	void FlushPendingRemoves();

	EffectInstance const* Find(GasId instance_id) const;
	std::vector<EffectInstance> const& Active() const noexcept { return active_; }

private:
	void RemoveInternal(size_t index, EffectRemoveReason reason);

	AbilitySystemComponent& owner_;
	std::vector<EffectInstance> active_;
	GasId next_instance_id_ {1};
};

} // namespace Gas
