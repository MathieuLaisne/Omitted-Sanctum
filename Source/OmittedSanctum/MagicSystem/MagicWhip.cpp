// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicWhip.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMagicWhip::AMagicWhip()
{
	// 1. Setup Collision
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	// Attach to the RootComponent defined in MagicEntity (which is a SceneComponent)
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->InitSphereRadius(20.0f);

	// Configure collision to hit Pawns and WorldDynamic objects
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	// Register Hit Event
	SphereComponent->OnComponentHit.AddDynamic(this, &AMagicWhip::OnHit);

	// 2. Setup Projectile Movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = SphereComponent;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// 3. Setup Physics Handle
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->LinearDamping = Damping;
	PhysicsHandle->LinearStiffness = Stiffness;
	PhysicsHandle->InterpolationSpeed = InterpolationSpeed;
}

void AMagicWhip::BeginPlay()
{
	Super::BeginPlay();

	// Ensure the handle uses our configured values
	PhysicsHandle->LinearDamping = Damping;
	PhysicsHandle->LinearStiffness = Stiffness;
	PhysicsHandle->InterpolationSpeed = InterpolationSpeed;

	//TODO: Activate once we have a proper beam effect
	/*FAttachmentTransformRules AttachmentInfo = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false);
	AttachToActor(GetWorld()->GetFirstPlayerController()->GetPawn(), AttachmentInfo, "Hand");*/
}

void AMagicWhip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeElapsed += DeltaTime;
	if (TimeElapsed >= Duration)
		Destroy();

	// If we have latched onto someone, update the target location based on Player's view
	if (bIsLatched && GetInstigator())
	{
		if (!LatchedEnemy)
		{
			bIsLatched = false;
			return;
		}

		FVector PlayerLoc;
		FRotator PlayerRot;

		// Get Player View Point (Camera location)
		GetInstigator()->GetController()->GetPlayerViewPoint(PlayerLoc, PlayerRot);

		// Calculate the point in space where the enemy should be dragged to
		FVector TargetLocation = PlayerLoc + (PlayerRot.Vector() * HoldDistance);

		// Update the Physics Handle goal
		PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, GetActorRotation());

		// Move the Whip actor itself to the target location so Niagara effects 
		// stay near the enemy. 
		// TODO: remove once we get a proper beam effect.
		SetActorLocation(TargetLocation);


		if(FMath::RoundToFloat(TimeElapsed) == FMath::FloorToFloat(TimeElapsed * 100)/100)
			LatchedEnemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnTick, EffectDuration);
	}

}

void AMagicWhip::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only process if we haven't latched yet and we hit a valid component
	if (bIsLatched || !OtherActor || !OtherComp)
	{
		return;
	}

	AOSEnemy* HitEnemy = Cast<AOSEnemy>(OtherActor);
	if (HitEnemy && OtherActor != GetInstigator() && !bIsLatched)
	{
		bIsLatched = true;

		// 1. Disable the projectile flight
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->ProjectileGravityScale = 0.0f; // Stop falling

		HitEnemy->GetCharacterMovement()->DisableMovement();

		// Detach controller (prevents AI from trying to rotate the capsule while flying)
		HitEnemy->DetachFromControllerPendingDestroy();

		// 3. Enable Physics Simulation on the mesh
		OtherComp->SetSimulatePhysics(true);
		OtherComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); // Ensure it collides with walls/other enemies

		// 4. Grab the component
		// We grab at the exact bone/location we hit
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			OtherComp,
			Hit.BoneName,
			Hit.ImpactPoint,
			OtherComp->GetComponentRotation()
		);

		GrabbedComponent = OtherComp;

		// Store this actor as the one we are holding
		LatchedEnemy = HitEnemy;

		LatchedEnemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnHit, EffectDuration);
	}
	else
	{
		// If we hit a wall/floor, just destroy the projectile
		Destroy();
	}
}

void AMagicWhip::Destroyed()
{
	// When the magic whip expires/is destroyed, release the enemy.
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		PhysicsHandle->ReleaseComponent();

		// Note: The enemy is left in a ragdoll state (Simulate Physics = true).
		// Because Physics Handle moves objects by applying forces/velocity, 
		// the enemy will naturally conserve the velocity they had the moment we released them.

		LatchedEnemy->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	}

	Super::Destroyed();
}