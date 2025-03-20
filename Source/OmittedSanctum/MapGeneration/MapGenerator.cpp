// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGenerator.h"
#include "Algo/RandomShuffle.h"
#include "MapGeneratorLibrary.h"
#include "Room.h"

// Sets default values
AMapGenerator::AMapGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMapGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMapGenerator::GenerateRoomLayout(const TArray<FOSRoomData>& AvailableRooms, int GridSize, bool bNeedsKeyRoom, bool bGotBoss)
{
  // Define Initialize grid
	Grid.Empty();
  Grid.SetNum(GridSize * GridSize);

	int EntranceIndex = GridSize / 2; //place entrance in the middle of first row.
	int ExitIndex = GridSize * GridSize - FMath::RandRange(1, GridSize - 2); //place exit (boss or next) somewhere on last row
	int KeyIndex = GridSize * GridSize / 4 + FMath::RandRange(-GridSize, GridSize); //place the key somewhere in the middle of the grid

  // Step 1: Place Entrance Room
  Grid[EntranceIndex] = UMapGeneratorLibrary::GetRoomOfType(AvailableRooms, FOSRoomType::Entrance);

  // Step 2: Place Next or Boss Room
	if(bGotBoss)
		Grid[ExitIndex] = UMapGeneratorLibrary::GetRoomOfType(AvailableRooms, FOSRoomType::Boss);
	else
		Grid[ExitIndex] = UMapGeneratorLibrary::GetRoomOfType(AvailableRooms, FOSRoomType::Next);

  // Step 3: Place Key Room (if needed)
	if (bNeedsKeyRoom)
	{
		Grid[KeyIndex] = UMapGeneratorLibrary::GetRoomOfType(AvailableRooms, FOSRoomType::KeyRoom);
	}

  // Step 4: Create path between Exit and Entrance
	TSet<FRoomPosition> Visited;
	UE_LOG(LogTemp, Warning, TEXT("Making Path from Exit (%d)"), ExitIndex);
	PlaceNextRoom(IndexToRoomPosition(ExitIndex, GridSize), IndexToRoomPosition(EntranceIndex, GridSize), GridSize, AvailableRooms, Visited);

	// Step 5: Create path between Key Room and Entrance (if needed)
	if (bNeedsKeyRoom)
	{
		Visited.Empty();
		UE_LOG(LogTemp, Warning, TEXT("Making Path from Key Room"));
		PlaceNextRoom(IndexToRoomPosition(KeyIndex, GridSize), IndexToRoomPosition(EntranceIndex, GridSize), GridSize, AvailableRooms, Visited);
	}
  
	// Step 6: Add rooms to complexify the map

  // Step 7: Ensure Valid Path Exists
  //while(!HasAvailablePath(IndexToRoomPosition(EntranceIndex, GridSize), IndexToRoomPosition(ExitIndex, GridSize), bNeedsKeyRoom ? IndexToRoomPosition(KeyIndex, GridSize) : FRoomPosition(-1, -1), GridSize))
  //{
		//UE_LOG(LogTemp, Warning, TEXT("No Path found, Retrying Generation"));
  //  GenerateRoomLayout(AvailableRooms, GridSize, bNeedsKeyRoom, bGotBoss); // Retry generation
  //}

	UE_LOG(LogTemp, Warning, TEXT("Finished Generating"));
	FString mapString = "\n\n";
	for (int i = 0; i < GridSize; i++)
	{
		for (int j = 0; j < GridSize; j++)
		{
			mapString += *UEnum::GetValueAsString(	Grid[i * GridSize + j].Types[0]	);
			mapString += "\t\t";
		}
		mapString += "\n";
	}
	UE_LOG(LogTemp, Warning, TEXT("%s "), *FString(mapString));
}

