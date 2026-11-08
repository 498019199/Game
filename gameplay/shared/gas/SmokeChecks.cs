using System;
using System.Linq;
using Game.Protocol;

namespace Game.Combat
{
    public static class SmokeChecks
    {
        public static void Run(GameCatalog catalog, bool skills)
        {
            var world = new GameWorld(catalog);
            Response Send(Request request)
            {
                request.Version = 1;
                Response result = world.Handle(request);
                if (result.Status != ErrorCode.Ok) throw new GameFault(result.Status, result.Error);
                return result;
            }
            Send(new Request { CreateEntity = new CreateEntity { Id = 1, Name = "Attacker", MaxHp = 100, Attack = 10, MaxMana = 50 } });
            Send(new Request { CreateEntity = new CreateEntity { Id = 2, Name = "Defender", MaxHp = 100, Attack = 5, MaxMana = 50 } });
            if (skills)
            {
                Send(new Request { CastSkill = new CastSkill { SourceId = 1, TargetId = 2, SkillId = 1001 } });
                Response hit = Send(new Request { Tick = new Tick { DeltaMs = 200 } });
                Check(Math.Abs(hit.Entities.Single(e => e.Id == 2).Hp - 90) < 0.001f, "BasicStrike damage");
                Response end = Send(new Request { Tick = new Tick { DeltaMs = 100 } });
                Check(!end.Entities.Single(e => e.Id == 1).Casting, "BasicStrike completion");
                Send(new Request { SetAttribute = new SetAttribute { EntityId = 1, Attribute = AttributeId.Mana, Value = 5 } });
                Response fail = world.Handle(new Request { Version = 1, CastSkill = new CastSkill { SourceId = 1, TargetId = 2, SkillId = 1002 } });
                Check(fail.Status == ErrorCode.InsufficientMana, "ManaBolt mana gate");
            }
            else
            {
                var buff = new EffectSpec { DefinitionId = 1001, Duration = DurationPolicy.Timed, DurationMs = 500 };
                buff.Tags.Add("Buff.AtkUp");
                buff.Modifiers.Add(new Modifier { Attribute = AttributeId.Attack, Op = ModifierOp.PercentAdd, Value = 0.2f });
                Response applied = Send(new Request { ApplyEffect = new ApplyEffect { TargetId = 1, Effect = buff } });
                Check(Math.Abs(applied.Entities[0].Attack - 12) < 0.001f, "ATK buff");
                Send(new Request { Damage = new DealDamage { SourceId = 1, TargetId = 2, Amount = applied.Entities[0].Attack } });
                Response expired = Send(new Request { Tick = new Tick { DeltaMs = 500 } });
                Check(Math.Abs(expired.Entities[0].Attack - 10) < 0.001f && expired.Entities[1].Hp == 88,
                    "Buff expiry and damage");
            }
        }
        private static void Check(bool value, string name)
        { if (!value) throw new GameFault(ErrorCode.Internal, "Smoke failed: " + name); }
    }
}
