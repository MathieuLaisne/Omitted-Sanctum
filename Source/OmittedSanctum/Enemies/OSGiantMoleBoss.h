#pragma once

#include "CoreMinimal.h"
#include "OSEnemy.h"
#include "OmittedSanctum/OSTypes.h"
#include "OSGiantMoleBoss.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class EGiantMoleState : uint8
{
	Idle,
	IntroCinematic,
	Burrowed,
	Charging,
	SurfaceAttack,
	Stunned,
	Dying
};

UENUM(BlueprintType)
enum class EGiantMoleSurfaceType : uint8
{
	Floor,
	Wall,
	Ceiling,
	None
};

/**
 * Giant Mole Boss - A blind, powerful boss that moves through any connected surface.
 */
UCLASS()
class OMITTEDSANCTUM_API AOSGiantMoleBoss : public AOSEnemy
{
	GENERATED_BODY()

public:
	AOSGiantMoleBoss();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss")
	EGiantMoleState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mole Boss")
	EGiantMoleSurfaceType CurrentSurfaceType;

	// --- Movement Settings ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Movement")
	float BurrowSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Movement")
	float FastMoveSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Movement")
	float ChargeSpeed = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Movement")
	float SurfaceDetectionRange = 300.0f;

	// --- Behaviour Settings ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Behaviour")
	float StunDuration = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Behaviour")
	float RandomAttackChance = 0.005f; // Per tick/interval chance

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Behaviour")
	float MinAttackInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Behaviour")
	float AttackWarningTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Behaviour")
	float IntroCinematicDuration = 3.0f;

	// --- Effects ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Effects")
	UNiagaraSystem* TrailFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Effects")
	float TrailLifeSpan = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Effects")
	float TrailSpawnInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Effects")
	UNiagaraSystem* BurrowedVFX;

	// --- Sound detection threshold ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mole Boss/Behaviour")
	EOSNoiseLevel SoundThreshold = EOSNoiseLevel::Noisy;

	// --- Functions ---

	UFUNCTION(BlueprintCallable, Category = "Mole Boss/Behaviour")
	void SetState(EGiantMoleState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Mole Boss/Effects")
	void OnStateChanged(EGiantMoleState NewState);

	UFUNCTION(BlueprintCallable, Category = "Mole Boss/Behaviour")
	void TriggerIntroCinematic();

	// Called when the mole hits something during a charge
	UFUNCTION(BlueprintCallable, Category = "Mole Boss/Behaviour")
	void OnChargeHit(AActor* HitActor);

protected:
	void HandleIntroCinematic(float DeltaTime);
	void HandleBurrowedMovement(float DeltaTime);
	void HandleCharging(float DeltaTime);
	void HandleSurfaceAttack(float DeltaTime);
	void HandleStunned(float DeltaTime);
	
	void DetectSurface();
	FVector ProjectMovementOnSurface(const FVector& Direction);
	void UpdateOrientation(float DeltaTime);

	UFUNCTION()
	void OnNoiseDetected(AActor* NoiseMaker, const FVector& Location, EOSNoiseLevel NoiseLevel);

	// Internal State
	FVector TargetLocation;
	FVector ChargeDirection;
	FVector CurrentSurfaceNormal;
	
	float StateTimer;
	float AttackTimer;
	float WanderTimer;
	FVector RandomWanderDirection;

	bool bHasTarget;
	bool bIsLoudNoiseReaction;

	UPROPERTY()
	UNiagaraComponent* ActiveBurrowedVFX;

	void PickNewWanderDirection();
	void SpawnTrailEffect();

	float TrailSpawnTimer;
};