bool AMapGenerator::HasAvailablePath(const FRoomPosition& EntrancePos, const FRoomPosition& ExitPos, const FRoomPosition& KeyPos, int GridSize)
{
	TMap<FRoomPosition, const FOSRoomData*> RoomMap;

	// Step 1: Build Room Map
	for (int i = 0; i < Grid.Num(); i++)
	{
		int X = i % GridSize;
		int Y = i / GridSize;
		FRoomPosition Pos(X, Y);
		RoomMap.Add(Pos, &Grid[i]);
	}

	if (EntrancePos.X == -1 || ExitPos.X == -1)
	{
		return false; // No valid Entrance or Next room
	}

	// Step 2: Breadth-First Search (BFS) between Entrance and Next
	TQueue<FRoomPosition> Queue;
	TSet<FRoomPosition> Visited;
	bool valid = false;

	Queue.Enqueue(EntrancePos);
	Visited.Add(EntrancePos);

	while (!Queue.IsEmpty())
	{
		FRoomPosition CurrentPos(-1, -1);
		Queue.Dequeue(CurrentPos);

		if (CurrentPos == ExitPos)
		{
			valid = true; // Found a path
			Queue.Empty();
			Visited.Empty();
			break;
		}

		const FOSRoomData* CurrentRoom = *RoomMap.Find(CurrentPos);
		if (!CurrentRoom) continue;

		// Check valid neighbours
		TArray<FRoomPosition> Neighbours = NeighboursToVectors(*CurrentRoom, CurrentPos, GridSize);

		for (const FRoomPosition& Neighbour : Neighbours)
		{
			if (!IsWithinBounds(Neighbour, GridSize))
				continue;
			if (RoomMap.Contains(Neighbour) && !Visited.Contains(Neighbour) && !EmptyPos(Neighbour, GridSize))
			{
				Queue.Enqueue(Neighbour);
				Visited.Add(Neighbour);
			}
		}
	}

	// Step 3: Breadth-First Search (BFS) between Entrance and Key
	if (KeyPos != FRoomPosition(-1, -1))
	{
		Queue.Empty();
		Visited.Empty();

		Queue.Enqueue(EntrancePos);
		Visited.Add(EntrancePos);

		while (!Queue.IsEmpty())
		{
			FRoomPosition CurrentPos(-1, -1);
			Queue.Dequeue(CurrentPos);

			if (CurrentPos == ExitPos)
			{
				valid = valid && true; // Found a path
				Queue.Empty();
				Visited.Empty();
				break;
			}

			const FOSRoomData* CurrentRoom = *RoomMap.Find(CurrentPos);
			if (!CurrentRoom) continue;

			// Check valid neighbors
			TArray<FRoomPosition> Neighbours = NeighboursToVectors(*CurrentRoom, CurrentPos, GridSize);

			for (const FRoomPosition& Neighbour : Neighbours)
			{
				if (RoomMap.Contains(Neighbour) && !Visited.Contains(Neighbour))
				{
					Queue.Enqueue(Neighbour);
					Visited.Add(Neighbour);
				}
			}
		}
	}

	return valid;
}

