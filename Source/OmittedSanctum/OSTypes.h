#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Items/Item.h"
#include "OSTypes.generated.h"

UENUM(BlueprintType)
enum class EOSPlayerClass : uint8
{
  Doctor        UMETA(DisplayName = "The Doctor"),
  Detective     UMETA(DisplayName = "The Detective"),
  Scholar       UMETA(DisplayName = "The Scholar")
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
  TArray<TSubclassOf<AItem>> StartingItems;

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
  TArray<FName> UnlockedItemIDs;
};

/**
 * Defines an item sold in the Meta-Shop.
 */
USTRUCT(BlueprintType)
struct FOSShopItem : public FTableRowBase
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FText ItemName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FText Description;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  int32 Price = 100;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D* Icon;
};