using System;
using System.Linq;
using Game.Protocol;

namespace Game.Combat
{
    internal sealed class SkillCast
    {
        public readonly SkillDef Skill;
        public readonly int TargetId;
        private readonly SkillFlow flow;
        private int phaseIndex, eventIndex;
        private long elapsed;
        public SkillCast(SkillDef skill, SkillFlow flow, int targetId)
        { Skill = skill; this.flow = flow; TargetId = targetId; }

        public void Advance(GameWorld world, Entity source, int delta)
        {
            long budget = delta;
            while (source.Cast == this && phaseIndex < flow.Phases.Count)
            {
                Phase phase = flow.Phases[phaseIndex];
                if (phase.Kind == PhaseKind.RulePlan)
                {
                    if (!source.CanCast || Skill.BlockedByTags.Any(source.HasTag))
                        throw new GameFault(ErrorCode.Blocked, "Casting blocked");
                    if (phase.CommitCost)
                    {
                        world.ValidateCost(source, Skill);
                        if (Skill.CooldownMs > 0)
                        {
                            var cooldown = new EffectSpec { DefinitionId = Skill.Id,
                                Duration = DurationPolicy.Timed, DurationMs = Skill.CooldownMs };
                            cooldown.Tags.Add("Cooldown.Skill." + Skill.Id);
                            world.Apply(source, cooldown);
                        }
                        source.SpendMana(Skill.ManaCost);
                    }
                    ++phaseIndex;
                    continue;
                }
                long limit = elapsed + budget;
                bool next = false;
                long consumed = 0;
                while (eventIndex < phase.Events.Count && phase.Events[eventIndex].TimeMs <= limit)
                {
                    TimelineEvent ev = phase.Events[eventIndex++];
                    consumed = Math.Max(0, ev.TimeMs - elapsed);
                    if (ev.Action == TimelineAction.Cue)
                        world.Emit(new CombatEvent { Kind = EventKind.Cue, SourceId = source.Id, TargetId = TargetId, Cue = ev.Cue });
                    else if (ev.Action == TimelineAction.Hit)
                        world.Damage(source, world.Find(TargetId), source.Attribute(AttributeId.Attack) * ev.DamageScale);
                    else { next = true; break; }
                }
                if (eventIndex == phase.Events.Count) next = true;
                if (!next) { elapsed = limit; return; }
                budget -= consumed;
                ++phaseIndex; eventIndex = 0; elapsed = 0;
            }
            if (source.Cast == this && phaseIndex == flow.Phases.Count) world.Finish(source, "completed");
        }
    }
}
