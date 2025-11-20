// Fill out your copyright notice in the Description page of Project Settings.


#include "OSSpellbook.h"

// Sets default values
AOSSpellbook::AOSSpellbook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOSSpellbook::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOSSpellbook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UAbilitySystemComponent* AOSSpellbook::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

