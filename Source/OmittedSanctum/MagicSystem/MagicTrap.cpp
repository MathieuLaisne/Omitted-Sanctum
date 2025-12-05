// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicTrap.h"
#include "OmittedSanctum/OSEnemy.h"

AMagicTrap::AMagicTrap()
{
	PrimaryActorTick.bCanEverTick = true;

	EffectVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EffectVolume"));
	EffectVolume->SetupAttachment(RootComponent);
	EffectVolume->SetCollisionProfileName(TEXT("Trigger"));

	// Bind overlap events
	EffectVolume->OnComponentBeginOverlap.AddDynamic(this, &AMagicTrap::OnOverlapBegin);
	EffectVolume->OnComponentEndOverlap.AddDynamic(this, &AMagicTrap::OnOverlapEnd);

	// Defaults
	TimeSinceLastTick = 0.0f;
}

void AMagicTrap::BeginPlay()
{
	Super::BeginPlay();
}

void AMagicTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeElapsed += DeltaTime;
	if (TimeElapsed >= Duration)
		Destroy();

	if (ActorsInsideTrap.Num() > 0)
	{
		TimeSinceLastTick += DeltaTime;

		if (TimeSinceLastTick >= .2f) //every 20 milliseconds only
		{
			TimeSinceLastTick = 0.0f;
			// Iterate backwards to safely remove invalid actors
			for (int32 i = ActorsInsideTrap.Num() - 1; i >= 0; i--)
			{
				AActor* Target = ActorsInsideTrap[i];
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
					ActorsInsideTrap.RemoveAt(i);
				}
			}
		}
	}
}

void AMagicTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		// Add to list for DoT
		if (!ActorsInsideTrap.Contains(OtherActor))
		{
			ActorsInsideTrap.Add(OtherActor);
		}

		// Apply initial "OnHit" effect immediately upon entering
		AOSEnemy* Enemy = Cast<AOSEnemy>(OtherActor);
		if (Enemy)
		{
			Enemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnHit, EffectDuration);
		}
	}
}

void AMagicTrap::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ActorsInsideTrap.Remove(OtherActor);
	}
}