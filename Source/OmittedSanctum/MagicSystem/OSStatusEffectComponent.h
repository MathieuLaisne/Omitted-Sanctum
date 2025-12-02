#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagicHelper.h"
#include "OSStatusEffectComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMITTEDSANCTUM_API UOSStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOSStatusEffectComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Adds a new effect, checking for resistances and immunities
	UFUNCTION(BlueprintCallable, Category = "Magic|Status")
	void ApplyEffect(FOSEffectStrength NewEffect, float Duration);

	// Helpers for Gameplay Logic
	UFUNCTION(BlueprintCallable, Category = "Magic|Status")
	bool CanMove() const; // Checks for Root

	UFUNCTION(BlueprintCallable, Category = "Magic|Status")
	bool CanAttack() const; // Checks for Stun

	UFUNCTION(BlueprintCallable, Category = "Magic|Status")
	float GetIncomingDamageMultiplier() const; // Checks for Resistant

	UFUNCTION(BlueprintCallable, Category = "Magic|Status")
	float GetOutgoingDamageMultiplier() const; // Checks for Weak

	UFUNCTION(BlueprintCallable, Category = "Magic|Status")
	float GetSpeedMultiplier() const; // Checks for Slow

private:
	// Active effects and their remaining duration
	UPROPERTY(VisibleAnywhere, Category = "Magic|Status")
	TMap<TEnumAsByte<EOSMagicEffects>, float> ActiveEffects;

	// Cooldowns to prevent re-application (Cleanse mechanics)
	TMap<TEnumAsByte<EOSMagicEffects>, float> ImmunityCooldowns;

	// Timer for DoT ticks (accumulates DeltaTime)
	float DoT_Timer;
	const float DoT_Interval = 1.0f; // Apply DoT every 1 second

	void HandleDoT();
};