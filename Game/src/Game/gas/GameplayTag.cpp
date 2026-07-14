#include <game/gas/GameplayTag.h>

#include <algorithm>

namespace Gas
{
namespace
{
bool IsPrefixMatch(std::string const& owned, std::string_view query)
{
	if (owned == query)
	{
		return true;
	}
	if (owned.size() <= query.size())
	{
		return false;
	}
	if (owned.compare(0, query.size(), query) != 0)
	{
		return false;
	}
	return owned[query.size()] == '.';
}
} // namespace

void GameplayTagContainer::AddTag(std::string_view tag)
{
	if (tag.empty())
	{
		return;
	}
	std::string key(tag);
	if (tags_.insert(key).second)
	{
		tags_ordered_.push_back(std::move(key));
	}
}

void GameplayTagContainer::RemoveTag(std::string_view tag)
{
	auto it = tags_.find(std::string(tag));
	if (it == tags_.end())
	{
		return;
	}
	tags_.erase(it);
	tags_ordered_.erase(
		std::remove(tags_ordered_.begin(), tags_ordered_.end(), std::string(tag)),
		tags_ordered_.end());
}

void GameplayTagContainer::RemoveTagsWithPrefix(std::string_view prefix)
{
	for (auto it = tags_ordered_.begin(); it != tags_ordered_.end();)
	{
		if (IsPrefixMatch(*it, prefix) || *it == prefix)
		{
			tags_.erase(*it);
			it = tags_ordered_.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void GameplayTagContainer::Clear()
{
	tags_.clear();
	tags_ordered_.clear();
}

bool GameplayTagContainer::HasTagExact(std::string_view tag) const
{
	return tags_.find(std::string(tag)) != tags_.end();
}

bool GameplayTagContainer::HasTag(std::string_view tag) const
{
	if (HasTagExact(tag))
	{
		return true;
	}
	for (auto const& owned : tags_ordered_)
	{
		if (IsPrefixMatch(owned, tag))
		{
			return true;
		}
	}
	return false;
}

bool GameplayTagContainer::HasAny(std::vector<std::string_view> const& query) const
{
	for (auto t : query)
	{
		if (HasTag(t))
		{
			return true;
		}
	}
	return false;
}

bool GameplayTagContainer::HasAll(std::vector<std::string_view> const& query) const
{
	for (auto t : query)
	{
		if (!HasTag(t))
		{
			return false;
		}
	}
	return true;
}

} // namespace Gas
