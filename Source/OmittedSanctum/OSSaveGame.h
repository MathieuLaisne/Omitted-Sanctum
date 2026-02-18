// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "OSTypes.h"
#include "OSSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API UOSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
  UPROPERTY(VisibleAnywhere, Category = "Basic")
  FString SaveSlotName = "CurrentRun";

  UPROPERTY(VisibleAnywhere, Category = "Basic")
  uint32 UserIndex = 0;

  UPROPERTY(VisibleAnywhere, Category = "Meta")
  TMap<EOSPlayerClass, FOSClassMetaProgress> ClassProgressMap;

  FOSClassMetaProgress& GetProgressFor(EOSPlayerClass ClassType)
  {
    return ClassProgressMap.FindOrAdd(ClassType);
  }

  // --- Player State ---
  UPROPERTY(VisibleAnywhere, Category = "Player")
  EOSPlayerClass PlayerClass;

  UPROPERTY(VisibleAnywhere, Category = "Player")
  int32 CurrentHP;

  UPROPERTY(VisibleAnywhere, Category = "Player")
  int32 MaxHP;

  UPROPERTY(VisibleAnywhere, Category = "Player")
  FRoomPosition PlayerPosition;

  UPROPERTY(VisibleAnywhere, Category = "Player")
  TArray<FSaveItem> InventoryItems;

  // --- Dungeon State ---
  UPROPERTY(VisibleAnywhere, Category = "Run")
  bool bHasActiveRun;

  UPROPERTY(VisibleAnywhere, Category = "Run")
  int32 CurrentFloorIndex;

  UPROPERTY(VisibleAnywhere, Category = "Map")
  TMap<int32, FFloorSaveData> FloorHistory;

  // Helper to get current floor data easily
  FFloorSaveData& GetCurrentFloorData()
  {
    return FloorHistory.FindOrAdd(CurrentFloorIndex);
  }
};
