// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MagicEntity.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/SphereComponent.h"
#include <OmittedSanctum/OSEnemy.h>
#include "MagicWhip.generated.h"

/**
 * A projectile that latches onto enemies and allows the player to manipulate their position
 * using physics.
 */
UCLASS()
class OMITTEDSANCTUM_API AMagicWhip : public AMagicEntity
{
	GENERATED_BODY()

public:
	AMagicWhip();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** Collision component for the projectile head */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whip")
	USphereComponent* SphereComponent;

	/** Movement component for the initial flight */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whip")
	UProjectileMovementComponent* ProjectileMovement;

	/** The component that handles the physics gripping logic */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whip")
	UPhysicsHandleComponent* PhysicsHandle;

	/** How far in front of the player to hold the captured target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Whip", meta = (DisplayPriority = 1))
	float HoldDistance = 500.0f;

	/** Stiffness of the physics handle (how tightly it holds the target) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Whip", meta = (DisplayPriority = 1))
	float Stiffness = 1500.0f;

	/** Damping of the physics handle (reduces oscillation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Whip", meta = (DisplayPriority = 1))
	float Damping = 100.0f;

	/** How long to interpolate the grab (smoothness) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Whip", meta = (DisplayPriority = 1))
	float InterpolationSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 1))
	float Duration = 30.0f;

	UPROPERTY(BlueprintReadOnly)
	AOSEnemy* LatchedEnemy;

protected:

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	bool bIsLatched = false;
	float TimeElapsed = 0.0f;

	UPROPERTY()
	UPrimitiveComponent* GrabbedComponent;
};