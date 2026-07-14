#pragma once

#include <game/GameApi.h>
#include <game/gas/GasConventions.h>

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace Gas
{

// Hierarchical tags: "State.Stunned". HasTag("State") matches "State" or "State.*".
class GAME_API GameplayTagContainer
{
public:
	void AddTag(std::string_view tag);
	void RemoveTag(std::string_view tag);
	void RemoveTagsWithPrefix(std::string_view prefix);
	void Clear();

	bool HasTag(std::string_view tag) const;
	bool HasTagExact(std::string_view tag) const;
	bool HasAny(std::vector<std::string_view> const& tags) const;
	bool HasAll(std::vector<std::string_view> const& tags) const;

	std::vector<std::string> const& AllTags() const noexcept { return tags_ordered_; }

private:
	std::unordered_set<std::string> tags_;
	std::vector<std::string> tags_ordered_;
};

// Well-known tags used by P0 framework / samples.
namespace Tags
{
inline constexpr char const* StateStunned = "State.Stunned";
inline constexpr char const* StateRooted = "State.Rooted";
inline constexpr char const* StateSilenced = "State.Silenced";
inline constexpr char const* AbilityActive = "Ability.Active";
} // namespace Tags

} // namespace Gas
