// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicWall.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "NavAreas/NavArea_Null.h"
#include "Components/StaticMeshComponent.h"
#include "OmittedSanctum/OSEnemy.h"


AMagicWall::AMagicWall()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Setup Visual/Collision Mesh
	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	WallMesh->SetupAttachment(RootComponent);
	WallMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// 2. Setup Trigger Volume (for Firewall mode)
	EffectVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EffectVolume"));
	EffectVolume->SetupAttachment(WallMesh);
	EffectVolume->SetCollisionProfileName(TEXT("Trigger"));

	// Bind overlap events
	EffectVolume->OnComponentBeginOverlap.AddDynamic(this, &AMagicWall::OnOverlapBegin);
	EffectVolume->OnComponentEndOverlap.AddDynamic(this, &AMagicWall::OnOverlapEnd);

	// 3. Setup Nav Modifier for AI
	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));

	// Defaults
	bIsSolidWall = false; // Default to damaging type
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	TimeSinceLastTick = 0.0f;
}

void AMagicWall::BeginPlay()
{
	Super::BeginPlay();
	UpdateWallState();
}

void AMagicWall::UpdateWallState()
{
	if (bIsSolidWall)
	{
		// -- Solid Mode (Earth/Ice) --
		// Blocks physics, blocks Navigation
		WallMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WallMesh->SetCollisionProfileName(TEXT("BlockAll"));

		// Set NavModifier to Obstacle (AI cannot path through it)
		NavModifier->SetAreaClass(UNavArea_Obstacle::StaticClass());
	}
	else
	{
		// Ensure the trigger volume is active
		EffectVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		EffectVolume->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}

void AMagicWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeElapsed += DeltaTime;
	if (TimeElapsed >= Duration)
		Destroy();

	if (ActorsInsideWall.Num() > 0)
	{
		TimeSinceLastTick += DeltaTime;

		if (TimeSinceLastTick >= .2f) //every 20 milliseconds only
		{
			TimeSinceLastTick = 0.0f;

			// Iterate backwards to safely remove invalid actors
			for (int32 i = ActorsInsideWall.Num() - 1; i >= 0; i--)
			{
				AActor* Target = ActorsInsideWall[i];
				if (Target && Target->IsValidLowLevel())
				{
					AOSEnemy* Enemy = Cast<AOSEnemy>(Target);
					if (Enemy)
					{
						Enemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnTick, EffectDuration);
					}
				}
				else
				{
					ActorsInsideWall.RemoveAt(i);
				}
			}
		}
	}
}

void AMagicWall::Initialize(bool IsSolid, int MaxHP)
{
	bIsSolidWall = IsSolid;
	MaxHealth = MaxHP;
	CurrentHealth = MaxHealth;
	UpdateWallState();
}

void AMagicWall::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsSolidWall && OtherActor && OtherActor != this)
	{
		// Add to list for DoT
		if (!ActorsInsideWall.Contains(OtherActor))
		{
			ActorsInsideWall.Add(OtherActor);
		}

		// Apply initial "OnHit" effect immediately upon entering
		AOSEnemy* Enemy = Cast<AOSEnemy>(OtherActor);
		if (Enemy)
		{
			Enemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnHit, EffectDuration);
		}
	}
}

void AMagicWall::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!bIsSolidWall && OtherActor)
	{
		ActorsInsideWall.Remove(OtherActor);
	}
}

float AMagicWall::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Only Solid walls take structural damage
	if (bIsSolidWall && CurrentHealth > 0)
	{
		float mult = StatusComponent->GetIncomingDamageMultiplier();

		CurrentHealth -= ActualDamage * mult;

		if (CurrentHealth <= 0)
		{
			// Wall Destroyed
			Destroy();
		}
	}

	return ActualDamage;
}