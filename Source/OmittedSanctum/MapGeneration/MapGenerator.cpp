// Fill out your copyright notice in the Description page of Project Settings.

#include "MapGenerator.h"
#include "Room.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Containers/Queue.h"
#include "MapGeneratorLibrary.h"

AOSMapGenerator::AOSMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	RandomSeed = 12345;
}

void AOSMapGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void AOSMapGenerator::GenerateDungeon()
{
	if (!CurrentFloorConfig || !CurrentFloorConfig->RoomTable)
	{
		UE_LOG(LogTemp, Error, TEXT("MapGenerator: Missing FloorConfig or RoomTable!"));
		return;
	}

	ClearDungeon();
	RNG.Initialize(RandomSeed);

	TArray<FOSRoomData*> AllRows = GetAllRoomRows();
	if (AllRows.Num() == 0) return;

	FRoomPosition StartPos(0, 0);
	TArray<FRoomPosition> PendingPositions;

	// Entrance Setup
	FOSRoomData* StartRoom = &CurrentFloorConfig->Entrance;
	FGeneratedRoomNode StartNode;
	StartNode.Position = StartPos;
	StartNode.RoomDataPtr = StartRoom;
	StartNode.SpawnRotation = FRotator::ZeroRotator;
	StartNode.PathDistanceFromStart = 0;

	RoomGrid.Add(GetTypeHash(StartPos), StartNode);

	// Initial Neighbor Search based on Fixed Entrance Connections
	// NOTE: North is Negative Y, South is Positive Y
	if (StartRoom->Connections.North) PendingPositions.Add(FRoomPosition(0, -1));
	if (StartRoom->Connections.South) PendingPositions.Add(FRoomPosition(0, 1));
	if (StartRoom->Connections.East)  PendingPositions.Add(FRoomPosition(1, 0));
	if (StartRoom->Connections.West)  PendingPositions.Add(FRoomPosition(-1, 0));

	int32 CurrentRoomCount = 1;
	int32 TargetCount = CurrentFloorConfig->DesiredRoomCount;

	//QA
	bool bGenerationSuccessful = false;

	// Growth Loop
	int32 MaxIterations = TargetCount * 10; // Safety break
	int32 Iterations = 0;

	while (PendingPositions.Num() > 0 && CurrentRoomCount < TargetCount && Iterations < MaxIterations &&  !bGenerationSuccessful)
	{
		Iterations++;

		// Instead of purely random, we prefer positions with fewer neighbors.
		// This encourages the dungeon to grow OUTWARDS (tendrils) rather than filling inward gaps.
		int32 SelectedIndex = -1;
		TArray<int32> OutwardGrowthIndices;

		for (int32 i = 0; i < PendingPositions.Num(); i++)
		{
			// If a spot only has 1 neighbor, it's growing into empty space.
			if (GetNeighborCount(PendingPositions[i]) <= 1)
			{
				OutwardGrowthIndices.Add(i);
			}
		}

		// 70% chance to pick an outward node (creates gaps/corridors), 30% chance to fill a gap
		if (OutwardGrowthIndices.Num() > 0 && RNG.FRand() < 0.7f)
		{
			SelectedIndex = OutwardGrowthIndices[RNG.RandRange(0, OutwardGrowthIndices.Num() - 1)];
		}
		else
		{
			SelectedIndex = RNG.RandRange(0, PendingPositions.Num() - 1);
		}

		FRoomPosition TargetPos = PendingPositions[SelectedIndex];

		// Remove immediately so we don't process it twice
		PendingPositions.RemoveAt(SelectedIndex);

		if (RoomGrid.Contains(GetTypeHash(TargetPos))) continue;

		// Find a room
		FRoomCandidateResult Result = FindCompatibleRoom(TargetPos, AllRows);

		if (Result.bIsValid)
		{
			FGeneratedRoomNode NewNode;
			NewNode.Position = TargetPos;
			NewNode.RoomDataPtr = Result.RoomData;
			NewNode.SpawnRotation = Result.Rotation;
			NewNode.PathDistanceFromStart = 0;

			RoomGrid.Add(GetTypeHash(TargetPos), NewNode);
			CurrentRoomCount++;

			// Add valid neighbors based on the ROTATED connections
			int32 RotSteps = FMath::RoundToInt(Result.Rotation.Yaw / 90.0f);
			FOSRoomPossibleNeighbour RotatedConns = GetRotatedConnections(Result.RoomData->Connections, RotSteps);

			// Only add to pending if the spot isn't already occupied
			auto AddIfEmpty = [&](FRoomPosition P) {
				if (!RoomGrid.Contains(GetTypeHash(P))) PendingPositions.AddUnique(P);
				};

			if (RotatedConns.North) AddIfEmpty(TargetPos + FRoomPosition(0, -1));
			if (RotatedConns.South) AddIfEmpty(TargetPos + FRoomPosition(0, 1));
			if (RotatedConns.East)  AddIfEmpty(TargetPos + FRoomPosition(1, 0));
			if (RotatedConns.West)  AddIfEmpty(TargetPos + FRoomPosition(-1, 0));
		}

		// --- QUALITY CONTROL CHECK ---
		CalculatePathDistances(FRoomPosition(0, 0));

		int32 MaxDist = 0;
		for (auto& Elem : RoomGrid)
		{
			if (Elem.Value.PathDistanceFromStart > MaxDist)
			{
				MaxDist = Elem.Value.PathDistanceFromStart;
			}
		}

		// A good floor should have a path at least 40% of the total room count
		// e.g., if DesiredRoomCount is 20, the Exit must be at least 8 steps away.
		int32 MinRequiredDist = FMath::Max(5, CurrentFloorConfig->DesiredRoomCount * .7f);

		if (MaxDist >= MinRequiredDist && RoomGrid.Num() >= CurrentFloorConfig->DesiredRoomCount * 0.8f)
		{
			bGenerationSuccessful = true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Dungeon generation rejected (Too small/compact). Retrying..."));
		}
	}

	//All rooms with walls to the void get linked to a dead end.
	SealDungeon();

	CalculatePathDistances(StartPos);
	AssignSpecialRooms();
	SpawnRoomActors();
}

