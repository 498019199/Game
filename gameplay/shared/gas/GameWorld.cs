using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using Game.Protocol;

namespace Game.Combat
{
    // One world per session/room. The host serializes requests for each world.
    public sealed class GameWorld
    {
        internal readonly GameCatalog Catalog;
        internal readonly SortedDictionary<int, Entity> Entities = new SortedDictionary<int, Entity>();
        private ulong nextEffect = 1;
        private Response response = new Response();
        public ulong BeatIndex { get; private set; }
        public long TimeMs { get; private set; }
        public GameWorld(GameCatalog catalog) { Catalog = catalog; }

        public Response Handle(Request request)
        {
            response = new Response { Version = 1, RequestId = request.RequestId };
            try
            {
                if (request.Version != 1) throw new GameFault(ErrorCode.UnsupportedVersion, "Expected protocol version 1");
                switch (request.BodyCase)
                {
                    case Request.BodyOneofCase.CreateEntity:
                        Create(request.CreateEntity); break;
                    case Request.BodyOneofCase.DeleteEntity:
                        Delete(request.DeleteEntity.Id); break;
                    case Request.BodyOneofCase.CastSkill:
                        var cast = request.CastSkill;
                        Launch(Find(cast.SourceId), Find(cast.TargetId), cast.SkillId); break;
                    case Request.BodyOneofCase.Tick:
                        Step(request.Tick.DeltaMs); break;
                    case Request.BodyOneofCase.ApplyEffect:
                        if (request.ApplyEffect.Effect == null) throw new GameFault(ErrorCode.InvalidArgument, "Missing effect");
                        response.EffectInstanceId = Apply(Find(request.ApplyEffect.TargetId), request.ApplyEffect.Effect); break;
                    case Request.BodyOneofCase.RemoveEffect:
                        var remove = request.RemoveEffect;
                        Guard.Require(remove.Reason != RemoveReason.None && Enum.IsDefined(typeof(RemoveReason), remove.Reason), "Invalid removal reason");
                        Entity owner = Find(remove.TargetId);
                        ActiveEffect? effect = owner.Effects.Find(e => e.Id == remove.InstanceId);
                        if (effect == null) throw new GameFault(ErrorCode.NotFound, "Effect not found");
                        Remove(owner, effect, remove.Reason); break;
                    case Request.BodyOneofCase.SetTag:
                        Guard.Tag(request.SetTag.Tag);
                        Entity tagged = Find(request.SetTag.EntityId);
                        Guard.Require(!request.SetTag.Present || tagged.ManualTags.Count < 128 || tagged.ManualTags.Contains(request.SetTag.Tag), "Too many tags");
                        if (request.SetTag.Present) tagged.ManualTags.Add(request.SetTag.Tag);
                        else tagged.ManualTags.Remove(request.SetTag.Tag);
                        break;
                    case Request.BodyOneofCase.SetAttribute:
                        var attribute = request.SetAttribute;
                        Guard.Require(attribute.Attribute != AttributeId.None && Enum.IsDefined(typeof(AttributeId), attribute.Attribute), "Unknown attribute");
                        Guard.Value(attribute.Value);
                        Find(attribute.EntityId).SetBase(attribute.Attribute, attribute.Value); break;
                    case Request.BodyOneofCase.Damage:
                        Guard.Value(request.Damage.Amount);
                        Damage(Find(request.Damage.SourceId), Find(request.Damage.TargetId), request.Damage.Amount); break;
                    case Request.BodyOneofCase.CancelCast:
                        Finish(Find(request.CancelCast.Id), "cancelled"); break;
                    case Request.BodyOneofCase.Snapshot:
                        if (request.Snapshot.EntityId != 0) Find(request.Snapshot.EntityId);
                        break;
                    case Request.BodyOneofCase.FindNpc:
                        response.Npc = FindNpc(request.FindNpc.Id).Clone(); break;
                    case Request.BodyOneofCase.NpcCatalog:
                        response.NpcCatalog.Add(Catalog.Npcs.Values.Select(n => n.Clone())); break;
                    case Request.BodyOneofCase.Gm:
                        ExecuteGm(request.Gm.Command); break;
                    default: throw new GameFault(ErrorCode.InvalidArgument, "Missing/unknown command");
                }
            }
            catch (GameFault error) { response.Status = error.Code; response.Error = error.Message; }
            response.BeatIndex = BeatIndex; response.TimeMs = TimeMs;
            if (request.BodyCase == Request.BodyOneofCase.Snapshot && request.Snapshot.EntityId != 0)
            {
                if (Entities.TryGetValue(request.Snapshot.EntityId, out Entity? one)) response.Entities.Add(one.Snapshot());
            }
            else response.Entities.Add(Entities.Values.Select(e => e.Snapshot()));
            return response;
        }
        internal Entity Find(int id)
        {
            if (!Entities.TryGetValue(id, out Entity? entity)) throw new GameFault(ErrorCode.NotFound, "Entity not found: " + id);
            return entity;
        }
        private NpcDef FindNpc(int id)
        {
            if (!Catalog.Npcs.TryGetValue(id, out NpcDef? npc)) throw new GameFault(ErrorCode.NotFound, "NPC not found: " + id);
            return npc;
        }
        private void Create(CreateEntity definition)
        {
            Guard.Require(definition.Id > 0 && !Entities.ContainsKey(definition.Id) && Entities.Count < 128 && definition.Name.Length <= 128,
                "Invalid/duplicate entity or world limit reached");
            Guard.Value(definition.MaxHp); Guard.Value(definition.Attack); Guard.Value(definition.MaxMana);
            Guard.Require(definition.MaxHp > 0, "Max HP must be positive");
            Entities.Add(definition.Id, new Entity(definition));
        }
        private void Delete(int id)
        {
            Entity removed = Find(id);
            foreach (Entity entity in Entities.Values)
                if (entity.Id == id || entity.Cast?.TargetId == id) Finish(entity, "entity removed");
            foreach (ActiveEffect effect in removed.Effects.ToArray()) Remove(removed, effect, RemoveReason.Death);
            Entities.Remove(id);
        }
        internal void Emit(CombatEvent value) { response.Events.Add(value); }
        internal ulong Apply(Entity entity, EffectSpec spec)
        {
            Guard.Effect(spec);
            Guard.Require(entity.Effects.Count < 32, "Too many effects on entity");
            var effect = new ActiveEffect { Id = nextEffect++, Spec = spec.Clone(),
                Remaining = spec.Duration == DurationPolicy.Infinite ? -1 : spec.DurationMs };
            entity.Effects.Add(effect); entity.ClampHp();
            Emit(new CombatEvent { Kind = EventKind.EffectApplied, TargetId = entity.Id,
                DefinitionId = spec.DefinitionId, EffectInstanceId = effect.Id });
            if (spec.Duration == DurationPolicy.Instant)
            { Remove(entity, effect, RemoveReason.Expired); return 0; }
            return effect.Id;
        }
        private void Remove(Entity entity, ActiveEffect effect, RemoveReason reason)
        {
            entity.Effects.Remove(effect); entity.ClampHp();
            Emit(new CombatEvent { Kind = EventKind.EffectRemoved, TargetId = entity.Id,
                DefinitionId = effect.Spec.DefinitionId, EffectInstanceId = effect.Id, RemoveReason = reason });
        }
        internal void Damage(Entity source, Entity target, float amount)
        {
            // Gameplay damage calculation lives here only. The prototype's independent
            // integer demo was removed during migration; GAS retains normalized floats.
            Guard.Require(Guard.Finite(amount) && amount >= 0, "Invalid calculated damage");
            float damage = Math.Min(target.Hp, amount);
            target.Hp -= damage;
            Emit(new CombatEvent { Kind = EventKind.Damage, SourceId = source.Id, TargetId = target.Id, Damage = damage });
            if (target.Hp == 0) Finish(target, "death");
        }
        internal void Finish(Entity source, string cue)
        {
            if (source.Cast == null) return;
            source.Cast = null;
            Emit(new CombatEvent { Kind = EventKind.CastEnded, SourceId = source.Id, Cue = cue });
        }
        private void Launch(Entity source, Entity target, int skillId)
        {
            if (source.Cast != null) throw new GameFault(ErrorCode.Busy, "Already casting");
            if (!Catalog.Skills.TryGetValue(skillId, out SkillDef? skill)) throw new GameFault(ErrorCode.NotFound, "Unknown skill");
            if (target.Hp <= 0 || source.Id == target.Id) throw new GameFault(ErrorCode.InvalidArgument, "Invalid skill target");
            if (!source.CanCast || skill.BlockedByTags.Any(source.HasTag)) throw new GameFault(ErrorCode.Blocked, "Casting blocked by state/tag");
            if (source.HasTag("Cooldown.Skill." + skillId)) throw new GameFault(ErrorCode.Cooldown, "Skill on cooldown");
            // Validate first commitment before installing a cast or mutating mana.
            Phase first = Catalog.Flows[skill.FlowId].Phases[0];
            if (first.Kind == PhaseKind.RulePlan && first.CommitCost)
                ValidateCost(source, skill);
            source.Cast = new SkillCast(skill, Catalog.Flows[skill.FlowId], target.Id);
            try { source.Cast.Advance(this, source, 0); }
            catch (GameFault) { Finish(source, "failed"); throw; }
        }
        internal void ValidateCost(Entity source, SkillDef skill)
        {
            if (source.Attribute(AttributeId.Mana) + 0.001f < skill.ManaCost)
                throw new GameFault(ErrorCode.InsufficientMana, "Insufficient mana");
            Guard.Require(skill.CooldownMs == 0 || source.Effects.Count < 32, "Too many effects for cooldown");
        }
        private void Step(int delta)
        {
            Guard.Require(delta >= 0 && delta <= 60000, "Tick must be between 0 and 60000 ms");
            TimeMs += delta;
            // Same phase order as the former CombatBeat: effects, timelines, cleanup.
            foreach (Entity entity in Entities.Values)
            {
                foreach (ActiveEffect effect in entity.Effects)
                {
                    long activeDelta = effect.Spec.Duration == DurationPolicy.Timed ? Math.Min(delta, Math.Max(0, effect.Remaining)) : delta;
                    if (effect.Spec.PeriodMs > 0)
                    {
                        effect.PeriodElapsed += activeDelta;
                        uint ticks = (uint)(effect.PeriodElapsed / effect.Spec.PeriodMs);
                        effect.PeriodElapsed %= effect.Spec.PeriodMs;
                        if (ticks > 0) Emit(new CombatEvent { Kind = EventKind.EffectTicked,
                            TargetId = entity.Id, DefinitionId = effect.Spec.DefinitionId,
                            EffectInstanceId = effect.Id, TickCount = ticks });
                    }
                    if (effect.Spec.Duration == DurationPolicy.Timed)
                    { effect.Remaining -= delta; effect.Expired = effect.Remaining <= 0; }
                }
                try { entity.Cast?.Advance(this, entity, delta); }
                catch (GameFault error) { Finish(entity, error.Message); }
            }
            foreach (Entity entity in Entities.Values)
                foreach (ActiveEffect effect in entity.Effects.Where(e => e.Expired).ToArray())
                    Remove(entity, effect, RemoveReason.Expired);
            ++BeatIndex;
        }
        private void ExecuteGm(string command)
        {
            Guard.Require(command.Length <= 1024, "Command too long");
            string[] words = command.Split(new[] { ' ', '\t', '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries);
            if (words.Length == 0) return;
            if (words[0] == "help")
                response.Message = "commands: help, clear, /createnpc <id>, /gas smoke, /gas skillsmoke";
            else if (words[0] == "clear") response.ClearLog = true;
            else if (words[0] == "/createnpc")
            {
                Guard.Require(words.Length >= 2 && int.TryParse(words[1], NumberStyles.Integer, CultureInfo.InvariantCulture, out _), "usage: /createnpc <id>");
                response.Npc = FindNpc(int.Parse(words[1], CultureInfo.InvariantCulture)).Clone();
                response.Message = "spawn npc " + response.Npc.Id;
            }
            else if (words[0] == "/gas" && words.Length == 2 && (words[1] == "smoke" || words[1] == "skillsmoke"))
            {
                SmokeChecks.Run(Catalog, words[1] == "skillsmoke");
                response.Message = "C# gas " + words[1] + ": OK";
            }
            else response.Message = "unknown: " + command;
        }
    }
}
