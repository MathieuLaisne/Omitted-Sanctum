// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicEntity.h"
#include "Components/CapsuleComponent.h"
#include "MagicTrap.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API AMagicTrap : public AMagicEntity
{
	GENERATED_BODY()
	
public:
	AMagicTrap();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Trigger volume for applying effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap")
	UCapsuleComponent* EffectVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 1))
	float Duration = 30.0f;

protected:

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	TArray<AActor*> ActorsInsideTrap;
	float TimeSinceLastTick;
	float TimeElapsed = 0.0f;

};