FRoomCandidateResult AOSMapGenerator::FindCompatibleRoom(const FRoomPosition& Pos, const TArray<FOSRoomData*>& AvailableRows, bool bForceDeadEnd)
{
	FRoomCandidateResult BestResult;


	TArray<FOSRoomData*> ShuffledRows = AvailableRows;

	int32 LastIndex = ShuffledRows.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = RNG.RandRange(i, LastIndex);
		if (i != Index) ShuffledRows.Swap(i, Index);
	}

	for (FOSRoomData* Candidate : ShuffledRows)
	{
		// Check 4 rotations (0, 90, 180, 270)
		for (int32 i = 0; i < 4; i++)
		{
			FOSRoomPossibleNeighbour RotatedConns = GetRotatedConnections(Candidate->Connections, i);

			if (DoConnectionsFit(Pos, RotatedConns))
			{
				// If we are Sealing (bForceDeadEnd), we want to ensure this room 
				// DOES NOT open any new doors into the void if possible.
				if (bForceDeadEnd)
				{
					bool bOpensToNothing = false;

					auto CheckVoid = [&](FRoomPosition Offset, bool bOpen) {
						if (bOpen && !RoomGrid.Contains(GetTypeHash(Pos + Offset))) bOpensToNothing = true;
						};

					CheckVoid(FRoomPosition(0, -1), RotatedConns.North);
					CheckVoid(FRoomPosition(0, 1), RotatedConns.South);
					CheckVoid(FRoomPosition(1, 0), RotatedConns.East);
					CheckVoid(FRoomPosition(-1, 0), RotatedConns.West);

					if (!bOpensToNothing)
					{
						// Perfect seal found
						BestResult.RoomData = Candidate;
						BestResult.Rotation = FRotator(0.0f, i * 90.0f, 0.0f);
						BestResult.bIsValid = true;
						return BestResult;
					}
				}
				else
				{
					// Normal generation, accept the first valid one
					BestResult.RoomData = Candidate;
					BestResult.Rotation = FRotator(0.0f, i * 90.0f, 0.0f);
					BestResult.bIsValid = true;
					return BestResult;
				}
			}
		}
	}
	return BestResult;
}

