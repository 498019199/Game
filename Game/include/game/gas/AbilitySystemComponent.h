#pragma once

#include <game/GameApi.h>
#include <game/gas/AttributeSet.h>
#include <game/gas/EffectContainer.h>
#include <game/gas/EffectTypes.h>
#include <game/gas/GameplayTag.h>
#include <game/gas/SkillPipeline.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Gas
{

class GAME_API AbilitySystemComponent
{
public:
	AbilitySystemComponent();
	~AbilitySystemComponent();

	AbilitySystemComponent(AbilitySystemComponent const&) = delete;
	AbilitySystemComponent& operator=(AbilitySystemComponent const&) = delete;

	void SetDebugName(std::string_view name) { debug_name_ = std::string(name); }
	std::string const& DebugName() const noexcept { return debug_name_; }

	GameplayTagContainer& Tags() noexcept { return tags_; }
	GameplayTagContainer const& Tags() const noexcept { return tags_; }

	AttributeSet& Attributes() noexcept { return attributes_; }
	AttributeSet const& Attributes() const noexcept { return attributes_; }

	EffectContainer& Effects() noexcept { return effects_; }
	EffectContainer const& Effects() const noexcept { return effects_; }

	SkillPipelineRunner& Pipeline() noexcept { return pipeline_; }
	SkillPipelineRunner const& Pipeline() const noexcept { return pipeline_; }

	bool HasTag(std::string_view tag) const { return tags_.HasTag(tag); }
	float GetAttribute(AttributeId id) const { return attributes_.GetCurrent(id); }

	bool CanMove() const;
	bool CanCast() const;
	bool IsCasting() const { return pipeline_.Running(); }

	GasId ApplyEffect(EffectSpec const& spec);
	void StepEffects(TimeMs dt_ms);
	void StepPipeline(TimeMs dt_ms);
	void FlushEffects();

	void SpendMp(float amount);
	bool HasMp(float amount) const;

	void AddEventHandler(GasEventHandler handler);
	void Broadcast(GasEvent const& evt);

	void InitDefaultCombatStats(float max_hp, float atk, float max_mp);

private:
	std::string debug_name_;
	GameplayTagContainer tags_;
	AttributeSet attributes_;
	EffectContainer effects_;
	SkillPipelineRunner pipeline_;
	void Broadcast(GasEvent const& evt);

	void InitDefaultCombatStats(float max_hp, float atk, float max_mp);

private:
	std::string debug_name_;
	GameplayTagContainer tags_;
	AttributeSet attributes_;
	EffectContainer effects_;
	SkillPipelineRunner pipeline_;
	std::vector<GasEventHandler> handlers_;
};

} // namespace Gas
