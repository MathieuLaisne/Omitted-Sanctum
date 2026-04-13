// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OSTypes.h"
#include "OSSaveGame.h"
#include "OSGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMinimapUpdate);

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API UOSGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
  UOSGameInstance();

  UDataTable* METAPROGDT;

  UFUNCTION(BlueprintCallable, Category = "Game Flow")
  void SetSelectedClass(EOSPlayerClass NewClass);

  UFUNCTION(BlueprintCallable, Category = "Game Flow")
  EOSPlayerClass GetSelectedClass() const { return CurrentSelectedClass; }

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
  UDataTable* ClassDataTable;

  UFUNCTION(BlueprintCallable, Category = "Game Flow")
  bool HasActiveRun();

  /**
  * Function to be called before loading the next scene.
  */
  UFUNCTION(BlueprintCallable, Category = "Game Flow")
  void AdvanceToNextFloor();

  /**
  * Function to be called before loading the previous scene.
  */
  UFUNCTION(BlueprintCallable, Category = "Game Flow")
  void GetToPreviousFloor();

  UFUNCTION(BlueprintCallable, Category = "Game Flow")
  void EndActiveRun();

#pragma region Save/Load System
  UFUNCTION(BlueprintCallable, Category = "SaveSystem")
  void CreateNewRun();

  UFUNCTION(BlueprintCallable, Category = "SaveSystem")
  void SaveCurrentRun();

  UFUNCTION(BlueprintCallable, Category = "SaveSystem")
  bool LoadRun();

  // Helper to get data for the Generator or PlayerState
  UFUNCTION(BlueprintCallable, Category = "SaveSystem")
  UOSSaveGame* GetCurrentSaveData() const { return CurrentSaveGame; }
#pragma endregion

#pragma region MiniMap
  UFUNCTION(BlueprintCallable, Category = "MapSystem")
  void RegisterRoomOnMinimap(FRoomPosition Pos, FOSRoomPossibleNeighbour OpenDoors);

  UFUNCTION(BlueprintCallable, Category = "MapSystem")
  void MarkRoomAsExplored(FRoomPosition Pos);

  //To use to reinitilaize the map in cas we remade the map.
  UFUNCTION(BlueprintCallable, Category = "MapSystem")
  void EmptyMap();

  UFUNCTION(BlueprintCallable, Category = "MapSystem")
  bool IsRoomExplored(FRoomPosition Pos);

  UFUNCTION(BlueprintCallable, Category = "MapSystem")
  FOSRoomPossibleNeighbour GetRoomDoors(FRoomPosition Pos);

  UPROPERTY(BlueprintAssignable, Category = "Events")
  FOnMinimapUpdate OnMinimapUpdate;
#pragma endregion

#pragma region MagicSystem
  // Checks if the player currently holds the Grimoire
  UFUNCTION(BlueprintCallable, Category = "MagicSystem")
  bool HasGrimoire();

  // Attempts to record a spell. Returns true if successful, false if they don't have a Grimoire or already know it.
  UFUNCTION(BlueprintCallable, Category = "MagicSystem")
  void RecordSpell(FOSMagicSpell NewSpell);

  // Gets the currently active spell. Returns false if no spell is equipped.
  UFUNCTION(BlueprintCallable, Category = "MagicSystem")
  int32 GetActiveSpellIndex();

  // Sets the active spell by index (from the RecordedSpells array)
  UFUNCTION(BlueprintCallable, Category = "MagicSystem")
  void SetActiveSpell(int32 SpellIndex);

  //Get all the spells with active spell set on 0
  UFUNCTION(BlueprintCallable, Category = "MagicSystem")
  void GetAllSpells(TArray<FOSMagicSpell>& OutAllSpells);
#pragma endregion

#pragma region Meta
  // Call this when the player finishes a run (Death or Win) to bank their earnings
  UFUNCTION(BlueprintCallable, Category = "MetaProgress")
  void AddMetaCurrency(EOSPlayerClass ClassType, int32 Amount);

  // Returns current currency for UI
  UFUNCTION(BlueprintCallable, Category = "MetaProgress")
  int32 GetMetaCurrency(EOSPlayerClass ClassType);

  // Attempts to buy an item from the Shop Table
  // Returns true if successful (enough money)
  UFUNCTION(BlueprintCallable, Category = "MetaProgress")
  bool PurchaseShopItem(EOSPlayerClass ClassType, FName ItemRowName);

  // Checks if an item is already unlocked
  UFUNCTION(BlueprintCallable, Category = "MetaProgress")
  bool IsItemUnlocked(EOSPlayerClass ClassType, FSaveItem Item);

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
  UDataTable* ShopDataTable;
#pragma endregion

  UFUNCTION(BlueprintCallable, Category = "Inventory")
  TArray<FSaveItem> GetInventory();

private:
  EOSPlayerClass CurrentSelectedClass;

  UPROPERTY()
  UOSSaveGame* CurrentSaveGame;
};
