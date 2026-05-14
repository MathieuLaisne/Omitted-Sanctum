#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapGeneration/RoomStructures.h"
#include "OSTypes.generated.h"

class AItem;

#pragma region Enums
UENUM(BlueprintType)
enum EOSItemType
{
	Lore,
	Consumable,
	Weapon,
	Spellbook
};

UENUM(BlueprintType)
enum class EOSPlayerClass : uint8
{
  Doctor        UMETA(DisplayName = "The Doctor"),
  Detective     UMETA(DisplayName = "The Detective"),
  Scholar       UMETA(DisplayName = "The Scholar")
};

UENUM(BlueprintType)
enum class EOSNoiseLevel : uint8
{
  Silent        UMETA(DisplayName = "Silent"),
  Faint         UMETA(DisplayName = "Faint"),
  Noisy         UMETA(DisplayName = "Noisy"),
  Loud          UMETA(DisplayName = "Loud")
};
#pragma endregion

#pragma region Structs
USTRUCT(BlueprintType)
struct FOSItemData : public FTableRowBase
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString ItemName;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Description;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<EOSItemType> ItemType;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Amount = 1;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Price = 100;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D* Icon = nullptr;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shopkeeper")
  TArray<FText> ShopkeeperFocusLines;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shopkeeper")
  TArray<FText> ShopkeeperPurchaseLines;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AItem> ItemClass = nullptr;

  FOSItemData() : ItemName(""), Description(""), ItemType(EOSItemType::Consumable), Amount(1), Price(100), Icon(nullptr), ItemClass(nullptr) {}
  FOSItemData(FString n, TEnumAsByte<EOSItemType> t, int a) : ItemName(n), Description(""), ItemType(t), Amount(a), Price(100), Icon(nullptr), ItemClass(nullptr) {}
  bool operator==(const FOSItemData& Other) const { return ItemName == Other.ItemName && ItemType == Other.ItemType; }
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
  TArray<FOSItemData> StartingItems;

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
  TArray<FOSItemData> UnlockedItemIDs;

  FOSClassMetaProgress() : MetaCurrency(0), UnlockedItemIDs({}) {};
  FOSClassMetaProgress(int amount, TArray<FOSItemData> items) : MetaCurrency(amount), UnlockedItemIDs(items) {};
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
#pragma endregion