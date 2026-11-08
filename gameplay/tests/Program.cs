using System;
using System.IO;
using System.Linq;
using Game.Combat;
using Game.Protocol;
using Game.Runtime;
using Google.Protobuf;

if (args.Length != 2) throw new ArgumentException("Expected CATALOG.pb OUTPUT_DIRECTORY");
Directory.CreateDirectory(args[1]);
Catalog source = Catalog.Parser.ParseFrom(File.ReadAllBytes(args[0]));
var catalog = new GameCatalog(source);
var session = new ProtocolSession(catalog);
using var requests = File.Create(Path.Combine(args[1], "requests.pb"));
using var responses = File.Create(Path.Combine(args[1], "expected.pb"));
ulong sequence = 0;
int count = 0;

void Check(bool condition, string label)
{ if (!condition) throw new InvalidOperationException("FAIL: " + label); }
EntitySnapshot E(Response r, int id = 1) => r.Entities.Single(e => e.Id == id);
void Near(float actual, float expected) => Check(Math.Abs(actual - expected) < 0.001f, $"expected {expected}, got {actual}");
Response Send(Request request, ErrorCode status = ErrorCode.Ok, Action<Response>? verify = null)
{
    request.Version = 1; request.RequestId = ++sequence;
    byte[] bytes = request.ToByteArray();
    byte[] output = session.Exchange(bytes);
    Response result = Response.Parser.ParseFrom(output);
    Check(result.Version == 1 && result.RequestId == sequence, "response envelope");
    Check(result.Status == status, $"{request.BodyCase}: {result.Status} != {status}: {result.Error}");
    verify?.Invoke(result);
    Frames.Write(requests, bytes); Frames.Write(responses, output); ++count;
    return result;
}
Request Tick(int ms) => new Request { Tick = new Tick { DeltaMs = ms } };
Request Cast(int skill) => new Request { CastSkill = new CastSkill { SourceId = 1, TargetId = 2, SkillId = skill } };
Request Tag(string tag, bool present) => new Request { SetTag = new SetTag { EntityId = 1, Tag = tag, Present = present } };
Request Stat(AttributeId attribute, float value) => new Request { SetAttribute = new SetAttribute { EntityId = 1, Attribute = attribute, Value = value } };
Request Snapshot() => new Request { Snapshot = new SnapshotQuery() };

Send(new Request { CreateEntity = new CreateEntity { Id = 1, Name = "攻击者", MaxHp = 100, Attack = 10, MaxMana = 50 } }, verify: r => Near(E(r).Hp, 100));
Send(new Request { CreateEntity = new CreateEntity { Id = 2, Name = "Defender", MaxHp = 100, Attack = 5, MaxMana = 50 } });
Send(new Request { CreateEntity = new CreateEntity { Id = 1, MaxHp = 100 } }, ErrorCode.InvalidArgument);
var buff = new EffectSpec { DefinitionId = 7, Duration = DurationPolicy.Timed, DurationMs = 500, PeriodMs = 100 };
buff.Tags.Add("Buff.AtkUp");
buff.Modifiers.Add(new Modifier { Attribute = AttributeId.Attack, Op = ModifierOp.PercentAdd, Value = 0.2f });
ulong first = Send(new Request { ApplyEffect = new ApplyEffect { TargetId = 1, Effect = buff } }, verify: r => Near(E(r).Attack, 12)).EffectInstanceId;
var secondBuff = new EffectSpec { DefinitionId = 8, Duration = DurationPolicy.Infinite };
secondBuff.Tags.Add("Buff.AtkUp");
secondBuff.Modifiers.Add(new Modifier { Attribute = AttributeId.Attack, Op = ModifierOp.Add, Value = 5 });
ulong second = Send(new Request { ApplyEffect = new ApplyEffect { TargetId = 1, Effect = secondBuff } }, verify: r => Near(E(r).Attack, 18)).EffectInstanceId;
Send(new Request { RemoveEffect = new RemoveEffect { TargetId = 1, InstanceId = first, Reason = RemoveReason.Manual } }, verify: r => {
    Near(E(r).Attack, 15); Check(E(r).Tags.Contains("Buff.AtkUp"), "overlapping tag retained"); });
