// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicEntity.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
#include "MagicWall.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API AMagicWall : public AMagicEntity
{
	GENERATED_BODY()

public:
	AMagicWall();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Visual representation and physical collision for solid walls */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	UStaticMeshComponent* WallMesh;

	/** Trigger volume for applying effects */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall")
	UBoxComponent* EffectVolume;

	/** Modifies the navigation mesh to influence AI pathfinding costs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UNavModifierComponent* NavModifier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Config")
	bool bIsSolidWall;

	/** Maximum health for solid walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Config", meta = (EditCondition = "bIsSolidWall"))
	float MaxHealth;

	/** Current health of the wall */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Config")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 1))
	float Duration = 30.0f;

	UFUNCTION(BlueprintCallable)
	void Initialize(bool IsSolid, int MaxHP);

protected:

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/** Helper to configure collision and NavArea based on bIsSolidWall */
	void UpdateWallState();

	/** Tracks actors currently inside the firewall for DoT application */
	TArray<AActor*> ActorsInsideWall;
	float TimeSinceLastTick;
	float TimeElapsed = 0.0f;
};
