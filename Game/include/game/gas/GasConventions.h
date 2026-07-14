#pragma once

#include <cstdint>

// Locked P0 data conventions — see openspec/plan/gas-data-conventions.md
// Do not introduce a second unit system for the same field.

namespace Gas
{

// ---------------------------------------------------------------------------
// Ratios / percents: normalized float in logic. 0.2f == 20%.
// Config column "foo_pct" (integer 20) must convert via PercentFromPctInt.
// ---------------------------------------------------------------------------
inline constexpr float kPercentUnit = 0.01f; // 1 pct point

[[nodiscard]] inline constexpr float PercentFromPctInt(int32_t pct_int) noexcept
{
	return static_cast<float>(pct_int) * kPercentUnit;
}

[[nodiscard]] inline constexpr bool LooksLikeWrongPercentMagnitude(float normalized_or_suspicious) noexcept
{
	// Heuristic for loaders: if a 0~1 field is >> 1, likely fed 20 instead of 0.2.
	return normalized_or_suspicious > 5.f;
}

// ---------------------------------------------------------------------------
// Time: milliseconds (int32). Seconds → ms: * 1000.
// Cooldown / Timeline / Effect duration all use TimeMs.
// ---------------------------------------------------------------------------
using TimeMs = int32_t;

inline constexpr TimeMs kTimeMsPerSecond = 1000;

[[nodiscard]] inline constexpr TimeMs SecondsToMs(float seconds) noexcept
{
	return static_cast<TimeMs>(seconds * static_cast<float>(kTimeMsPerSecond));
}

// ---------------------------------------------------------------------------
// Stable runtime IDs (int32). Config "name" is editor/debug only.
// ---------------------------------------------------------------------------
using GasId = int32_t;

inline constexpr GasId kInvalidGasId = 0;

// ---------------------------------------------------------------------------
// Closed enums (not free strings) for small sets.
// ---------------------------------------------------------------------------
enum class FactionId : int32_t
{
	None = 0,
	Player = 1,
	Enemy = 2,
	Neutral = 3,
};

enum class SkillSlot : int32_t
{
	None = -1,
	Slot0 = 0,
	Slot1 = 1,
	Slot2 = 2,
	Slot3 = 3,
	Passive = 100,
};

// Attribute PercentAdd uses normalized ratios (AttributeSet).
// Commit timing (P0): on successful RulePlan when commit_cost is true.

} // namespace Gas