FOSRoomPossibleNeighbour AOSMapGenerator::GetRotatedConnections(const FOSRoomPossibleNeighbour& Original, int32 Steps)
{
	int32 Rot = Steps % 4;
	if (Rot < 0) Rot += 4;
	if (Rot == 0) return Original;

	FOSRoomPossibleNeighbour NewConns;

	// 90 Deg Clockwise Rotation:
	// North (-Y)  <-- Old West (-X)
	// East (+X)   <-- Old North (-Y)
	// South (+Y)  <-- Old East (+X)
	// West (-X)   <-- Old South (+Y)

	if (Rot == 1) // 90
	{
		NewConns.North = Original.West;
		NewConns.East = Original.North;
		NewConns.South = Original.East;
		NewConns.West = Original.South;
	}
	else if (Rot == 2) // 180
	{
		NewConns.North = Original.South;
		NewConns.East = Original.West;
		NewConns.South = Original.North;
		NewConns.West = Original.East;
	}
	else if (Rot == 3) // 270
	{
		NewConns.North = Original.East;
		NewConns.East = Original.South;
		NewConns.South = Original.West;
		NewConns.West = Original.North;
	}
	return NewConns;
}

bool AOSMapGenerator::DoConnectionsFit(const FRoomPosition& Pos, const FOSRoomPossibleNeighbour& CandidateConns)
{
	auto CheckDir = [&](FRoomPosition Offset, bool bMySideOpen) -> bool
		{
			FRoomPosition NeighborPos = Offset + Pos;
			if (RoomGrid.Contains(GetTypeHash(NeighborPos)))
			{
				const FGeneratedRoomNode& Neighbor = RoomGrid[GetTypeHash(NeighborPos)];

				int32 NeighborRotSteps = FMath::RoundToInt(Neighbor.SpawnRotation.Yaw / 90.0f);
				FOSRoomPossibleNeighbour NeighborConns = GetRotatedConnections(Neighbor.RoomDataPtr->Connections, NeighborRotSteps);

				bool bNeighborFacingMeOpen = false;

				// COORDINATE CHECK (North = -Y, South = +Y)
				if (Offset.X == 1)  bNeighborFacingMeOpen = NeighborConns.West;   // Neighbor is East (+X), looks West
				if (Offset.X == -1) bNeighborFacingMeOpen = NeighborConns.East;   // Neighbor is West (-X), looks East

				if (Offset.Y == 1)  bNeighborFacingMeOpen = NeighborConns.North;  // Neighbor is South (+Y), looks North
				if (Offset.Y == -1) bNeighborFacingMeOpen = NeighborConns.South;  // Neighbor is North (-Y), looks South

				return bMySideOpen == bNeighborFacingMeOpen;
			}
			return true;
		};

	if (!CheckDir(FRoomPosition(0, -1), CandidateConns.North)) return false; // Check North (-Y)
	if (!CheckDir(FRoomPosition(0, 1), CandidateConns.South)) return false;  // Check South (+Y)
	if (!CheckDir(FRoomPosition(1, 0), CandidateConns.East)) return false;   // Check East (+X)
	if (!CheckDir(FRoomPosition(-1, 0), CandidateConns.West)) return false;  // Check West (-X)

	return true;
}

