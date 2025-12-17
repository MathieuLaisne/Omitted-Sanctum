// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagicEntity.h"
#include "OSMagicResistanceComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OMITTEDSANCTUM_API UOSMagicResistanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOSMagicResistanceComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	EOSMagicResWeak FindResistance(FString Element);
	TPair<int, float> CalculateResistanceModifier(EOSMagicResWeak FoundResistance);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOSMagicElementRelation> Resistances;
};