Send(Tag("Buff.AtkUp", false), verify: r => Check(E(r).Tags.Contains("Buff.AtkUp"), "manual removal cannot remove effect-owned tag"));
Send(new Request { RemoveEffect = new RemoveEffect { TargetId = 1, InstanceId = second, Reason = RemoveReason.Dispelled } }, verify: r => {
    Near(E(r).Attack, 10); Check(!E(r).Tags.Contains("Buff.AtkUp"), "last tag removed"); });
Send(Tag("State.Stunned.Heavy", true), verify: r => Check(!E(r).CanMove && !E(r).CanCast, "hierarchical state tags"));
Send(Cast(1001), ErrorCode.Blocked, r => { Near(E(r).Mana, 50); Check(!E(r).Casting, "blocked cast is atomic"); });
Send(Tag("State.Stunned.Heavy", false));
Send(Cast(1001), verify: r => Check(E(r).Casting && r.Events.Any(e => e.Kind == EventKind.Cue && e.Cue == "CastStart"), "t=0 cue"));
Send(Cast(1002), ErrorCode.Busy);
Send(Tick(199), verify: r => Near(E(r, 2).Hp, 100));
Send(Tick(1), verify: r => { Near(E(r, 2).Hp, 90); Near(r.Events.Single(e => e.Kind == EventKind.Damage).Damage, 10); });
Send(Tick(100), verify: r => Check(!E(r).Casting && !E(r).Tags.Contains("Ability.Active"), "timeline ended"));
Send(Cast(1002), verify: r => { Near(E(r).Mana, 40); Check(E(r).Tags.Contains("Cooldown.Skill.1002"), "cooldown granted"); });
Send(Tick(250), verify: r => Near(E(r, 2).Hp, 75));
Send(Tick(150), verify: r => Check(!E(r).Casting, "mana bolt ended"));
Send(Cast(1002), ErrorCode.Cooldown, r => Near(E(r).Mana, 40));
Send(Tick(600), verify: r => Check(!E(r).Tags.Contains("Cooldown.Skill.1002"), "cooldown exact expiry"));
Send(Stat(AttributeId.Mana, 5));
Send(Cast(1002), ErrorCode.InsufficientMana, r => Near(E(r).Mana, 5));
Send(Cast(99999), ErrorCode.NotFound);
Send(new Request { ApplyEffect = new ApplyEffect { TargetId = 1, Effect = buff } });
Send(Tick(600), verify: r => {
    Check(r.Events.Single(e => e.Kind == EventKind.EffectTicked).TickCount == 5, "no ticks past expiry");
    Near(E(r).Attack, 10); Check(E(r).Effects.Count == 0, "expired effect removed"); });
var instant = buff.Clone(); instant.Duration = DurationPolicy.Instant;
Send(new Request { ApplyEffect = new ApplyEffect { TargetId = 1, Effect = instant } }, verify: r => {
    Check(r.EffectInstanceId == 0 && E(r).Effects.Count == 0, "instant effect lifecycle");
    Check(r.Events.Count == 2, "instant apply/remove events"); Near(E(r).Attack, 10); });
Send(Stat(AttributeId.MaxHp, 80), verify: r => Near(E(r).Hp, 80));
Send(Stat(AttributeId.Hp, 100), verify: r => Near(E(r).Hp, 80));
Send(Stat(AttributeId.Mana, float.NaN), ErrorCode.InvalidArgument, r => Near(E(r).Mana, 5));
Send(Tick(-1), ErrorCode.InvalidArgument);
Send(Tick(60001), ErrorCode.InvalidArgument);
Send(new Request { ApplyEffect = new ApplyEffect { TargetId = 1 } }, ErrorCode.InvalidArgument);
Send(new Request { RemoveEffect = new RemoveEffect { TargetId = 1, InstanceId = 9999, Reason = RemoveReason.Manual } }, ErrorCode.NotFound);
Send(Cast(1001));
Send(new Request { DeleteEntity = new EntityId { Id = 2 } }, verify: r => Check(r.Entities.Count == 1 && !E(r).Casting, "deletion cancels incoming cast"));
Send(new Request { CreateEntity = new CreateEntity { Id = 2, Name = "Reused ID", MaxHp = 100, Attack = 5, MaxMana = 50 } });
Send(Tick(1000), verify: r => Near(E(r, 2).Hp, 100));
Send(Cast(1001));
Send(new Request { CancelCast = new EntityId { Id = 1 } });
Send(Tick(1000), verify: r => Near(E(r, 2).Hp, 100));
Send(new Request { Damage = new DealDamage { SourceId = 1, TargetId = 2, Amount = 1000 } }, verify: r => {
    Near(E(r, 2).Hp, 0); Near(r.Events.Single(e => e.Kind == EventKind.Damage).Damage, 100); });
