using System;
using System.Collections.Generic;
using System.Linq;
using Game.Protocol;

namespace Game.Combat
{
    internal sealed class ActiveEffect
    {
        public ulong Id;
        public EffectSpec Spec = new EffectSpec();
        public long Remaining, PeriodElapsed;
        public bool Expired;
    }
    internal sealed class Entity
    {
        public readonly int Id;
        public readonly string Name;
        private readonly float[] bases = new float[6];
        public float Hp;
        public readonly HashSet<string> ManualTags = new HashSet<string>(StringComparer.Ordinal);
        public readonly List<ActiveEffect> Effects = new List<ActiveEffect>();
        public SkillCast? Cast;
        public Entity(CreateEntity definition)
        {
            Id = definition.Id; Name = definition.Name;
            bases[(int)AttributeId.MaxHp] = Hp = definition.MaxHp;
            bases[(int)AttributeId.Attack] = definition.Attack;
            bases[(int)AttributeId.MaxMana] = bases[(int)AttributeId.Mana] = definition.MaxMana;
        }
        public float Attribute(AttributeId attribute)
        {
            if (attribute == AttributeId.Hp) return Hp;
            double add = 0, percent = 0;
            foreach (ActiveEffect effect in Effects)
                foreach (Modifier modifier in effect.Spec.Modifiers)
                    if (modifier.Attribute == attribute)
                    {
                        if (modifier.Op == ModifierOp.Add) add += modifier.Value;
                        else percent += modifier.Value;
                    }
            return (float)Math.Max(0, (bases[(int)attribute] + add) * (1 + percent));
        }
        public void SetBase(AttributeId attribute, float value)
        {
            if (attribute == AttributeId.Hp) Hp = value;
            else bases[(int)attribute] = value;
            ClampHp();
        }
        public void SpendMana(float value)
        {
            // Preserve the former AttributeSet/SpendMp convention (current to base).
            bases[(int)AttributeId.Mana] = Math.Max(0, Attribute(AttributeId.Mana) - value);
        }
        public void ClampHp() => Hp = Math.Max(0, Math.Min(Hp, Attribute(AttributeId.MaxHp)));
        public IEnumerable<string> Tags()
        {
            var tags = new SortedSet<string>(ManualTags, StringComparer.Ordinal);
            foreach (var effect in Effects) foreach (string tag in effect.Spec.Tags) tags.Add(tag);
            if (Cast != null) tags.Add("Ability.Active");
            return tags;
        }
        public bool HasTag(string query) => Tags().Any(t => t == query || t.StartsWith(query + ".", StringComparison.Ordinal));
        public bool CanMove => Hp > 0 && !HasTag("State.Stunned") && !HasTag("State.Rooted");
        public bool CanCast => Hp > 0 && !HasTag("State.Stunned") && !HasTag("State.Silenced");
        public EntitySnapshot Snapshot()
        {
            var snapshot = new EntitySnapshot { Id = Id, Name = Name, Hp = Hp,
                MaxHp = Attribute(AttributeId.MaxHp), Attack = Attribute(AttributeId.Attack),
                Mana = Attribute(AttributeId.Mana), MaxMana = Attribute(AttributeId.MaxMana),
                Casting = Cast != null, SkillId = Cast?.Skill.Id ?? 0, CanMove = CanMove, CanCast = CanCast };
            snapshot.Tags.Add(Tags());
            foreach (var effect in Effects)
                snapshot.Effects.Add(new EffectSnapshot { InstanceId = effect.Id,
                    DefinitionId = effect.Spec.DefinitionId, RemainingMs = effect.Remaining });
            return snapshot;
        }
    }
}
