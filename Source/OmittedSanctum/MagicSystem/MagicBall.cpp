// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicBall.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OmittedSanctum/OSEnemy.h"

AMagicBall::AMagicBall()
{
	PrimaryActorTick.bCanEverTick = true;

	EffectVolume = CreateDefaultSubobject<USphereComponent>(TEXT("EffectVolume"));
	EffectVolume->SetupAttachment(RootComponent);
	EffectVolume->SetCollisionProfileName(TEXT("Trigger"));

	// Bind overlap events
	EffectVolume->OnComponentBeginOverlap.AddDynamic(this, &AMagicBall::OnOverlapBegin);

}

void AMagicBall::BeginPlay()
{
	Super::BeginPlay();
}

void AMagicBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector ForwardSpeed = GetActorForwardVector() * Speed;
	SetActorLocation(GetActorLocation() + ForwardSpeed);
}

void AMagicBall::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TArray<AActor*> ActorsInExplosion;

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	objectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), ExplosionRadius, objectTypes, AActor::StaticClass(), { }, ActorsInExplosion);
	for (int i = ActorsInExplosion.Num() - 1; i >= 0; i--)
	{
		AOSEnemy* HitEnemy = Cast<AOSEnemy>(ActorsInExplosion[i]);
		HitEnemy->ApplyMagicDamage(Element.name, Damage, Element.CanApplyOnHit, EffectDuration);
	}
	Destroy();
}