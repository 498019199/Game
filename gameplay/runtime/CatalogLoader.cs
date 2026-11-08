using System;
using System.IO;
using System.Text.Json;
using Game.Combat;
using Game.Protocol;
using Google.Protobuf;

namespace Game.Runtime;

public static class CatalogLoader
{
    // JSON is an authoring format only. Runtime communication/config uses protobuf.
    public static Catalog FromJsonDirectory(string directory)
    {
        var catalog = new Catalog();
        using var flows = JsonDocument.Parse(File.ReadAllText(Path.Combine(directory, "skill_flows.json")));
        foreach (JsonElement row in flows.RootElement.EnumerateArray())
        {
            var flow = new SkillFlow { Id = row.GetProperty("id").GetInt32(), Name = Text(row, "name") };
            foreach (JsonElement entry in row.GetProperty("phases").EnumerateArray())
            {
                string kind = Text(entry, "type");
                if (kind == "WaitUntil") throw new InvalidDataException("WaitUntil is not implemented");
                if (kind != "RulePlan" && kind != "Timeline") throw new InvalidDataException("Unknown phase " + kind);
                var phase = new Phase { Kind = kind == "RulePlan" ? PhaseKind.RulePlan : PhaseKind.Timeline,
                    CommitCost = !entry.TryGetProperty("commit_cost", out var cost) || cost.GetBoolean() };
                if (phase.Kind == PhaseKind.Timeline)
                    foreach (JsonElement ev in entry.GetProperty("events").EnumerateArray())
                    {
                        TimelineAction action = Text(ev, "action") switch
                        { "Cue" => TimelineAction.Cue, "EmitHit" => TimelineAction.Hit, "End" => TimelineAction.End,
                          _ => throw new InvalidDataException("Unknown timeline action") };
                        phase.Events.Add(new TimelineEvent { TimeMs = ev.GetProperty("t_ms").GetInt32(), Action = action,
                            Cue = Text(ev, "cue"), DamageScale = ev.TryGetProperty("damage_scale", out var scale) ? scale.GetSingle() : 1 });
                    }
                flow.Phases.Add(phase);
            }
            catalog.Flows.Add(flow);
        }
        using var skills = JsonDocument.Parse(File.ReadAllText(Path.Combine(directory, "skills.json")));
        foreach (JsonElement row in skills.RootElement.EnumerateArray())
        {
            var skill = new SkillDef { Id = row.GetProperty("id").GetInt32(), Name = Text(row, "name"),
                Slot = row.GetProperty("slot").GetInt32(), FlowId = row.GetProperty("cast_flow_id").GetInt32(),
                ManaCost = row.GetProperty("cost_mp").GetSingle(), CooldownMs = row.GetProperty("cooldown_ms").GetInt32() };
            if (row.TryGetProperty("blocked_by_tags", out var tags))
                foreach (JsonElement tag in tags.EnumerateArray()) skill.BlockedByTags.Add(tag.GetString() ?? "");
            catalog.Skills.Add(skill);
        }
        using var npcs = JsonDocument.Parse(File.ReadAllText(Path.Combine(directory, "npc.json")));
        foreach (JsonElement row in npcs.RootElement.EnumerateArray())
            catalog.Npcs.Add(new NpcDef { Id = row.GetProperty("id").GetInt32(), Prefab = Text(row, "prefab") });
        _ = new GameCatalog(catalog); // Validate atomically before producing deployable bytes.
        return catalog;
    }
    public static GameCatalog Read(string path) => new GameCatalog(Catalog.Parser.ParseFrom(File.ReadAllBytes(path)));
    private static string Text(JsonElement row, string key) => row.TryGetProperty(key, out var value) ? value.GetString() ?? "" : "";
}
