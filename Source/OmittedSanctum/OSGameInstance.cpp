#include "OSGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MapGeneration/MapGenerator.h"
#include "OSPlayerState.h"

void UOSGameInstance::SetSelectedClass(EOSPlayerClass NewClass)
{
  CurrentSelectedClass = NewClass;
}

void UOSGameInstance::CreateNewRun()
{
  // Create a fresh save object
  CurrentSaveGame = Cast<UOSSaveGame>(UGameplayStatics::CreateSaveGameObject(UOSSaveGame::StaticClass()));

  if (CurrentSaveGame)
  {
    CurrentSaveGame->PlayerClass = CurrentSelectedClass;
    CurrentSaveGame->CurrentFloorIndex = 1;

    // Generate a fresh seed for the new run
    CurrentSaveGame->CurrentDungeonSeed = FMath::Rand();

    CurrentSaveGame->MinimapData.Empty();

    // Look up base stats from DataTable
    if (ClassDataTable)
    {
      FString Context;
      // Find row by Enum name
      FName RowName = *UEnum::GetValueAsString(CurrentSelectedClass);
      FOSClassInfo* Row = ClassDataTable->FindRow<FOSClassInfo>(RowName, Context);

      CurrentSaveGame->CurrentHP = Row->BaseMaxHP;
      CurrentSaveGame->MaxHP = Row->BaseMaxHP;
    }
    else
    {
      CurrentSaveGame->CurrentHP = 100;
    }
  }
}

void UOSGameInstance::SaveCurrentRun()
{
  if (!CurrentSaveGame) return;

  // 1. Gather Data from World
  APawn* PlayerPawn = GetFirstLocalPlayerController()->GetPawn();
  if (PlayerPawn)
  {
    AOSPlayerState* PS = PlayerPawn->GetPlayerState<AOSPlayerState>();
    if (PS)
    {
      CurrentSaveGame->CurrentHP = PS->CurrentHP;
      // Add MaxHP to PlayerState if not there, or fetch from StatusComponent
    }
  }

  // 2. Write to disk
  UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSaveGame->SaveSlotName, CurrentSaveGame->UserIndex);
  UE_LOG(LogTemp, Log, TEXT("Game Saved!"));
}

bool UOSGameInstance::LoadRun()
{
  // Check if save exists
  if (UGameplayStatics::DoesSaveGameExist("CurrentRun", 0))
  {
    CurrentSaveGame = Cast<UOSSaveGame>(UGameplayStatics::LoadGameFromSlot("CurrentRun", 0));
    if (CurrentSaveGame)
    {
      CurrentSelectedClass = CurrentSaveGame->PlayerClass;
      return true;
    }
  }
  return false;
}

void UOSGameInstance::RegisterRoomOnMinimap(FRoomPosition Pos, FOSRoomPossibleNeighbour OpenDoors)
{
  if (CurrentSaveGame)
  {
    // If it doesn't exist, add it as unexplored. 
    // If it exists, we just update doors (generation phase).
    if (!CurrentSaveGame->MinimapData.Contains(Pos))
    {
      FMinimapRoomData NewData = FMinimapRoomData(false, OpenDoors);
      CurrentSaveGame->MinimapData.Add(Pos, NewData);
    }
  }
}

void UOSGameInstance::MarkRoomAsExplored(FRoomPosition Pos)
{
  if (CurrentSaveGame)
  {
    FMinimapRoomData* Data = CurrentSaveGame->MinimapData.Find(Pos);
    if (Data)
    {
      Data->bIsExplored = true;
    }
  }
}

bool UOSGameInstance::IsRoomExplored(FRoomPosition Pos)
{
  if (CurrentSaveGame)
  {
    if (const FMinimapRoomData* Data = CurrentSaveGame->MinimapData.Find(Pos))
    {
      return Data->bIsExplored;
    }
  }
  return false;
}

FOSRoomPossibleNeighbour UOSGameInstance::GetRoomDoors(FRoomPosition Pos)
{
  return CurrentSaveGame->MinimapData[Pos].OpenDoors;
}

void UOSGameInstance::AddMetaCurrency(EOSPlayerClass ClassType, int32 Amount)
{
  FOSClassMetaProgress& Progress = CurrentSaveGame->GetProgressFor(ClassType);
  Progress.MetaCurrency += Amount;

  // Auto-save meta progress immediately
  SaveCurrentRun();
}

int32 UOSGameInstance::GetMetaCurrency(EOSPlayerClass ClassType)
{
  if (CurrentSaveGame)
  {
    return CurrentSaveGame->GetProgressFor(ClassType).MetaCurrency;
  }
  return 0;
}

bool UOSGameInstance::PurchaseShopItem(EOSPlayerClass ClassType, FName ItemRowName)
{
  if (!ShopDataTable) return false;

  // 1. Find the Item Data
  FOSShopItem* ShopItem = ShopDataTable->FindRow<FOSShopItem>(ItemRowName, TEXT("ShopPurchase"));
  if (!ShopItem) return false;

  // 2. Get Player Progress
  FOSClassMetaProgress& Progress = CurrentSaveGame->GetProgressFor(ClassType);

  // 3. Check specific conditions
  if (Progress.UnlockedItemIDs.Contains(ItemRowName))
  {
    return false; // Already unlocked this unique item
  }

  // 4. Check Money
  if (Progress.MetaCurrency >= ShopItem->Price)
  {
    // Transaction
    Progress.MetaCurrency -= ShopItem->Price;

    Progress.UnlockedItemIDs.Add(ItemRowName);

    SaveCurrentRun(); // Save immediately
    return true;
  }

  return false; // Not enough cash
}

bool UOSGameInstance::IsItemUnlocked(EOSPlayerClass ClassType, FName ItemRowName)
{
  if (CurrentSaveGame)
  {
    return CurrentSaveGame->GetProgressFor(ClassType).UnlockedItemIDs.Contains(ItemRowName);
  }
  return false;
}