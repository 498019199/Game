using System;
using System.Collections.Generic;
using Game.Protocol;

namespace Game.Combat
{
    public sealed class GameFault : Exception
    {
        public GameFault(ErrorCode code, string message) : base(message) { Code = code; }
        public ErrorCode Code { get; }
    }
    internal static class Guard
    {
        public static void Require(bool valid, string message)
        { if (!valid) throw new GameFault(ErrorCode.InvalidArgument, message); }
        public static bool Finite(float v) => !float.IsNaN(v) && !float.IsInfinity(v);
        public static void Value(float v) => Require(Finite(v) && v >= 0 && v <= 1_000_000_000, "Invalid attribute/amount");
        public static void Tag(string tag) => Require(tag.Length > 0 && tag.Length <= 128, "Invalid tag");
        public static void Effect(EffectSpec spec)
        {
            if (spec == null) throw new GameFault(ErrorCode.InvalidArgument, "Missing effect");
            Require(spec.DefinitionId > 0, "Invalid effect definition");
            Require(Enum.IsDefined(typeof(DurationPolicy), spec.Duration), "Unknown effect duration");
            Require(spec.DurationMs >= 0 && spec.PeriodMs >= 0, "Negative effect time");
            Require(spec.Tags.Count <= 8 && spec.Modifiers.Count <= 8, "Effect exceeds limits");
            foreach (string tag in spec.Tags) Tag(tag);
            foreach (Modifier mod in spec.Modifiers)
                Require(Enum.IsDefined(typeof(AttributeId), mod.Attribute) && mod.Attribute != AttributeId.None &&
                    Enum.IsDefined(typeof(ModifierOp), mod.Op) && Finite(mod.Value) && Math.Abs(mod.Value) <= 1_000_000,
                    "Invalid effect modifier");
        }
    }
    public sealed class GameCatalog
    {
        internal readonly Dictionary<int, SkillDef> Skills = new Dictionary<int, SkillDef>();
        internal readonly Dictionary<int, SkillFlow> Flows = new Dictionary<int, SkillFlow>();
        internal readonly SortedDictionary<int, NpcDef> Npcs = new SortedDictionary<int, NpcDef>();
        public GameCatalog(Catalog source)
        {
            if (source == null) throw new GameFault(ErrorCode.InvalidArgument, "Missing catalog");
            Guard.Require(source.Skills.Count <= 4096 && source.Flows.Count <= 4096 && source.Npcs.Count <= 16384, "Catalog too large");
            foreach (SkillFlow input in source.Flows)
            {
                var flow = input.Clone();
                Guard.Require(flow.Id > 0 && !Flows.ContainsKey(flow.Id), "Invalid/duplicate flow id");
                Guard.Require(flow.Phases.Count > 0 && flow.Phases.Count <= 64, "Invalid flow phases");
                foreach (Phase phase in flow.Phases)
                {
                    Guard.Require(Enum.IsDefined(typeof(PhaseKind), phase.Kind) && phase.Events.Count <= 128, "Invalid phase");
                    int previous = -1;
                    foreach (TimelineEvent ev in phase.Events)
                    {
                        Guard.Require(ev.TimeMs >= 0 && ev.TimeMs >= previous &&
                            Enum.IsDefined(typeof(TimelineAction), ev.Action) && ev.Cue.Length <= 128,
                            "Invalid/unsorted timeline event");
                        Guard.Value(ev.DamageScale);
                        previous = ev.TimeMs;
                    }
                }
                Flows.Add(flow.Id, flow);
            }
            foreach (SkillDef input in source.Skills)
            {
                var skill = input.Clone();
                Guard.Require(skill.Id > 0 && !Skills.ContainsKey(skill.Id) && Flows.ContainsKey(skill.FlowId), "Invalid skill/flow reference");
                Guard.Value(skill.ManaCost);
                Guard.Require(skill.CooldownMs >= 0 && skill.BlockedByTags.Count <= 32, "Invalid skill restrictions");
                foreach (string tag in skill.BlockedByTags) Guard.Tag(tag);
                Skills.Add(skill.Id, skill);
            }
            foreach (NpcDef input in source.Npcs)
            {
                var npc = input.Clone();
                npc.Prefab = npc.Prefab.Replace('\\', '/').TrimStart('/');
                Guard.Require(npc.Id > 0 && !Npcs.ContainsKey(npc.Id) && npc.Prefab.Length > 0 &&
                    npc.Prefab.Length <= 1024 && !npc.Prefab.Contains(".."), "Invalid NPC definition");
                Npcs.Add(npc.Id, npc);
            }
        }
    }
}
