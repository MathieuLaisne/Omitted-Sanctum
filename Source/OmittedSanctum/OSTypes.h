#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Items/Item.h"
#include "MapGeneration/RoomStructures.h"
#include "OSTypes.generated.h"

UENUM(BlueprintType)
enum class EOSPlayerClass : uint8
{
  Doctor        UMETA(DisplayName = "The Doctor"),
  Detective     UMETA(DisplayName = "The Detective"),
  Scholar       UMETA(DisplayName = "The Scholar")
};

USTRUCT(BlueprintType)
struct FSaveItem
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString ItemName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<EOSItemType> ItemType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int Amount;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AItem> ItemClass;

  FSaveItem() : ItemName(""), ItemType(EOSItemType::Consumable), Amount(0) {};
  FSaveItem(FString n, TEnumAsByte<EOSItemType> t) : ItemName(n), ItemType(t), Amount(1) {};
  FSaveItem(FString n, TEnumAsByte<EOSItemType> t, int a) : ItemName(n), ItemType(t), Amount(a) {};

  bool operator==(const FSaveItem& Other) const { return ItemName == Other.ItemName && ItemType == Other.ItemType; };
};

/** Data Table Row for configuring class stats */
USTRUCT(BlueprintType)
struct FOSClassInfo : public FTableRowBase
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FText DisplayName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<APawn> PawnClass;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 BaseMaxHP = 100;

  // Matches: First-Aid Kit, Gun, Grimoire, etc.
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FSaveItem> StartingItems;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D* ClassIcon;
};

USTRUCT(BlueprintType)
struct FOSClassMetaProgress
{
  GENERATED_BODY()

  // Persistent Currency specific to this class
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 MetaCurrency = 0;

  // List of Item IDs (Row Names) that this class has unlocked in the shop
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<FSaveItem> UnlockedItemIDs;

  FOSClassMetaProgress() : MetaCurrency(0), UnlockedItemIDs({}) {};
  FOSClassMetaProgress(int amount, TArray<FSaveItem> items) : MetaCurrency(amount), UnlockedItemIDs(items) {};
};

/**
 * Defines an item sold in the Meta-Shop.
 */
USTRUCT(BlueprintType)
struct FOSShopItem : public FTableRowBase
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString ItemName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Description;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<EOSItemType> ItemType;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Price = 100;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D* Icon;
};

USTRUCT(BlueprintType)
struct FMinimapRoomData
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bIsExplored = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  bool bIsDiscovered = false;

  // We store the connections here so the UI knows where to draw lines/openings
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FOSRoomPossibleNeighbour OpenDoors;

  FMinimapRoomData() {};
  FMinimapRoomData(bool bExplored, bool bDiscovered, FOSRoomPossibleNeighbour Doors)
    : bIsExplored(bExplored), bIsDiscovered(bDiscovered), OpenDoors(Doors) {
  }

};

USTRUCT(BlueprintType)
struct FFloorSaveData
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere)
  int32 FloorSeed; // The seed used to generate this specific floor

  UPROPERTY(VisibleAnywhere)
  TMap<FRoomPosition, FMinimapRoomData> MinimapData; // The map for this specific floor
};
