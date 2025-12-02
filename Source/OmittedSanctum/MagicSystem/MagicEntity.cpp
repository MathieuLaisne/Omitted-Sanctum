// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicEntity.h"
#include "NiagaraComponent.h"
#include <OmittedSanctum/OSPlayerState.h>
#include <Kismet/GameplayStatics.h>

// Sets default values
AMagicEntity::AMagicEntity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
	NiagaraComponent->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void AMagicEntity::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMagicEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagicEntity::SetMagicElement(FOSMagicElement elem)
{
	Element = elem;
	NiagaraComponent->SetAsset(elem.ElementEffect);
}

void AMagicEntity::ApplyEffectToCaster()
{
	TArray<FOSEffectStrength> Effects = Element.CanApplyToCaster;
	AOSPlayerState* PlayerState = Cast<AOSPlayerState>(UGameplayStatics::GetPlayerState(GetWorld(), 0));
	for (FOSEffectStrength Effect : Effects)
	{
		if (Effect.strength > FMath::RandRange(0, 100))
		{
			PlayerState->StatusComponent->ApplyEffect(Effect, EffectDuration);
		}
		else
			break;
	}
}

void AMagicEntity::ApplyEffectToSelf()
{
	TArray<FOSEffectStrength> Effects = Element.CanApplyToSelf;
	for (FOSEffectStrength Effect : Effects)
	{
		if (Effect.strength > FMath::RandRange(0, 100))
		{
			StatusComponent->ApplyEffect(Effect, EffectDuration);
		}
		else
			break;
	}
}

