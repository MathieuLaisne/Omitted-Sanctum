// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MagicSystem/MagicEntity.h"
#include "MagicSystem/OSStatusEffectComponent.h"
#include "OSEnemy.generated.h"

UCLASS()
class OMITTEDSANCTUM_API AOSEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AOSEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	int MaxHP;
	int CurrentHP;
	int Strength;
	float AttackRange;
	float AttackSpeed;
	TArray<FOSMagicElementRelation> Resistances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UOSStatusEffectComponent* StatusComponent;

	UFUNCTION(BlueprintCallable)
	void Damage(int amount);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDamage(int HpLost);

	UFUNCTION(BlueprintImplementableEvent)
	void Die();

	UFUNCTION(BlueprintCallable)
	void ApplyMagicDamage(FString Element, int HitDamage, TArray<FOSEffectStrength> Effects, float effectDuration);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Attack();

};