bool AMapGenerator::PlaceNextRoom(FRoomPosition Current, FRoomPosition Target, int GridSize, const TArray<FOSRoomData>& AvailableRooms, TSet<FRoomPosition>& Visited)
{
	if (Current == Target) return true; // Reached the Target room, should be useless

	Visited.Add(Current);

	FOSRoomData& CurrentRoomData = Grid[RoomPositionToIndex(Current, GridSize)];

	// Get valid neighbours
	TArray<FRoomPosition> Neighbours = NeighboursToVectors(CurrentRoomData, Current, GridSize);

	if (Neighbours.Contains(Target)) return true; //If the current room is adjacent to the target, then we can try to find the proper room to finish the path

	Algo::RandomShuffle(Neighbours);

	for (const FRoomPosition& NextPos : Neighbours)
	{
		if (Visited.Contains(NextPos) || !IsWithinBounds(NextPos, GridSize)) continue;

		// Get all possible rooms and shuffle the list
		TArray<FOSRoomData> ChosenList = GetValidRooms(AvailableRooms, CurrentRoomData, Current, NextPos);
		Algo::RandomShuffle(ChosenList);

		if (ChosenList.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No valid rooms found for NextPos (%d, %d), backtracking."), NextPos.X, NextPos.Y);
			continue; // Skip if no valid room is available (shouldn't happen as there should always have 1 cross room of all types.)
		}

		for (const FOSRoomData& Chosen : ChosenList)
		{

			UE_LOG(LogTemp, Warning, TEXT("Chosen Room is: %s"), *FString(UMapGeneratorLibrary::ToString(Chosen)));

			bool changedRoom = false;
			FOSRoomData PrevRoom = Grid[RoomPositionToIndex(NextPos, GridSize)];
			FOSRoomData NextRoom = Chosen;
			ARoom* room = nullptr;

			if (EmptyPos(NextPos, GridSize))
			{
				Grid[RoomPositionToIndex(NextPos, GridSize)] = NextRoom;
				changedRoom = true;
				room = GetWorld()->SpawnActor<ARoom>(ARoom::StaticClass(), FVector(220 * NextPos.X, 220 * NextPos.Y, 0), FRotator());
				room->PossibleNeighbours = NextRoom.PossibleNeighbours;
				room->MeshComponent->SetStaticMesh(NextRoom.MeshRoom);
				room->MeshComponent->SetMaterial(0, NextRoom.Material);
			}
			else
			{
				NextRoom = Grid[RoomPositionToIndex(NextPos, GridSize)];
			}

			if (UMapGeneratorLibrary::HasNeededNeighbourAvailable(NextRoom.PossibleNeighbours, Grid[RoomPositionToIndex(Target, GridSize)].PossibleNeighbours, NextPos, Target)) //if the room chosen can also link to the target.
			{
				UE_LOG(LogTemp, Warning, TEXT("Path found"));
				return true;
			}
			else if (RoomIsNeighbour(NextPos, Target)) //if the room chosen is next to target but cannot link to it.
			{
				if(room)
					room->Destroy();
				return false;
			}

			if (PlaceNextRoom(NextPos, Target, GridSize, AvailableRooms, Visited))
			{
				return true;
			}

			if (changedRoom)
			{
				if (room)
					room->Destroy();
				Grid[RoomPositionToIndex(NextPos, GridSize)] = PrevRoom;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No path found from (%d, %d), backtracking."), Current.X, Current.Y);
	return false;
}

bool AMapGenerator::IsWithinBounds(const FRoomPosition & Pos, int GridSize)
{
	return Pos.X >= 0 && Pos.X < GridSize && Pos.Y >= 0 && Pos.Y < GridSize;
}

TArray<FRoomPosition> AMapGenerator::NeighboursToVectors(const FOSRoomData& RoomData, FRoomPosition Current, int GridSize)
{
	const FOSRoomPossibleNeighbour& Neighbours = RoomData.PossibleNeighbours;
	TArray<FRoomPosition> Available;

	if (Neighbours.West)
		Available.Add(Current + FRoomPosition(-1, 0));

	if (Neighbours.East)
		Available.Add(Current + FRoomPosition(1, 0));

	if (Neighbours.South)
		Available.Add(Current + FRoomPosition(0, -1));

	if (Neighbours.North)
		Available.Add(Current + FRoomPosition(0, 1));

	return Available;
}

// Select a random valid room that connects to the given neighbors
FOSRoomData AMapGenerator::GetRandomValidRoom(const TArray<FOSRoomData>& AvailableRooms, const FOSRoomData& CurrentRoom, const FRoomPosition& CurrentPos, const FRoomPosition& NextPos)
{
	FOSRoomData Chosen;
	do {
		Chosen = AvailableRooms[FMath::RandRange(0, AvailableRooms.Num() - 1)];
	} while (!UMapGeneratorLibrary::HasNeededNeighbourAvailable(CurrentRoom.PossibleNeighbours, Chosen.PossibleNeighbours, CurrentPos, NextPos) && IsSpecial(Chosen));

	return Chosen;
}

// Get all valid rooms that connects to the given neighbors
TArray<FOSRoomData> AMapGenerator::GetValidRooms(const TArray<FOSRoomData>& AvailableRooms, const FOSRoomData& CurrentRoom, const FRoomPosition& CurrentPos, const FRoomPosition& NextPos)
{
	TArray<FOSRoomData> Chosen;
	FString string;
	for (const FOSRoomData& room : AvailableRooms)
	{
		if (UMapGeneratorLibrary::HasNeededNeighbourAvailable(CurrentRoom.PossibleNeighbours, room.PossibleNeighbours, CurrentPos, NextPos) && !IsSpecial(room))
		{
			Chosen.Add(room);
			string += UMapGeneratorLibrary::ToString(room) + "; ";
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("All Valid: %s"), *FString(string));
	return Chosen;
}

bool AMapGenerator::IsSpecial(const FOSRoomData& Chosen)
{
	return Chosen.Types.Contains(FOSRoomType::Boss) || Chosen.Types.Contains(FOSRoomType::Entrance) || Chosen.Types.Contains(FOSRoomType::Next) || Chosen.Types.Contains(FOSRoomType::KeyRoom);
}

bool AMapGenerator::RoomIsNeighbour(const FRoomPosition& Pos1, const FRoomPosition& Pos2)
{
	FRoomPosition distance = FRoomPosition(Pos1.X - Pos2.X, Pos1.Y - Pos2.Y);
	return distance.X <= 1 && distance.X >= -1 && distance.Y <= 1 && distance.Y >= -1;
}