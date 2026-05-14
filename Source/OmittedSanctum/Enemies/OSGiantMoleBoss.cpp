#include "OSGiantMoleBoss.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "OmittedSanctum/OSGameInstance.h"

AOSGiantMoleBoss::AOSGiantMoleBoss()
{
	PrimaryActorTick.bCanEverTick = true;

	CurrentState = EGiantMoleState::Idle;
	CurrentSurfaceType = EGiantMoleSurfaceType::None;
	
	bHasTarget = false;
	bIsLoudNoiseReaction = false;
	
	AttackTimer = MinAttackInterval;
	WanderTimer = 0.0f;
	TrailSpawnTimer = 0.0f;
	TrailLifeSpan = 2.0f;
	TrailSpawnInterval = 0.1f;
}

void AOSGiantMoleBoss::BeginPlay()
{
	Super::BeginPlay();

	if (UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance()))
	{
		GI->OnNoiseMade.AddDynamic(this, &AOSGiantMoleBoss::OnNoiseDetected);
	}

	// Initialize burrowed VFX component but keep it inactive/hidden if not burrowed
	if (BurrowedVFX)
	{
		ActiveBurrowedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(BurrowedVFX, GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, false);
		if (ActiveBurrowedVFX)
		{
			ActiveBurrowedVFX->SetVisibility(false);
		}
	}
}

void AOSGiantMoleBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case EGiantMoleState::IntroCinematic:
		HandleIntroCinematic(DeltaTime);
		break;
	case EGiantMoleState::Burrowed:
		HandleBurrowedMovement(DeltaTime);
		break;
	case EGiantMoleState::Charging:
		HandleCharging(DeltaTime);
		break;
	case EGiantMoleState::SurfaceAttack:
		HandleSurfaceAttack(DeltaTime);
		break;
	case EGiantMoleState::Stunned:
		HandleStunned(DeltaTime);
		break;
	default:
		break;
	}
}

void AOSGiantMoleBoss::SetState(EGiantMoleState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	StateTimer = 0.0f;

	OnStateChanged(NewState);

	// Reset physics and movement based on state
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	
	// Default VFX visibility
	if (ActiveBurrowedVFX)
	{
		ActiveBurrowedVFX->SetVisibility(NewState == EGiantMoleState::Burrowed || NewState == EGiantMoleState::Charging);
	}

	switch(NewState)
	{
		case EGiantMoleState::Burrowed:
		case EGiantMoleState::Charging:
			MoveComp->StopMovementImmediately();
			MoveComp->SetMovementMode(MOVE_Flying);
			break;
		case EGiantMoleState::Stunned:
			StateTimer = StunDuration;
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
			MoveComp->SetMovementMode(MOVE_Falling); 
			break;
		case EGiantMoleState::SurfaceAttack:
			StateTimer = AttackWarningTime;
			MoveComp->StopMovementImmediately();
			break;
		case EGiantMoleState::IntroCinematic:
			StateTimer = IntroCinematicDuration;
			break;
		default:
			break;
	}
}

void AOSGiantMoleBoss::TriggerIntroCinematic()
{
	if (CurrentState == EGiantMoleState::Idle)
	{
		SetState(EGiantMoleState::IntroCinematic);
	}
}

void AOSGiantMoleBoss::HandleIntroCinematic(float DeltaTime)
{
	StateTimer -= DeltaTime;
	
	// cinematic-ish thing which ends with the mole burrowing itself inside the floor.
	if (StateTimer <= 0)
	{
		// Force find floor surface
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), GetActorLocation() + FVector::DownVector * 1000.0f, ECC_Visibility, Params))
		{
			SetActorLocation(Hit.Location + Hit.ImpactNormal * 10.0f);
			CurrentSurfaceNormal = Hit.ImpactNormal;
			CurrentSurfaceType = EGiantMoleSurfaceType::Floor;
		}
		
		SetState(EGiantMoleState::Burrowed);
	}
}

