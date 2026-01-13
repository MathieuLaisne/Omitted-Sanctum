// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OSGameMode.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API AOSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	void PostLogin(APlayerController* NewPlayer) override;
};
