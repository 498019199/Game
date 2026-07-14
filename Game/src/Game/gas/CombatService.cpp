#include <game/gas/CombatService.h>

#include <game/gas/SkillCatalog.h>
#include <game/gas/SkillPipeline.h>

#include <common/Log.h>

#include <cmath>

namespace Gas
{
namespace
{
SkillCatalog const* g_skill_catalog = nullptr;
}

void CombatService::SetSkillCatalog(SkillCatalog const* catalog)
{
	g_skill_catalog = catalog;
}

SkillCatalog const* CombatService::GetSkillCatalog()
{
	return g_skill_catalog;
}

float CombatService::ApplyDamage(AbilitySystemComponent& source, AbilitySystemComponent& target, float amount)
{
	if (amount <= 0.f)
	{
		return 0.f;
	}

	float const before = target.GetAttribute(AttributeId::HP);
	target.Attributes().ApplyHPDelta(-amount);
	float const after = target.GetAttribute(AttributeId::HP);
	float const dealt = before - after;

	GasEvent evt;
	evt.type = GasEventType::DamageApplied;
	evt.source = &source;
	evt.target = &target;
	evt.damage = dealt;
	source.Broadcast(evt);
	if (&source != &target)
	{
		target.Broadcast(evt);
	}

	return dealt;
}

void CombatService::HandleCommand(CombatCommand& cmd)
{
	switch (cmd.type)
	{
	case CombatCommandType::ApplyDamage:
		if (cmd.source && cmd.target)
		{
			float dmg = cmd.damage;
			if (dmg <= 0.f)
			{
				dmg = cmd.source->GetAttribute(AttributeId::ATK);
			}
			ApplyDamage(*cmd.source, *cmd.target, dmg);
		}
		break;
	case CombatCommandType::ApplyEffect:
		if (cmd.target)
		{
			cmd.target->ApplyEffect(cmd.effect_spec);
		}
		break;
	case CombatCommandType::CastSkill:
		if (cmd.source && cmd.target)
		{
			if (!g_skill_catalog)
			{
				LogError() << "[GAS] CastSkill: SkillCatalog not set" << std::endl;
				break;
			}
			ActivateFailReason fail = ActivateFailReason::None;
			SkillLaunchContext ctx;
			ctx.source = cmd.source;
			ctx.target = cmd.target;
			if (!AbilityLauncher::Launch(*g_skill_catalog, ctx, cmd.skill_id, &fail))
			{
				LogInfo() << "[GAS] CastSkill fail skill=" << cmd.skill_id
						  << " reason=" << static_cast<int>(fail) << std::endl;
			}
		}
		break;
	default:
		break;
	}
}

EffectSpec MakeSampleAtkBuff(TimeMs duration_ms, float percent_add)
{
	EffectSpec spec;
	spec.effect_def_id = 1001;
	spec.duration_policy = EffectDurationPolicy::Duration;
	spec.duration_ms = duration_ms;
	spec.granted_tags.push_back("Buff.AtkUp");
	EffectModifierDef mod;
	mod.attribute = AttributeId::ATK;
	mod.op = ModifierOp::PercentAdd;
	mod.value = percent_add;
	spec.modifiers.push_back(mod);
	return spec;
}

bool RunGasSmokeTest()
{
	using CommonWorker::LogInfo;
	using CommonWorker::LogError;

	AbilitySystemComponent attacker;
	AbilitySystemComponent defender;
	attacker.SetDebugName("Attacker");
	defender.SetDebugName("Defender");
	attacker.InitDefaultCombatStats(100.f, 10.f, 50.f);
	defender.InitDefaultCombatStats(100.f, 5.f, 50.f);

	int applied = 0;
	int removed = 0;
	attacker.AddEventHandler([&](GasEvent const& e)
	{
		if (e.type == GasEventType::EffectApplied)
		{
			++applied;
		}
		if (e.type == GasEventType::EffectRemoved)
		{
			++removed;
		}
	});

	float const atk0 = attacker.GetAttribute(AttributeId::ATK);
	GasId buff_id = attacker.ApplyEffect(MakeSampleAtkBuff(500, 0.2f));
	float const atk1 = attacker.GetAttribute(AttributeId::ATK);
	if (std::fabs(atk1 - atk0 * 1.2f) > 0.01f)
	{
		LogError() << "[GAS] smoke fail: ATK buff expected " << (atk0 * 1.2f) << " got " << atk1 << std::endl;
		return false;
	}

	attacker.Tags().AddTag(Tags::StateStunned);
	if (attacker.CanMove() || attacker.CanCast())
	{
		LogError() << "[GAS] smoke fail: stunned should block move/cast" << std::endl;
		return false;
	}
	attacker.Tags().RemoveTag(Tags::StateStunned);

	CombatBeat beat;
	beat.Register(&attacker);
	beat.Register(&defender);

	CombatCommand dmg;
	dmg.type = CombatCommandType::ApplyDamage;
	dmg.source = &attacker;
	dmg.target = &defender;
	dmg.damage = attacker.GetAttribute(AttributeId::ATK);
	beat.Enqueue(std::move(dmg));
	beat.Step(0);

	float const hp = defender.GetAttribute(AttributeId::HP);
	if (std::fabs(hp - (100.f - atk1)) > 0.01f)
	{
		LogError() << "[GAS] smoke fail: HP expected " << (100.f - atk1) << " got " << hp << std::endl;
		return false;
	}

	// Expire buff via beat continuous + cleanup.
	beat.Step(500);
	float const atk2 = attacker.GetAttribute(AttributeId::ATK);
	if (std::fabs(atk2 - atk0) > 0.01f || attacker.HasTag("Buff.AtkUp"))
	{
		LogError() << "[GAS] smoke fail: buff should expire ATK=" << atk2 << std::endl;
		return false;
	}

	if (applied < 1 || removed < 1)
	{
		LogError() << "[GAS] smoke fail: events applied=" << applied << " removed=" << removed << std::endl;
		return false;
	}

	(void)buff_id;
	LogInfo() << "[GAS] smoke ok: ATK " << atk0 << "->" << atk1 << " damage, defender HP=" << hp
			  << " buff expired ATK=" << atk2 << std::endl;
	return true;
}

bool RunGasSkillConfigSmokeTest()
{
	using CommonWorker::LogInfo;
	using CommonWorker::LogError;

	SkillCatalog catalog;
	if (!catalog.LoadSkillsJson("Config/skills.json") || !catalog.LoadFlowsJson("Config/skill_flows.json"))
	{
		LogError() << "[GAS] skill-config smoke fail: load json" << std::endl;
		return false;
	}
	if (!catalog.FindSkill(1001) || !catalog.FindFlow(2001))
	{
		LogError() << "[GAS] skill-config smoke fail: missing BasicStrike entries" << std::endl;
		return false;
	}

	AbilitySystemComponent attacker;
	AbilitySystemComponent defender;
	attacker.InitDefaultCombatStats(100.f, 10.f, 50.f);
	defender.InitDefaultCombatStats(100.f, 5.f, 50.f);

	CombatService::SetSkillCatalog(&catalog);
	CombatBeat beat;
	beat.Register(&attacker);
	beat.Register(&defender);

	CombatCommand cast;
	cast.type = CombatCommandType::CastSkill;
	cast.source = &attacker;
	cast.target = &defender;
	cast.skill_id = 1001;
	beat.Enqueue(std::move(cast));
	beat.Step(0); // RulePlan + t=0 cue

	if (!attacker.IsCasting())
	{
		LogError() << "[GAS] skill-config smoke fail: expected casting after launch" << std::endl;
		CombatService::SetSkillCatalog(nullptr);
		return false;
	}

	beat.Step(200); // EmitHit at 200
	float const hp_after_hit = defender.GetAttribute(AttributeId::HP);
	if (std::fabs(hp_after_hit - 90.f) > 0.01f)
	{
		LogError() << "[GAS] skill-config smoke fail: HP after hit expected 90 got " << hp_after_hit << std::endl;
		CombatService::SetSkillCatalog(nullptr);
		return false;
	}

	beat.Step(100); // End at 300
	if (attacker.IsCasting())
	{
		LogError() << "[GAS] skill-config smoke fail: still casting after End" << std::endl;
		CombatService::SetSkillCatalog(nullptr);
		return false;
	}

	// ManaBolt insufficient mp
	attacker.SpendMp(45.f); // leave 5, cost 10
	ActivateFailReason fail = ActivateFailReason::None;
	SkillLaunchContext ctx {&attacker, &defender, 1};
	bool launched = AbilityLauncher::Launch(catalog, ctx, 1002, &fail);
	if (launched || fail != ActivateFailReason::InsufficientMp)
	{
		LogError() << "[GAS] skill-config smoke fail: expected InsufficientMp, launched=" << launched
				   << " fail=" << static_cast<int>(fail) << std::endl;
		CombatService::SetSkillCatalog(nullptr);
		return false;
	}

	CombatService::SetSkillCatalog(nullptr);
	LogInfo() << "[GAS] skill-config smoke ok: BasicStrike hit, ManaBolt blocked by MP" << std::endl;
	return true;
}

} // namespace Gas