void AOSMapGenerator::CalculatePathDistances(const FRoomPosition& StartPos)
{
	for (auto& Elem : RoomGrid)
	{
		Elem.Value.PathDistanceFromStart = -1;
	}

	TQueue<FRoomPosition> Queue;
	Queue.Enqueue(StartPos);

	if (RoomGrid.Contains(GetTypeHash(StartPos)))
	{
		RoomGrid[GetTypeHash(StartPos)].PathDistanceFromStart = 0;
	}

	while (!Queue.IsEmpty())
	{
		FRoomPosition CurrentPos;
		Queue.Dequeue(CurrentPos);

		if (!RoomGrid.Contains(GetTypeHash(CurrentPos))) continue;
		FGeneratedRoomNode& CurrentNode = RoomGrid[GetTypeHash(CurrentPos)];

		int32 RotSteps = FMath::RoundToInt(CurrentNode.SpawnRotation.Yaw / 90.0f);
		FOSRoomPossibleNeighbour Conns = GetRotatedConnections(CurrentNode.RoomDataPtr->Connections, RotSteps);

		struct FDir { FRoomPosition Offset; bool bOpen; };

		// BFS Expansion following Coordinate Rules
		FDir Dirs[] = {
			{ FRoomPosition(0, -1), Conns.North }, // North is -Y
			{ FRoomPosition(0, 1), Conns.South },  // South is +Y
			{ FRoomPosition(1, 0), Conns.East },
			{ FRoomPosition(-1, 0), Conns.West }
		};

		for (const FDir& D : Dirs)
		{
			if (D.bOpen)
			{
				FRoomPosition NeighborPos = CurrentPos + D.Offset;
				if (RoomGrid.Contains(GetTypeHash(NeighborPos)))
				{
					FGeneratedRoomNode& NeighborNode = RoomGrid[GetTypeHash(NeighborPos)];
					if (NeighborNode.PathDistanceFromStart == -1)
					{
						NeighborNode.PathDistanceFromStart = CurrentNode.PathDistanceFromStart + 1;
						Queue.Enqueue(NeighborPos);
					}
				}
			}
		}
	}
}

void AOSMapGenerator::AssignSpecialRooms()
{
	FGeneratedRoomNode* ExitNode = nullptr;
	int32 MaxDist = -1;

	for (auto& Elem : RoomGrid)
	{
		if (Elem.Value.PathDistanceFromStart > MaxDist)
		{
			MaxDist = Elem.Value.PathDistanceFromStart;
			ExitNode = &Elem.Value;
		}
	}

	if (ExitNode) ExitNode->RoomDataPtr = &CurrentFloorConfig->Exit;

	if (CurrentFloorConfig->bRequiresKey)
	{
		FGeneratedRoomNode* KeyNode = nullptr;
		int32 BestScore = -1;

		for (auto& Elem : RoomGrid)
		{
			FGeneratedRoomNode& Node = Elem.Value;
			if (&Node == ExitNode || Node.PathDistanceFromStart == 0) continue;

			int32 DistToExit = FMath::Abs(Node.Position.X - ExitNode->Position.X) + FMath::Abs(Node.Position.Y - ExitNode->Position.Y);
			if (DistToExit > 1 && Node.PathDistanceFromStart > BestScore)
			{
				BestScore = Node.PathDistanceFromStart;
				KeyNode = &Node;
			}
		}

		if (KeyNode) KeyNode->RoomDataPtr = &CurrentFloorConfig->KeyRoom;
	}
}

void AOSMapGenerator::SpawnRoomActors()
{
	for (auto& Elem : RoomGrid)
	{
		FGeneratedRoomNode& Node = Elem.Value;
		FVector SpawnLocation(Node.Position.X * TileSize, Node.Position.Y * TileSize, 0.0f);

		uint32 RoomPosHash = GetTypeHash(Node.Position);
		int32 DeterministicSeed = HashCombine(RandomSeed, RoomPosHash);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (Node.RoomDataPtr && Node.RoomDataPtr->RoomClass)
		{
			ARoom* NewRoom = GetWorld()->SpawnActor<ARoom>(Node.RoomDataPtr->RoomClass, SpawnLocation, Node.SpawnRotation, SpawnParams);
			if (NewRoom)
			{
				NewRoom->seed = DeterministicSeed;
				if (NewRoom->PositionText)
				{
					NewRoom->PositionText->SetText(FText::FromString(FString::Printf(TEXT("%d, %d"), Node.Position.X, Node.Position.Y)));
				}
				SpawnedRooms.Add(NewRoom);
				NewRoom->Initialize();
			}
		}
	}
}

