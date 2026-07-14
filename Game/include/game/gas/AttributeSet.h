#pragma once

#include <game/GameApi.h>
#include <game/gas/GasConventions.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace Gas
{

enum class AttributeId : int32_t
{
	None = 0,
	MaxHP = 1,
	HP = 2, // current hit points (settable; clamped to MaxHP)
	ATK = 3,
	MP = 4,
	MaxMP = 5,
};

enum class ModifierOp : uint8_t
{
	Add = 0,        // flat
	PercentAdd = 1, // normalized ratio; aggregated as *(1 + sum)
};

struct AttributeModifier
{
	AttributeId attribute {AttributeId::None};
	ModifierOp op {ModifierOp::Add};
	float value {0.f};
	GasId source_effect_instance {kInvalidGasId};
};

// Base + Effect modifiers. Equipment layer reserved (empty) for later.
// current = (base + sumAdd) * (1 + sumPercentAdd)  for MaxHP/ATK/MP/MaxMP
// HP is a separate current value, clamped to [0, MaxHP].
class GAME_API AttributeSet
{
public:
	void SetBase(AttributeId id, float value);
	float GetBase(AttributeId id) const;

	void AddModifier(AttributeModifier const& mod);
	void RemoveModifiersFromSource(GasId effect_instance_id);
	void ClearModifiers();

	float GetCurrent(AttributeId id) const;

	void SetHP(float hp);
	void ApplyHPDelta(float delta);

	void Recalculate();

private:
	float ComputeMerged(AttributeId id) const;

	std::unordered_map<AttributeId, float> bases_;
	std::vector<AttributeModifier> modifiers_;
	float hp_ {0.f};
	mutable std::unordered_map<AttributeId, float> cache_;
	mutable bool dirty_ {true};
};

} // namespace Gas
