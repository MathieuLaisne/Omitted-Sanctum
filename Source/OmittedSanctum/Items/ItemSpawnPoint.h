// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.h"

#include "Components/MaterialBillboardComponent.h"
#include "ItemSpawnPoint.generated.h"

/**
* the chances this point has any type of items if it were to generate an item.Must add up to 1.
*/
USTRUCT(BlueprintType)
struct FItemChances {
	GENERATED_USTRUCT_BODY()

public:
	FItemChances() : LoreChance(1), ConsumableChance(0), WeaponChance(0), SpellbookChance(0) {};
	FItemChances(float L, float C, float W, float S) : LoreChance(L), ConsumableChance(C), WeaponChance(W), SpellbookChance(S) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoreChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumableChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeaponChance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpellbookChance;
};

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OMITTEDSANCTUM_API UItemSpawnPoint : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UItemSpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialBillboardComponent* billboard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemChances chances;

	/** When set to true, this spawn point will never count in the room's maximum rewards*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool AlwaysUsed = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	EOSItemType GetItemType(int32 chance);

};