Send(Cast(1001), ErrorCode.InvalidArgument);
Send(new Request { FindNpc = new EntityId { Id = 300001 } }, verify: r => Check(r.Npc.Prefab == "prefabs/model/chaos_knight.prefab", "NPC catalog"));
Send(new Request { FindNpc = new EntityId { Id = 999999 } }, ErrorCode.NotFound);
Send(new Request { NpcCatalog = new Empty() }, verify: r => Check(r.NpcCatalog.Count == source.Npcs.Count, "NPC entries"));
foreach (string command in new[] { "help", "clear", "   ", "/gas smoke", "/gas skillsmoke", "/createnpc 100000" })
    Send(new Request { Gm = new ExecuteGm { Command = command } });
Send(new Request { Gm = new ExecuteGm { Command = "/createnpc not-an-id" } }, ErrorCode.InvalidArgument);
Send(new Request(), ErrorCode.InvalidArgument);

// Unknown fields must survive version-compatible requests. Malformed payloads
// must fail without corrupting the following valid exchange.
foreach (var (bytes, expected) in new[] {
    (new byte[] { 0 }, ErrorCode.InvalidArgument),
    (new Request { Version = 2, RequestId = 999, Snapshot = new SnapshotQuery() }.ToByteArray(), ErrorCode.UnsupportedVersion),
    (new Request { Version = 1, RequestId = 1000, Snapshot = new SnapshotQuery() }.ToByteArray().Concat(new byte[] { 0xa0, 0x1f, 7 }).ToArray(), ErrorCode.Ok) })
{
    byte[] output = session.Exchange(bytes);
    Check(Response.Parser.ParseFrom(output).Status == expected, "wire compatibility/error boundary");
    Frames.Write(requests, bytes); Frames.Write(responses, output); ++count;
}
Send(Snapshot(), verify: r => { Near(E(r).Hp, 80); Near(E(r, 2).Hp, 0); });

var independent = new GameWorld(catalog);
Check(independent.Handle(new Request { Version = 1, Snapshot = new SnapshotQuery() }).Entities.Count == 0, "world isolation");
void BadCatalog(Action<Catalog> mutate)
{
    var candidate = source.Clone(); mutate(candidate);
    try { _ = new GameCatalog(candidate); throw new InvalidOperationException("Bad catalog accepted"); }
    catch (GameFault) { }
}
BadCatalog(c => c.Skills.Add(c.Skills[0].Clone()));
BadCatalog(c => c.Skills[0].FlowId = 987654);
BadCatalog(c => c.Skills[0].ManaCost = -1);
BadCatalog(c => c.Flows[0].Phases[1].Events[0].DamageScale = float.NaN);
BadCatalog(c => c.Npcs[0].Prefab = "../escape");
// A large tick carries its unused time across consecutive timeline phases.
var chained = source.Clone();
var custom = new SkillFlow { Id = 9998 };
foreach (int t in new[] { 100, 100 })
{
    var phase = new Phase { Kind = PhaseKind.Timeline };
    phase.Events.Add(new TimelineEvent { TimeMs = t, Action = TimelineAction.Hit, DamageScale = 1 });
    custom.Phases.Add(phase);
}
chained.Flows.Add(custom); chained.Skills.Add(new SkillDef { Id = 9998, FlowId = 9998 });
var chainWorld = new GameWorld(new GameCatalog(chained));
foreach (int id in new[] { 1, 2 }) chainWorld.Handle(new Request { Version = 1, CreateEntity = new CreateEntity { Id = id, MaxHp = 100, Attack = 10 } });
chainWorld.Handle(new Request { Version = 1, CastSkill = new CastSkill { SourceId = 1, TargetId = 2, SkillId = 9998 } });
Response chainResult = chainWorld.Handle(new Request { Version = 1, Tick = new Tick { DeltaMs = 250 } });
Near(chainResult.Entities[1].Hp, 80); Check(!chainResult.Entities[0].Casting, "timeline carryover");
Console.WriteLine($"PASS {count} protobuf exchanges with independent assertions; catalog validation, room isolation and timeline carryover");