void AOSGiantMoleBoss::HandleBurrowedMovement(float DeltaTime)
{
	DetectSurface();
	UpdateOrientation(DeltaTime);

	float CurrentMoveSpeed = bIsLoudNoiseReaction ? FastMoveSpeed : BurrowSpeed;

	if (bHasTarget)
	{
		if(CurrentSurfaceType == EGiantMoleSurfaceType::Wall && FVector::DistSquared(GetActorLocation(), TargetLocation) < 5000.0f)
		{
			ChargeDirection = CurrentSurfaceNormal;
			bIsLoudNoiseReaction = false;
			bHasTarget = false;
			SetState(EGiantMoleState::Charging);
			return;
		}

		FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
		FVector ProjectedDir = ProjectMovementOnSurface(Direction);

		AddMovementInput(ProjectedDir, CurrentMoveSpeed * DeltaTime);

		// If we reached target (and we were moving fast to attack)
		if (bIsLoudNoiseReaction && FVector::DistSquared(GetActorLocation(), TargetLocation) < 10000.0f) // 1 meter
		{
			if (CurrentSurfaceType == EGiantMoleSurfaceType::Floor || CurrentSurfaceType == EGiantMoleSurfaceType::Ceiling)
			{
				SetState(EGiantMoleState::SurfaceAttack);
				bIsLoudNoiseReaction = false;
				bHasTarget = false;
			}
		}
		
		// Reset target if we wander for too long or reach it
		if (FVector::DistSquared(GetActorLocation(), TargetLocation) < 2500.0f)
		{
			bHasTarget = false;
			bIsLoudNoiseReaction = false;
		}
	}
	else
	{
		// Move randomly
		WanderTimer -= DeltaTime;
		if (WanderTimer <= 0)
		{
			PickNewWanderDirection();
		}

		FVector ProjectedDir = ProjectMovementOnSurface(RandomWanderDirection);
		AddMovementInput(ProjectedDir, BurrowSpeed * DeltaTime);
	}

	// Random attacks in floor/roof
	if (CurrentSurfaceType == EGiantMoleSurfaceType::Floor || CurrentSurfaceType == EGiantMoleSurfaceType::Ceiling)
	{
		AttackTimer -= DeltaTime;
		if (AttackTimer <= 0)
		{
			if (FMath::FRand() < RandomAttackChance)
			{
				// Audio cue is handled in SetState/SurfaceAttack logic or via BlueprintEvent
				SetState(EGiantMoleState::SurfaceAttack);
				AttackTimer = MinAttackInterval;
			}
		}
	}

	SpawnTrailEffect();
}

void AOSGiantMoleBoss::HandleCharging(float DeltaTime)
{

	// Straight line charge
	FVector MoveDelta = ChargeDirection * ChargeSpeed * DeltaTime;
	FVector Start = GetActorLocation();
	FVector End = Start + MoveDelta;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> SweepHits;
	bool bHitFound = false;

	FCollisionShape SweepShape = FCollisionShape::MakeCapsule(50.0f, 0.5f);

	if (GetWorld()->SweepMultiByChannel(SweepHits, Start, End, GetActorQuat(), ECC_Pawn, SweepShape, Params))
	{
		for (const FHitResult& SweepHit : SweepHits)
		{
			// Ignore initial overlaps with the surface we are charging away from
			// A start penetrating hit with a normal aligned with our charge direction is the surface behind us
			if (SweepHit.bStartPenetrating && FVector::DotProduct(SweepHit.ImpactNormal, ChargeDirection) > 0.2f)
			{
				continue;
			}

			Hit = SweepHit;
			bHitFound = true;
			break;
		}
	}

	if (bHitFound)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->IsA(ACharacter::StaticClass()))
		{
			// Deal damage and burrow back
			OnChargeHit(HitActor);
		}
		else
		{
			// Hit a wall? Stun
			OnChargeHit(nullptr);
		}
	}
	else
	{
		SetActorLocation(End);
		// Only detect surface (and snap to it) if we are not charging away from it
		// This allows the mole to "launch" off surfaces along the normal
		if (CurrentSurfaceType != EGiantMoleSurfaceType::None && FVector::DotProduct(ChargeDirection, CurrentSurfaceNormal) < 0.5f)
		{
			DetectSurface();
		}
		else 
		{
			// If we are charging away from the wall, we eventually lose the surface connection
			// We'll set it to None if we are far enough or immediately if we are specifically launching
			if (FVector::DotProduct(ChargeDirection, CurrentSurfaceNormal) > 0.5f)
			{
				CurrentSurfaceType = EGiantMoleSurfaceType::None;
			}
		}
	}
}

