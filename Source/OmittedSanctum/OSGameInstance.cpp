#include "OSGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "MapGeneration/MapGenerator.h"
#include "MapGeneration/MapGeneratorLibrary.h"
#include "OSPlayerState.h"


UOSGameInstance::UOSGameInstance()
{
  ConstructorHelpers::FObjectFinder<UDataTable> METAPROGDT_BP(TEXT("/Script/Engine.DataTable'/Game/DataTables/DT_ClassInfo.DT_ClassInfo'"));
  METAPROGDT = METAPROGDT_BP.Object;
}

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

    FOSClassMetaProgress ScholarMeta = FOSClassMetaProgress(0, METAPROGDT->FindRow<FOSClassInfo>("Scholar", "")->StartingItems);
    FOSClassMetaProgress DetectiveMeta = FOSClassMetaProgress(0, METAPROGDT->FindRow<FOSClassInfo>("Detective", "")->StartingItems);
    FOSClassMetaProgress DoctorMeta = FOSClassMetaProgress(0, METAPROGDT->FindRow<FOSClassInfo>("Doctor", "")->StartingItems);
    
    CurrentSaveGame->ClassProgressMap.Add({ EOSPlayerClass::Scholar, ScholarMeta });
    CurrentSaveGame->ClassProgressMap.Add({ EOSPlayerClass::Detective, DetectiveMeta });
    CurrentSaveGame->ClassProgressMap.Add({ EOSPlayerClass::Doctor, DoctorMeta });
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

    CurrentSaveGame->RecordedSpells.Empty();
    CurrentSaveGame->InventoryItems.Empty();

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


    SaveCurrentRun();
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
        FMinimapRoomData* Northern = CurrentData.MinimapData.Find(Pos + FRoomPosition(0, -1));
          if (Northern)
            Northern->bIsDiscovered = true;
      }
      if (Data->OpenDoors.South)
      {
        FMinimapRoomData* Southern = CurrentData.MinimapData.Find(Pos + FRoomPosition(0, 1));
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

bool UOSGameInstance::HasGrimoire()
{
  // Search the combined inventory (Meta unlocks + current run items)
  TArray<FOSItemData> Inventory = GetInventory();
  for (const FOSItemData& Item : Inventory)
  {
    if (Item.ItemName.Equals(TEXT("Grimoire"), ESearchCase::IgnoreCase))
    {
      return true;
    }
  }
  return false;
}

void UOSGameInstance::RecordSpell(FOSMagicSpell NewSpell)
{
  if (!CurrentSaveGame) return;

  if (CurrentSaveGame->RecordedSpells.Contains(NewSpell))
  {
    return;
  }

  if (!HasGrimoire())
  {
    CurrentSaveGame->RecordedSpells.Empty();
    CurrentSaveGame->RecordedSpells.Add(NewSpell);
    CurrentSaveGame->ActiveSpellIndex = 0;
    SaveCurrentRun();
    return;
  }

  CurrentSaveGame->RecordedSpells.Add(NewSpell);

  // Auto-equip if it's their first spell
  if (CurrentSaveGame->ActiveSpellIndex == -1)
  {
    CurrentSaveGame->ActiveSpellIndex = 0;
  }

  SaveCurrentRun();
}

int32 UOSGameInstance::GetActiveSpellIndex()
{
  return CurrentSaveGame->ActiveSpellIndex;
}

void UOSGameInstance::SetActiveSpell(int32 SpellIndex)
{
  if (CurrentSaveGame && CurrentSaveGame->RecordedSpells.IsValidIndex(SpellIndex))
  {
    CurrentSaveGame->ActiveSpellIndex = SpellIndex;
    SaveCurrentRun();
  }
}

void UOSGameInstance::GetAllSpells(TArray<FOSMagicSpell>& OutAllSpells)
{
  OutAllSpells = CurrentSaveGame->RecordedSpells;
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
  FOSItemData* ShopItem = ShopDataTable->FindRow<FOSItemData>(ItemRowName, TEXT("ShopPurchase"));
  if (!ShopItem) return false;

  // 2. Get Player Progress
  FOSClassMetaProgress& Progress = CurrentSaveGame->GetProgressFor(ClassType);

  // 3. Check specific conditions
  if (Progress.UnlockedItemIDs.Contains(*ShopItem))
  {
    return false; // Already unlocked this unique item
  }

  // 4. Check Money
  if (Progress.MetaCurrency >= ShopItem->Price)
  {
    // Transaction
    Progress.MetaCurrency -= ShopItem->Price;

    Progress.UnlockedItemIDs.Add(*ShopItem);

    SaveCurrentRun(); // Save immediately
    return true;
  }

  return false; // Not enough cash
}

bool UOSGameInstance::IsItemUnlocked(EOSPlayerClass ClassType, FOSItemData Item)
{
  if (CurrentSaveGame)
  {
    return CurrentSaveGame->GetProgressFor(ClassType).UnlockedItemIDs.Contains(Item);
  }
  return false;
}

TArray<FOSItemData> UOSGameInstance::GetInventory()
{
  TArray<FOSItemData> Inventory = CurrentSaveGame->GetProgressFor(CurrentSelectedClass).UnlockedItemIDs;
  Inventory.Append(CurrentSaveGame->InventoryItems);
  return Inventory;
}

void UOSGameInstance::PickedItem(FOSItemData itemData)
{
  if (CurrentSaveGame->InventoryItems.Contains(itemData))
  {
    int32 existingIDX = CurrentSaveGame->InventoryItems.Find(itemData);
    CurrentSaveGame->InventoryItems[existingIDX].Amount += itemData.Amount;
  }
  else
    CurrentSaveGame->InventoryItems.Add(itemData);
  SaveCurrentRun();
}

void UOSGameInstance::BroadcastNoise(AActor* NoiseMaker, const FVector& Location, EOSNoiseLevel NoiseLevel)
{
	OnNoiseMade.Broadcast(NoiseMaker, Location, NoiseLevel);
}
