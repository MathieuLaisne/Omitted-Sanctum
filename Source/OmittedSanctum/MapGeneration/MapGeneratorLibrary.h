// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomStructures.h"
#include "MapGeneratorLibrary.generated.h"

/**
 * 
 */
UCLASS()
class OMITTEDSANCTUM_API UMapGeneratorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	static FOSRoomData GetRoomOfType(TArray<FOSRoomData> rooms, FOSRoomType RoomType);


	UFUNCTION(BlueprintCallable)
	static FString ToString(const FOSRoomData& roomData);
	UFUNCTION(BlueprintCallable)
	static FString NeighboursToString(const FOSRoomPossibleNeighbour& roomNeighbours);
};