void AOSGiantMoleBoss::HandleSurfaceAttack(float DeltaTime)
{
	StateTimer -= DeltaTime;

	// TODO: Warning phase with audio cues
	if (StateTimer <= 0)
	{
		Attack(); // Native or BP event
		
		// Burrow back in
		SetState(EGiantMoleState::Burrowed);
		AttackTimer = MinAttackInterval;
	}
}

void AOSGiantMoleBoss::HandleStunned(float DeltaTime)
{
	StateTimer -= DeltaTime;
	if (StateTimer <= 0)
	{
		// After coming out of its stun, the mole will burrow back into the wall it hit.
		SetState(EGiantMoleState::Burrowed);
	}
}

void AOSGiantMoleBoss::OnChargeHit(AActor* HitActor)
{
	if (HitActor)
	{
		// Damage the player
		Attack();
		SetState(EGiantMoleState::Burrowed);
	}
	else
	{
		// Hit a wall
		SetState(EGiantMoleState::Stunned);
	}
}

void AOSGiantMoleBoss::DetectSurface()
{
	FVector Start = GetActorLocation();
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FVector> TraceDirs = {
		-CurrentSurfaceNormal, // Try current normal direction first
		FVector::DownVector, FVector::UpVector,
		FVector::ForwardVector, -FVector::ForwardVector,
		FVector::RightVector, -FVector::RightVector
	};

	FHitResult BestHit;
	float MinDist = SurfaceDetectionRange;

	for (const FVector& Dir : TraceDirs)
	{
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Dir * SurfaceDetectionRange, ECC_Visibility, Params))
		{
			if (Hit.Distance < MinDist)
			{
				MinDist = Hit.Distance;
				BestHit = Hit;
			}
		}
	}

	if (BestHit.IsValidBlockingHit())
	{
		CurrentSurfaceNormal = BestHit.ImpactNormal;
		
		// Snap to surface (with slight offset)
		SetActorLocation(BestHit.Location + CurrentSurfaceNormal * 10.0f);

		float DotUp = FVector::DotProduct(CurrentSurfaceNormal, FVector::UpVector);
		if (DotUp > 0.7f) CurrentSurfaceType = EGiantMoleSurfaceType::Floor;
		else if (DotUp < -0.7f) CurrentSurfaceType = EGiantMoleSurfaceType::Ceiling;
		else CurrentSurfaceType = EGiantMoleSurfaceType::Wall;

		// If we are burrowed and found a surface, ensure we are in flying mode to move along it
		if (CurrentState == EGiantMoleState::Burrowed)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}
	}
	else
	{
		// If no surface found, we might be falling or transitioning
		CurrentSurfaceType = EGiantMoleSurfaceType::None;

		// If we are burrowed but lost the surface, gravity should take over
		if (CurrentState == EGiantMoleState::Burrowed)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
}

void AOSGiantMoleBoss::UpdateOrientation(float DeltaTime)
{
	if (CurrentState == EGiantMoleState::Charging && CurrentSurfaceType == EGiantMoleSurfaceType::None)
	{
		// When charging in mid-air, point the nose towards the charge direction
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), ChargeDirection.Rotation(), DeltaTime, 10.0f));
		return;
	}

	if (CurrentSurfaceType == EGiantMoleSurfaceType::None) return;

	// Up vector is always pointing to the closest surface normal
	FVector Up = CurrentSurfaceNormal;

	// Target direction for orientation
	FVector OrientTarget;
	if (CurrentState == EGiantMoleState::Charging)
	{
		OrientTarget = ChargeDirection;
	}
	else if (bHasTarget)
	{
		OrientTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
	}
	else
	{
		OrientTarget = RandomWanderDirection;
	}

	FVector ProjectedTarget = FVector::VectorPlaneProject(OrientTarget, Up).GetSafeNormal();
	if (ProjectedTarget.IsNearlyZero())
	{
		ProjectedTarget = GetActorForwardVector();
	}

	FVector Right = ProjectedTarget;
	FVector Forward = FVector::CrossProduct(Right, Up).GetSafeNormal();
	
	FRotator TargetRotation = FRotationMatrix::MakeFromZY(Up, Right).Rotator();
	
	// Smooth rotation
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 10.0f));
}

