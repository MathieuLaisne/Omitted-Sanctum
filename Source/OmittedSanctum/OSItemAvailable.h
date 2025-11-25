// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Items/Item.h"
#include "OSItemAvailable.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct OMITTEDSANCTUM_API FOSItemAvailable : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AItem> ItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EOSItemType> type;
};