TArray<FOSRoomData*> AOSMapGenerator::GetAllRoomRows()
{
	TArray<FOSRoomData*> Rows;
	if (CurrentFloorConfig && CurrentFloorConfig->RoomTable)
	{
		FString Context;
		CurrentFloorConfig->RoomTable->GetAllRows<FOSRoomData>(Context, Rows);
	}
	return Rows;
}

void AOSMapGenerator::ClearDungeon()
{
	for (ARoom* Room : SpawnedRooms)
	{
		if (Room)
		{
			Room->DestroySpawnedItems();

			Room->Destroy();
		}
	}
	SpawnedRooms.Empty();
	RoomGrid.Empty();
}

int32 AOSMapGenerator::GetNeighborCount(const FRoomPosition& Pos)
{
	int32 Count = 0;
	if (RoomGrid.Contains(GetTypeHash(Pos + FRoomPosition(0, -1)))) Count++;
	if (RoomGrid.Contains(GetTypeHash(Pos + FRoomPosition(0, 1)))) Count++;
	if (RoomGrid.Contains(GetTypeHash(Pos + FRoomPosition(1, 0)))) Count++;
	if (RoomGrid.Contains(GetTypeHash(Pos + FRoomPosition(-1, 0)))) Count++;
	return Count;
}

void AOSMapGenerator::SealDungeon()
{
	// 1. Find all exposed open doors that point to empty space
	TArray<FRoomPosition> OpenDoors;

	for (auto& Elem : RoomGrid)
	{
		FRoomPosition Pos = Elem.Value.Position;
		int32 RotSteps = FMath::RoundToInt(Elem.Value.SpawnRotation.Yaw / 90.0f);
		FOSRoomPossibleNeighbour Conns = GetRotatedConnections(Elem.Value.RoomDataPtr->Connections, RotSteps);

		auto CheckAndAdd = [&](FRoomPosition Offset, bool bIsOpen)
			{
				FRoomPosition NeighborPos = Pos + Offset;
				if (bIsOpen && !RoomGrid.Contains(GetTypeHash(NeighborPos)))
				{
					OpenDoors.AddUnique(NeighborPos);
				}
			};

		CheckAndAdd(FRoomPosition(0, -1), Conns.North);
		CheckAndAdd(FRoomPosition(0, 1), Conns.South);
		CheckAndAdd(FRoomPosition(1, 0), Conns.East);
		CheckAndAdd(FRoomPosition(-1, 0), Conns.West);
	}

	TArray<FOSRoomData*> AllRows = GetAllRoomRows();

	// 2. Fill them with Dead Ends (or best fits)
	for (const FRoomPosition& TargetPos : OpenDoors)
	{
		// We pass true to bForceDeadEnd to prefer rooms that don't open NEW paths
		FRoomCandidateResult Result = FindCompatibleRoom(TargetPos, AllRows, true);

		if (Result.bIsValid)
		{
			FGeneratedRoomNode NewNode;
			NewNode.Position = TargetPos;
			NewNode.RoomDataPtr = Result.RoomData;
			NewNode.SpawnRotation = Result.Rotation;
			NewNode.PathDistanceFromStart = 0;
			NewNode.bIsDeadEnd = true; // Mark as filler

			RoomGrid.Add(GetTypeHash(TargetPos), NewNode);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to seal dungeon at %d, %d. You might be missing a Dead End room configuration in your DataTable."), TargetPos.X, TargetPos.Y);
		}
	}
}