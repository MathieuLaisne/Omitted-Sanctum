// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicEntity.h"
#include "NiagaraComponent.h"

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

