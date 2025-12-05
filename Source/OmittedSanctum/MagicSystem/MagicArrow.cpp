// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicArrow.h"
#include "OmittedSanctum/OSEnemy.h"

AMagicArrow::AMagicArrow()
{
	PrimaryActorTick.bCanEverTick = true;

	EffectVolume = CreateDefaultSubobject<USphereComponent>(TEXT("EffectVolume"));
	EffectVolume->SetupAttachment(RootComponent);
	EffectVolume->SetCollisionProfileName(TEXT("Trigger"));

	// Bind overlap events
	EffectVolume->OnComponentBeginOverlap.AddDynamic(this, &AMagicArrow::OnOverlapBegin);

}

void AMagicArrow::BeginPlay()
{
	Super::BeginPlay();
}

void AMagicArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector ForwardSpeed = GetActorForwardVector() * Speed;
	SetActorLocation(GetActorLocation() + ForwardSpeed);
}

void AMagicArrow::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AOSEnemy* HitEnemy = Cast<AOSEnemy>(OtherActor);
	if (HitEnemy && OtherActor != GetInstigator())
	{
		HitEnemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnHit, EffectDuration);
		PiercingAmount--;
		if (PiercingAmount == 0)
			Destroy();
	}
	else if (OtherActor != GetInstigator())
		Destroy();
}