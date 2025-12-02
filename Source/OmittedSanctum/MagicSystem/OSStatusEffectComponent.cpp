#include "OSStatusEffectComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UOSStatusEffectComponent::UOSStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	DoT_Timer = 0.0f;
}

void UOSStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOSStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 1. Manage Active Effects Duration
	TArray<EOSMagicEffects> EffectsToRemove;

	for (auto& Elem : ActiveEffects)
	{
		Elem.Value -= DeltaTime; // Decrease duration

		if (Elem.Value <= 0.0f)
		{
			EffectsToRemove.Add(Elem.Key);

			// Setup Immunity Cooldowns as per GDD
			if (Elem.Key == EOSMagicEffects::Root)
			{
				// "cannot be reapplied for the next 5 seconds after it got cleansed" 
				ImmunityCooldowns.Add(EOSMagicEffects::Root, 5.0f);
			}
			else if (Elem.Key == EOSMagicEffects::Stun)
			{
				// "cannot be reapplied for the next 10 seconds after it got cleansed" 
				ImmunityCooldowns.Add(EOSMagicEffects::Stun, 10.0f);
			}
		}
	}

	// Remove expired effects
	for (EOSMagicEffects Effect : EffectsToRemove)
	{
		ActiveEffects.Remove(Effect);
	}

	// 2. Manage Immunity Cooldowns
	TArray<EOSMagicEffects> CooldownsToRemove;
	for (auto& Elem : ImmunityCooldowns)
	{
		Elem.Value -= DeltaTime;
		if (Elem.Value <= 0.0f)
		{
			CooldownsToRemove.Add(Elem.Key);
		}
	}
	for (EOSMagicEffects Effect : CooldownsToRemove)
	{
		ImmunityCooldowns.Remove(Effect);
	}

	// 3. Handle DoT Ticks (Burn, Poison, Electrified)
	DoT_Timer += DeltaTime;
	if (DoT_Timer >= DoT_Interval)
	{
		HandleDoT();
		DoT_Timer = 0.0f;
	}
}

void UOSStatusEffectComponent::ApplyEffect(FOSEffectStrength NewEffect, float Duration)
{
	if (NewEffect.effect == EOSMagicEffects::None) return;

	// Check Immunity (Cooldowns from previous cleanse)
	if (ImmunityCooldowns.Contains(NewEffect.effect))
	{
		return;
	}

	// Logic: If already active, refresh duration? Or keep longest? 
	// For now, we overwrite with new duration.
	ActiveEffects.Add(NewEffect.effect, Duration);
}

void UOSStatusEffectComponent::HandleDoT()
{
	int TotalDamage = 0;

	// GDD: Burn, Electrified, Poisoned are DoTs [cite: 179, 182, 183]
	// Note: You need to decide how much damage a DoT does. 
	// I am assuming a base value here, or you could pass damage in.
	const int BaseDotDamage = 5;

	if (ActiveEffects.Contains(EOSMagicEffects::DoT)) // Generic DoT map to Burn/Poison if needed
	{
		TotalDamage += BaseDotDamage;
	}

	// Since your enum has explicit "DoT" but GDD mentions "Burn" "Poisoned" etc,
	// ensure your Enum covers specific types or you map them to DoT. 
	// Assuming EOSMagicEffects::DoT covers all for now based on your provided header.

	if (TotalDamage > 0)
	{
		// Apply damage to owner
		UGameplayStatics::ApplyDamage(GetOwner(), TotalDamage, nullptr, nullptr, UDamageType::StaticClass());
	}
}

bool UOSStatusEffectComponent::CanMove() const
{
	// "Root: Disable movement" 
	return !ActiveEffects.Contains(EOSMagicEffects::Root);
}

bool UOSStatusEffectComponent::CanAttack() const
{
	// "Stun: Disable attacks" 
	return !ActiveEffects.Contains(EOSMagicEffects::Stun);
}

float UOSStatusEffectComponent::GetIncomingDamageMultiplier() const
{
	// "Resistant: reduce the damage taken by 10%"
	if (ActiveEffects.Contains(EOSMagicEffects::Resistant))
	{
		return 0.9f;
	}
	return 1.0f;
}

float UOSStatusEffectComponent::GetOutgoingDamageMultiplier() const
{
	// "Weak: Reduces the strength by 10%"
	if (ActiveEffects.Contains(EOSMagicEffects::Weak))
	{
		return 0.9f;
	}
	return 1.0f;
}

float UOSStatusEffectComponent::GetSpeedMultiplier() const
{
	// "Slow: reduce movement speed by 10%"
	if (ActiveEffects.Contains(EOSMagicEffects::Slow))
	{
		return 0.9f;
	}
	return 1.0f;
}