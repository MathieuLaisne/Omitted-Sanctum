// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OSItemBoxWidget.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API UOSItemBoxWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateInfoBox(const FString& ItemName, const FString& ItemDescription);
};
