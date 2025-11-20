// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomStructures.h"
#include "OSFloorConfig.generated.h"

/**
 * Configuration for a specific Floor (Manor, Dungeon, Cave, Temple).
 * Create a new Data Asset of this type for each floor.
 */
UCLASS()
class OMITTEDSANCTUM_API UOSFloorConfig : public UDataAsset
{
	GENERATED_BODY()
	
	public:
		/** Name of the floor (e.g., "The Manor") */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
		FString FloorName;

		/** The Entrance to this floor*/
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
		FOSRoomData Entrance;

		/** The room leading to the next floor (is a boss in floor 3 and 4)*/
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
		FOSRoomData Exit;

		/** The Table containing all possible rooms for this floor style */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (RequiredAssetDataTags = "RowStructure=/Script/OmittedSanctum.OSRoomData"))
		UDataTable* RoomTable;

		/** How many rooms to generate (Target)
		* Is automatically scaled with difficulty.
		*/
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
		int32 DesiredRoomCount = 20;

		/** * Specific rules for completion.
		 * Floor 2 requires a Key, Floors 3/4 require Bosses.
		 */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rules")
		bool bRequiresKey = false;

		/** The Room with the key to the next floor*/
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (EditCondition = "bRequiresKey"))
		FOSRoomData KeyRoom;

		/** * The visual scaling distortion.
		 * Floor 1: 10%, Floor 2: 20%, etc.
		 * Is automatically scaled with difficulty.
		 */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
		float DistortionAmount = 0.1f;
};
