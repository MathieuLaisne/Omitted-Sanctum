// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomStructures.h"
#include "MapGenerator.generated.h"

UCLASS()
class OMITTEDSANCTUM_API AMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMapGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void GenerateRoomLayout(const TArray<FOSRoomData>& AvailableRooms, int GridSize, bool bNeedsKeyRoom, bool gotBoss);

	bool HasAvailablePath(const FRoomPosition& EntrancePos, const FRoomPosition& ExitPos, const FRoomPosition& KeyPos, int GridSize);

	UPROPERTY(BlueprintReadWrite)
	TArray<FOSRoomData> Grid;

	UFUNCTION(BlueprintCallable)
	static FRoomPosition IndexToRoomPosition(int idx, int GridSize) {
		int x = idx % GridSize;
		int y = idx / GridSize;
		return FRoomPosition(x, y);
	}

	UFUNCTION(BlueprintCallable)
	static int RoomPositionToIndex(FRoomPosition pos, int GridSize) {
		int i = pos.Y * GridSize + pos.X;
		return i;
	}

	UFUNCTION(BlueprintCallable)
	bool EmptyPos(const FRoomPosition& NextPos, int GridSize)
	{
		FOSRoomData room = Grid[RoomPositionToIndex(NextPos, GridSize)];
		return room.Types.Contains(FOSRoomType::Empty);
	}

private:
	bool PlaceNextRoom(FRoomPosition Current, FRoomPosition Target, int GridSize, const TArray<FOSRoomData>& AvailableRooms, TSet<FRoomPosition>& Visited);
	static bool IsWithinBounds(const FRoomPosition& Pos, int GridSize);

	TArray<FRoomPosition> NeighboursToVectors(const FOSRoomData& RoomData, FRoomPosition Current, int GridSize);
	FOSRoomData GetRandomValidRoom(const TArray<FOSRoomData>& AvailableRooms, const FOSRoomData& CurrentRoom, const FRoomPosition& CurrentPos, const FRoomPosition& NextPos);
	TArray<FOSRoomData> GetValidRooms(const TArray<FOSRoomData>& AvailableRooms, const FOSRoomData& CurrentRoom, const FRoomPosition& CurrentPos, const FRoomPosition& NextPos);
	bool IsSpecial(const FOSRoomData& Chosen);
	bool RoomIsNeighbour(const FRoomPosition& Pos1, const FRoomPosition& Pos2);
};
