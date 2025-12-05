// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicEntity.h"
#include "Components/SphereComponent.h"
#include "MagicArrow.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API AMagicArrow : public AMagicEntity
{
	GENERATED_BODY()

public:
	AMagicArrow();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** Trigger volume for applying effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow")
	USphereComponent* EffectVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	int PiercingAmount = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	float Speed = 3.0f;

protected:

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
