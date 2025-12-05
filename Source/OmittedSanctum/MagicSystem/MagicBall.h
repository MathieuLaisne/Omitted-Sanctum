// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicEntity.h"
#include "Components/SphereComponent.h"
#include "MagicBall.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API AMagicBall : public AMagicEntity
{
	GENERATED_BODY()
public:
	AMagicBall();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** Trigger volume for applying effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ball")
	USphereComponent* EffectVolume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float ExplosionRadius = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ball")
	float Speed = 3.0f;

protected:

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
