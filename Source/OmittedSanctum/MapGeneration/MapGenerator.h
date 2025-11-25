// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomStructures.h"
#include "OSFloorConfig.h"
#include "Room.h"
#include "MapGenerator.generated.h"

/**
 * Structure to hold generation data before spawning actors
 */
struct FGeneratedRoomNode
{
	FRoomPosition Position;
	FOSRoomData* RoomDataPtr; // Pointer to the row in the DataTable
	FRotator SpawnRotation;   // The rotation needed for this room to fit
	int32 PathDistanceFromStart; // Actual walking distance (Nodes traversed)

	// To track which generic room we should swap for a special one (Key/Boss/Stairs)
	bool bIsDeadEnd;

	FGeneratedRoomNode()
		: RoomDataPtr(nullptr)
		, SpawnRotation(FRotator::ZeroRotator)
		, PathDistanceFromStart(0)
		, bIsDeadEnd(false)
	{
	}
};

/**
 * Helper struct to return search results
 */
struct FRoomCandidateResult
{
	FOSRoomData* RoomData;
	FRotator Rotation;
	bool bIsValid;

	FRoomCandidateResult() : RoomData(nullptr), Rotation(FRotator::ZeroRotator), bIsValid(false) {}
};

UCLASS()
class OMITTEDSANCTUM_API AOSMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AOSMapGenerator();

	/** The specific configuration for this run (Manor, Dungeon, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	UOSFloorConfig* CurrentFloorConfig;

	/** Seed for RNG to share runs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	int32 RandomSeed;

	/** Size of the tiles (e.g. 1000 units) to space them out correctly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
	float TileSize = 1000.0f;

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateDungeon();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void ClearDungeon();

protected:
	virtual void BeginPlay() override;

private:
	// Internal grid storage
	TMap<uint32, FGeneratedRoomNode> RoomGrid;

	// Helper to retrieve data from the table
	TArray<FOSRoomData*> GetAllRoomRows();

	// Logic to find a room that fits a specific spot (Checks all rotations)
	FRoomCandidateResult FindCompatibleRoom(const FRoomPosition& Pos, const TArray<FOSRoomData*>& AvailableRows, bool bForceDeadEnd = false);

	// Check if specific connections fit at a specific position
	bool DoConnectionsFit(const FRoomPosition& Pos, const FOSRoomPossibleNeighbour& Connections);

	// Helper to rotate connections
	FOSRoomPossibleNeighbour GetRotatedConnections(const FOSRoomPossibleNeighbour& Original, int32 RotationSteps);

	// Calculate true walking distances using BFS
	void CalculatePathDistances(const FRoomPosition& StartPos);

	// Post-process: assign Keys, Bosses, and Exits based on GDD logic
	void AssignSpecialRooms();

	// Spawn the actual actors
	void SpawnRoomActors();

	int32 GetNeighborCount(const FRoomPosition& Pos);

	void SealDungeon();

	// Random Stream for deterministic generation
	FRandomStream RNG;

	// Track spawned actors for cleanup
	UPROPERTY()
	TArray<ARoom*> SpawnedRooms;
};