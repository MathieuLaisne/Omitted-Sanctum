// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "MagicEntity.generated.h"

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
	Great					UMETA(DisplayName = "Great")
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
};

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

UCLASS()
class OMITTEDSANCTUM_API AMagicEntity : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicEntity();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOSMagicElement Element;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
