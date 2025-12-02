// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "MagicHelper.generated.h"

UENUM(BlueprintType)
enum EOSMagicResWeak
{
	/*
	* +/-5
	*/
	Minor					UMETA(DisplayName = "Minor"),
	/*
	* +/-10
	*/
	Small					UMETA(DisplayName = "Small"),
	/*
	* +/-25
	*/
	Moderate			UMETA(DisplayName = "Moderate"),
	/*
	* +/-50
	*/
	Great					UMETA(DisplayName = "Great"),
	/*
	* 0
	*/
	Zero					UMETA(DisplayName = "Zero")
};

UENUM(BlueprintType)
enum EOSMagicEffects
{
	/*
	* No effect, shouldn't be used normally.
	*/
	None					UMETA(DisplayName = "None"),
	/*
	* Affected cannot walk/jump.
	*/
	Root					UMETA(DisplayName = "Root"),
	/*
	* Reduce movement speed.
	*/
	Slow					UMETA(DisplayName = "Slow"),
	/*
	* Inflicts Damage Over Time.
	*/
	DoT						UMETA(DisplayName = "DoT"),
	/*
	* Affected takes less damage.
	*/
	Resistant			UMETA(DisplayName = "Resistant"),
	/*
	* Affected cannot attack.
	*/
	Stun					UMETA(DisplayName = "Stun"),
	/*
	* Affected deals less damage
	*/
	Weak				UMETA(DisplayName = "Weak")
};

USTRUCT(BlueprintType)
struct OMITTEDSANCTUM_API FOSEffectStrength
{
	GENERATED_USTRUCT_BODY()

public:
	FOSEffectStrength() : effect(EOSMagicEffects::None), strength(0) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EOSMagicEffects> effect;

	/*
	* Value is percentage of chance to get applied if no resistance.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 100))
	int strength;

	bool operator==(const FOSEffectStrength& Other) const
	{
		return Equals(Other);
	}

	bool Equals(const FOSEffectStrength& Other) const
	{
		return effect == Other.effect && strength == Other.strength;
	}
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FOSEffectStrength& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FOSEffectStrength& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FOSEffectStrength));
	return Hash;
}
#endif

USTRUCT(BlueprintType)
struct OMITTEDSANCTUM_API FOSMagicElement : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FOSMagicElement() : name("New") {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOSEffectStrength> CanApplyToSelf = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOSEffectStrength> CanApplyToCaster = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOSEffectStrength> CanApplyOnHit = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOSEffectStrength> CanApplyOnTick = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* ElementEffect;
};

USTRUCT(BlueprintType)
struct OMITTEDSANCTUM_API FOSMagicElementRelation
{
	GENERATED_USTRUCT_BODY()

public:
	FOSMagicElementRelation() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ElementName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EOSMagicResWeak> Relation;

};
