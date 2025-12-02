// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MagicSystem/OSStatusEffectComponent.h"
#include "OSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API AOSPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void OSTakeDamage(int dmg);

	UFUNCTION(BlueprintImplementableEvent)
	void GameOver();

	UFUNCTION(BlueprintImplementableEvent)
	void OnTakingDamage();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int CurrentHP = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UOSStatusEffectComponent* StatusComponent;
};