FVector AOSGiantMoleBoss::ProjectMovementOnSurface(const FVector& Direction)
{
	if (CurrentSurfaceType == EGiantMoleSurfaceType::None) return Direction;
	return FVector::VectorPlaneProject(Direction, CurrentSurfaceNormal).GetSafeNormal();
}

void AOSGiantMoleBoss::OnNoiseDetected(AActor* NoiseMaker, const FVector& Location, EOSNoiseLevel NoiseLevel)
{
	if (NoiseMaker == this) return;
	
	bIsLoudNoiseReaction = (NoiseLevel == EOSNoiseLevel::Loud || NoiseLevel == EOSNoiseLevel::Noisy);
	
	// Only react if burrowed
	if (CurrentState == EGiantMoleState::Burrowed)
	{
		TargetLocation = Location;
		bHasTarget = true;

		if (CurrentSurfaceType == EGiantMoleSurfaceType::Wall)
		{
			// Project the sound location unto the wall the mole is on
			FVector MoleLoc = GetActorLocation();
			
			// Project sound location onto the plane of the wall (defined by mole position and wall normal)
			FVector ProjectedLoc = Location - FVector::DotProduct(Location - MoleLoc, CurrentSurfaceNormal) * CurrentSurfaceNormal;

			// Consider the mole's hitbox so that it cannot hit the floor/ceiling too early into its charge
			float CapsuleHalfHeight = 90.0f;
			if (UCapsuleComponent* Capsule = GetCapsuleComponent())
			{
				CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			}

			// Ensure minimum clearance from floor and ceiling
			float MinClearance = CapsuleHalfHeight + 10.0f; // Buffer
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);

			FHitResult FloorHit, CeilingHit;
			// Trace Down/Up to find floor/ceiling near the projected point
			if (GetWorld()->LineTraceSingleByChannel(FloorHit, ProjectedLoc, ProjectedLoc + FVector::DownVector * (MinClearance + 100.0f), ECC_Visibility, Params))
			{
				if (FloorHit.Distance < MinClearance)
				{
					ProjectedLoc.Z += (MinClearance - FloorHit.Distance);
				}
			}
			if (GetWorld()->LineTraceSingleByChannel(CeilingHit, ProjectedLoc, ProjectedLoc + FVector::UpVector * (MinClearance + 100.0f), ECC_Visibility, Params))
			{
				if (CeilingHit.Distance < MinClearance)
				{
					ProjectedLoc.Z -= (MinClearance - CeilingHit.Distance);
				}
			}

			TargetLocation = ProjectedLoc;
			bHasTarget = true;

			
		}
		// Floor/Ceiling reactions are handled in HandleBurrowedMovement (move fast then attack)
	}
}

void AOSGiantMoleBoss::PickNewWanderDirection()
{
	RandomWanderDirection = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f)).GetSafeNormal();
	WanderTimer = FMath::FRandRange(2.0f, 5.0f);
}

void AOSGiantMoleBoss::SpawnTrailEffect()
{
	if (!TrailFX) return;

	TrailSpawnTimer -= GetWorld()->GetDeltaSeconds();
	if (TrailSpawnTimer <= 0)
	{
		// Spawn trail effect on the surface
		UNiagaraComponent* TrailComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TrailFX, GetActorLocation(), GetActorRotation());
		
		if (TrailComp && TrailLifeSpan > 0.0f)
		{
			FTimerHandle TimerHandle;
			TWeakObjectPtr<UNiagaraComponent> WeakTrail = TrailComp;
			GetWorldTimerManager().SetTimer(TimerHandle, [WeakTrail]()
			{
				if (WeakTrail.IsValid())
				{
					WeakTrail->Deactivate();
				}
			}, TrailLifeSpan, false);
		}

		TrailSpawnTimer = TrailSpawnInterval;
	}
}
