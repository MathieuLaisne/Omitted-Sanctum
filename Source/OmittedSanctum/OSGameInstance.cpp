#include "OSGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MapGeneration/MapGenerator.h"
#include "MapGeneration/MapGeneratorLibrary.h"
#include "OSPlayerState.h"

void UOSGameInstance::SetSelectedClass(EOSPlayerClass NewClass)
{
  CurrentSelectedClass = NewClass;
}

bool UOSGameInstance::HasActiveRun()
{
  if (UGameplayStatics::DoesSaveGameExist("CurrentRun", 0))
  {
    CurrentSaveGame = Cast<UOSSaveGame>(UGameplayStatics::LoadGameFromSlot("CurrentRun", 0));
  }
  else
  {
    CurrentSaveGame = Cast<UOSSaveGame>(UGameplayStatics::CreateSaveGameObject(UOSSaveGame::StaticClass()));
  }

  return CurrentSaveGame->bHasActiveRun;
}

void UOSGameInstance::AdvanceToNextFloor()
{
  if (!CurrentSaveGame) return;

  CurrentSaveGame->CurrentFloorIndex++;

  FFloorSaveData& NewFloor = CurrentSaveGame->GetCurrentFloorData();
  NewFloor.FloorSeed = FMath::Rand();

  SaveCurrentRun();
}

void UOSGameInstance::GetToPreviousFloor()
{
  if (!CurrentSaveGame) return;

  CurrentSaveGame->CurrentFloorIndex--;

  SaveCurrentRun();
}

void UOSGameInstance::EndActiveRun()
{
  CurrentSaveGame->bHasActiveRun = false;
  SaveCurrentRun();
}

void UOSGameInstance::CreateNewRun()
{
  if (CurrentSaveGame)
  {
    CurrentSaveGame->PlayerClass = CurrentSelectedClass;
    CurrentSaveGame->CurrentFloorIndex = 1;
    CurrentSaveGame->FloorHistory.Empty();

    // Generate a fresh seed for the new run
    FFloorSaveData& NewFloor = CurrentSaveGame->GetCurrentFloorData();
    NewFloor.FloorSeed = FMath::Rand();

    SaveCurrentRun();

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
    FFloorSaveData& CurrentData = CurrentSaveGame->GetCurrentFloorData();
    if (!CurrentData.MinimapData.Contains(Pos))
    {
      FMinimapRoomData NewData = FMinimapRoomData(false, false, OpenDoors);
      CurrentData.MinimapData.Add(Pos, NewData);
    }
  }
}

void UOSGameInstance::MarkRoomAsExplored(FRoomPosition Pos)
{
  if (CurrentSaveGame)
  {
    FFloorSaveData& CurrentData = CurrentSaveGame->GetCurrentFloorData();
    FMinimapRoomData* Data = CurrentData.MinimapData.Find(Pos);
    if (Data)
    {
      Data->bIsExplored = true;
      Data->bIsDiscovered = true;

      if (Data->OpenDoors.East)
      {
        FMinimapRoomData* Eastern = CurrentData.MinimapData.Find(Pos + FRoomPosition(1, 0));
        if (Eastern)
          Eastern->bIsDiscovered = true;
      }
      if (Data->OpenDoors.West)
      {
        FMinimapRoomData* Western = CurrentData.MinimapData.Find(Pos + FRoomPosition(-1, 0));
          if (Western)
            Western->bIsDiscovered = true;
      }
      if (Data->OpenDoors.North)
      {
        FMinimapRoomData* Northern = CurrentData.MinimapData.Find(Pos + FRoomPosition(0, 1));
          if (Northern)
            Northern->bIsDiscovered = true;
      }
      if (Data->OpenDoors.South)
      {
        FMinimapRoomData* Southern = CurrentData.MinimapData.Find(Pos + FRoomPosition(0, -1));
          if (Southern)
            Southern->bIsDiscovered = true;
      }

      CurrentSaveGame->PlayerPosition = Pos;

      FString positionStr = FString::Printf(TEXT("(%d, %d"), Pos.X, Pos.Y);
      UE_LOG(LogTemp, Log, TEXT("Room %s %s was explored"), *positionStr, *UMapGeneratorLibrary::RoomToString(Data->OpenDoors));

    }
    OnMinimapUpdate.Broadcast();
  }
}

void UOSGameInstance::EmptyMap()
{
  if (CurrentSaveGame)
  {
    FFloorSaveData& CurrentData = CurrentSaveGame->GetCurrentFloorData();
    CurrentData.MinimapData.Empty();
  }
}

bool UOSGameInstance::IsRoomExplored(FRoomPosition Pos)
{
  if (CurrentSaveGame)
  {
    FFloorSaveData& CurrentData = CurrentSaveGame->GetCurrentFloorData();
    if (const FMinimapRoomData* Data = CurrentData.MinimapData.Find(Pos))
    {
      return Data->bIsExplored;
    }
  }
  return false;
}

FOSRoomPossibleNeighbour UOSGameInstance::GetRoomDoors(FRoomPosition Pos)
{
  FFloorSaveData& CurrentData = CurrentSaveGame->GetCurrentFloorData();
  return CurrentData.MinimapData[Pos].OpenDoors;
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
  if (Progress.UnlockedItemIDs.Contains(FSaveItem(ShopItem->ItemName, ShopItem->ItemType)))
  {
    return false; // Already unlocked this unique item
  }

  // 4. Check Money
  if (Progress.MetaCurrency >= ShopItem->Price)
  {
    // Transaction
    Progress.MetaCurrency -= ShopItem->Price;

    Progress.UnlockedItemIDs.Add(FSaveItem(ShopItem->ItemName, ShopItem->ItemType));

    SaveCurrentRun(); // Save immediately
    return true;
  }

  return false; // Not enough cash
}

bool UOSGameInstance::IsItemUnlocked(EOSPlayerClass ClassType, FSaveItem Item)
{
  if (CurrentSaveGame)
  {
    return CurrentSaveGame->GetProgressFor(ClassType).UnlockedItemIDs.Contains(Item);
  }
  return false;
}

TArray<FSaveItem> UOSGameInstance::GetInventory()
{
  TArray<FSaveItem> Inventory = CurrentSaveGame->GetProgressFor(CurrentSelectedClass).UnlockedItemIDs;
  Inventory.Append(CurrentSaveGame->InventoryItems);
  return Inventory;
}
