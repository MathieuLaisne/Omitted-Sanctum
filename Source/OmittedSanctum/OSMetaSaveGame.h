#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "OSMetaSaveGame.generated.h"

UCLASS()
class OMITTEDSANCTUM_API UOSMetaSaveGame : public USaveGame
{
  GENERATED_BODY()

public:
  UPROPERTY(VisibleAnywhere, Category = "Meta")
  FString SaveSlotName = "ProfileData";

  UPROPERTY(VisibleAnywhere, Category = "Meta")
  uint32 UserIndex = 0;

  // The currency earned from runs
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
  int32 TotalCurrency = 0;

  // Items unlocked in the shop that can now appear in the dungeon
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
  TSet<FName> UnlockedItemIDs;

};