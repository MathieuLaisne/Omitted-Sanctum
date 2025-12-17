// Fill out your copyright notice in the Description page of Project Settings.


#include "OSEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OSPlayerState.h"

// Sets default values
AOSEnemy::AOSEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatusComponent = CreateDefaultSubobject<UOSStatusEffectComponent>(TEXT("StatusEffects"));
	ResistanceComponent = CreateDefaultSubobject<UOSMagicResistanceComponent>(TEXT("MagicResistance"));
}

// Called when the game starts or when spawned
void AOSEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	
	if (GetCharacterMovement())
	{
		// Handling Root 
		if (!StatusComponent->CanMove())
		{
			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}
		else
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}
	}
}

// Called every frame
void AOSEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AOSEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AOSEnemy::ApplyMagicDamage(FString Element, int HitDamage, TArray<FOSEffectStrength> Effects, float effectDuration)
{
	EOSMagicResWeak Res = ResistanceComponent->FindResistance(Element);
	auto[chance, damageReduction] = ResistanceComponent->CalculateResistanceModifier(Res);

	damageReduction *= StatusComponent->GetIncomingDamageMultiplier();
	Damage(FMath::RoundToInt(HitDamage * damageReduction));

	for(FOSEffectStrength effect : Effects)
	{
		if (effect.strength - chance > FMath::RandRange(0,100))
		{
			StatusComponent->ApplyEffect(effect, effectDuration);
		}
		else
			break;
	}
}

void AOSEnemy::Damage(int amount)
{
	CurrentHP -= amount;
	if (CurrentHP <= 0)
		Die();
	else
		OnDamage(amount);
}

void AOSEnemy::Attack_Implementation()
{
	// Check Stun status 
	if (!StatusComponent->CanAttack()) return;

	// Apply Weak status logic
	float WeakMult = StatusComponent->GetOutgoingDamageMultiplier();

	//TODO: extend so the attacks can hit Magic Walls and other enemies.
	AOSPlayerState* CurPlayerState = Cast<AOSPlayerState>(GetPlayerState());
	if (CurPlayerState)
	{
		CurPlayerState->OSTakeDamage(FMath::RoundToInt(Strength * WeakMult));
	}
}
