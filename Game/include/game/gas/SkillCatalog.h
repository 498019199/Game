#pragma once

#include <game/GameApi.h>
#include <game/gas/SkillDef.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace CommonWorker
{
class JsonValue;
}

namespace Gas
{

class GAME_API SkillCatalog
{
public:
	bool LoadSkillsJson(std::string_view path);
	bool LoadFlowsJson(std::string_view path);
	void Clear();

	SkillDef const* FindSkill(GasId id) const;
	SkillFlowDef const* FindFlow(GasId id) const;
	SkillDef const* FindSkillByName(std::string_view name) const;

	std::unordered_map<GasId, SkillDef> const& Skills() const noexcept { return skills_; }
	std::unordered_map<GasId, SkillFlowDef> const& Flows() const noexcept { return flows_; }

private:
	static bool ParseSkill(CommonWorker::JsonValue const& node, SkillDef& out, std::string& error);
	static bool ParseFlow(CommonWorker::JsonValue const& node, SkillFlowDef& out, std::string& error);

	std::unordered_map<GasId, SkillDef> skills_;
	std::unordered_map<GasId, SkillFlowDef> flows_;
};

} // namespace Gas
