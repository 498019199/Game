#include <game/gas/SkillCatalog.h>

#include <base/ZEngine.h>
#include <common/JsonDom.h>
#include <common/Log.h>

#include <cstring>

namespace Gas
{
using JsonValue = CommonWorker::JsonValue;

namespace
{
int32_t JsonAsInt(CommonWorker::JsonValue const& v, int32_t fallback = 0)
{
	using CommonWorker::JsonValueType;
	switch (v.Type())
	{
	case JsonValueType::Int:
		return v.ValueInt();
	case JsonValueType::UInt:
		return static_cast<int32_t>(v.ValueUInt());
	case JsonValueType::Float:
		return static_cast<int32_t>(v.ValueFloat());
	default:
		return fallback;
	}
}

float JsonAsFloat(CommonWorker::JsonValue const& v, float fallback = 0.f)
{
	using CommonWorker::JsonValueType;
	switch (v.Type())
	{
	case JsonValueType::Float:
		return v.ValueFloat();
	case JsonValueType::Int:
		return static_cast<float>(v.ValueInt());
	case JsonValueType::UInt:
		return static_cast<float>(v.ValueUInt());
	default:
		return fallback;
	}
}

std::string JsonAsString(CommonWorker::JsonValue const& v)
{
	if (v.Type() == CommonWorker::JsonValueType::String)
	{
		return std::string(v.ValueString());
	}
	return {};
}

bool LoadJsonFile(std::string_view path, CommonWorker::JsonValue& out)
{
	auto& res_loader = Context::Instance().ResLoaderInstance();
	ResIdentifierPtr file = res_loader.Open(path);
	if (!file)
	{
		LogError() << "[GAS] SkillCatalog: cannot open " << path << std::endl;
		return false;
	}
	out = LoadJson(*file);
	return true;
}
} // namespace

bool SkillCatalog::ParseSkill(CommonWorker::JsonValue const& node, SkillDef& out, std::string& error)
{
	if (node.Type() != CommonWorker::JsonValueType::Object)
	{
		error = "skill entry must be object";
		return false;
	}

	JsonValue const* id = node.Member("id");
	if (!id)
	{
		error = "skill missing id";
		return false;
	}
	out.id = JsonAsInt(*id);
	if (out.id == kInvalidGasId)
	{
		error = "skill id invalid";
		return false;
	}

	if (JsonValue const* name = node.Member("name"))
	{
		out.name = JsonAsString(*name);
	}
	if (JsonValue const* slot = node.Member("slot"))
	{
		out.slot = static_cast<SkillSlot>(JsonAsInt(*slot));
	}
	if (JsonValue const* flow = node.Member("cast_flow_id"))
	{
		out.cast_flow_id = JsonAsInt(*flow);
	}
	if (out.cast_flow_id == kInvalidGasId)
	{
		error = "skill missing cast_flow_id";
		return false;
	}
	if (JsonValue const* cost = node.Member("cost_mp"))
	{
		out.cost_mp = JsonAsFloat(*cost);
	}
	if (JsonValue const* cd = node.Member("cooldown_ms"))
	{
		out.cooldown_ms = JsonAsInt(*cd);
	}
	if (JsonValue const* blocked = node.Member("blocked_by_tags"))
	{
		if (blocked->Type() == CommonWorker::JsonValueType::Array)
		{
			for (JsonValue const& t : blocked->ValueArray())
			{
				out.blocked_by_tags.push_back(JsonAsString(t));
			}
		}
	}
	return true;
}

bool SkillCatalog::ParseFlow(CommonWorker::JsonValue const& node, SkillFlowDef& out, std::string& error)
{
	if (node.Type() != CommonWorker::JsonValueType::Object)
	{
		error = "flow entry must be object";
		return false;
	}

	JsonValue const* id = node.Member("id");
	if (!id)
	{
		error = "flow missing id";
		return false;
	}
	out.id = JsonAsInt(*id);
	if (out.id == kInvalidGasId)
	{
		error = "flow id invalid";
		return false;
	}
	if (JsonValue const* name = node.Member("name"))
	{
		out.name = JsonAsString(*name);
	}

	JsonValue const* phases = node.Member("phases");
	if (!phases || phases->Type() != CommonWorker::JsonValueType::Array)
	{
		error = "flow missing phases array";
		return false;
	}

	for (JsonValue const& phase_node : phases->ValueArray())
	{
		if (phase_node.Type() != CommonWorker::JsonValueType::Object)
		{
			error = "phase must be object";
			return false;
		}
		JsonValue const* type_v = phase_node.Member("type");
		std::string type = type_v ? JsonAsString(*type_v) : "";
		FlowPhaseDef phase;
		if (type == "RulePlan")
		{
			phase.type = FlowPhaseType::RulePlan;
			if (JsonValue const* commit = phase_node.Member("commit_cost"))
			{
				if (commit->Type() == CommonWorker::JsonValueType::Bool)
				{
					phase.rule_plan.commit_cost = commit->ValueBool();
				}
			}
		}
		else if (type == "Timeline")
		{
			phase.type = FlowPhaseType::Timeline;
			JsonValue const* events = phase_node.Member("events");
			if (!events || events->Type() != CommonWorker::JsonValueType::Array)
			{
				error = "Timeline missing events";
				return false;
			}
			for (JsonValue const& ev : events->ValueArray())
			{
				TimelineEventDef te;
				if (JsonValue const* t = ev.Member("t_ms"))
				{
					te.t_ms = JsonAsInt(*t);
				}
				std::string action = ev.Member("action") ? JsonAsString(*ev.Member("action")) : "";
				if (action == "Cue")
				{
					te.action = TimelineAction::Cue;
				}
				else if (action == "EmitHit")
				{
					te.action = TimelineAction::EmitHit;
				}
				else if (action == "End")
				{
					te.action = TimelineAction::End;
				}
				else
				{
					error = "unknown timeline action: " + action;
					return false;
				}
				if (JsonValue const* cue = ev.Member("cue"))
				{
					te.cue = JsonAsString(*cue);
				}
				if (JsonValue const* scale = ev.Member("damage_scale"))
				{
					te.damage_scale = JsonAsFloat(*scale, 1.f);
					if (LooksLikeWrongPercentMagnitude(te.damage_scale))
					{
						LogError() << "[GAS] damage_scale looks like pct int, use normalized float (got "
								   << te.damage_scale << ")" << std::endl;
					}
				}
				phase.timeline.events.push_back(std::move(te));
			}
		}
		else if (type == "WaitUntil")
		{
			LogInfo() << "[GAS] WaitUntil phase ignored in P0 flow " << out.id << std::endl;
			continue;
		}
		else
		{
			error = "unknown phase type: " + type;
			return false;
		}
		out.phases.push_back(std::move(phase));
	}

	if (out.phases.empty())
	{
		error = "flow has no phases";
		return false;
	}
	return true;
}

bool SkillCatalog::LoadSkillsJson(std::string_view path)
{
	CommonWorker::JsonValue root;
	if (!LoadJsonFile(path, root))
	{
		return false;
	}
	if (root.Type() != CommonWorker::JsonValueType::Array)
	{
		LogError() << "[GAS] skills.json root must be array: " << path << std::endl;
		return false;
	}

	for (JsonValue const& node : root.ValueArray())
	{
		SkillDef def;
		std::string error;
		if (!ParseSkill(node, def, error))
		{
			LogError() << "[GAS] skill parse fail: " << error << std::endl;
			return false;
		}
		if (skills_.contains(def.id))
		{
			LogError() << "[GAS] duplicate skill id " << def.id << std::endl;
			return false;
		}
		skills_.emplace(def.id, std::move(def));
	}
	LogInfo() << "[GAS] loaded " << skills_.size() << " skills from " << path << std::endl;
	return true;
}

bool SkillCatalog::LoadFlowsJson(std::string_view path)
{
	CommonWorker::JsonValue root;
	if (!LoadJsonFile(path, root))
	{
		return false;
	}
	if (root.Type() != CommonWorker::JsonValueType::Array)
	{
		LogError() << "[GAS] skill_flows.json root must be array: " << path << std::endl;
		return false;
	}

	for (JsonValue const& node : root.ValueArray())
	{
		SkillFlowDef def;
		std::string error;
		if (!ParseFlow(node, def, error))
		{
			LogError() << "[GAS] flow parse fail: " << error << std::endl;
			return false;
		}
		if (flows_.contains(def.id))
		{
			LogError() << "[GAS] duplicate flow id " << def.id << std::endl;
			return false;
		}
		flows_.emplace(def.id, std::move(def));
	}
	LogInfo() << "[GAS] loaded " << flows_.size() << " flows from " << path << std::endl;
	return true;
}

void SkillCatalog::Clear()
{
	skills_.clear();
	flows_.clear();
}

SkillDef const* SkillCatalog::FindSkill(GasId id) const
{
	auto it = skills_.find(id);
	return it == skills_.end() ? nullptr : &it->second;
}

SkillFlowDef const* SkillCatalog::FindFlow(GasId id) const
{
	auto it = flows_.find(id);
	return it == flows_.end() ? nullptr : &it->second;
}

SkillDef const* SkillCatalog::FindSkillByName(std::string_view name) const
{
	for (auto const& [id, def] : skills_)
	{
		(void)id;
		if (def.name == name)
		{
			return &def;
		}
	}
	return nullptr;
}

} // namespace Gas
