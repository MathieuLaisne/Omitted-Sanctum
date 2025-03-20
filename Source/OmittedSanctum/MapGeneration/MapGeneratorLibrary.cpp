// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorLibrary.h"
#include "RoomStructures.h"

/*
* There's is a 90° rotation in the trig sense, so we have to transform all directions accordingly
*/
bool UMapGeneratorLibrary::HasNeededNeighbourAvailable(FOSRoomPossibleNeighbour Origin, FOSRoomPossibleNeighbour NewNeighbour, const FRoomPosition& OriginPos, const FRoomPosition& NeighbourPos)
{
	FRoomPosition Direction = FRoomPosition(OriginPos.X - NeighbourPos.X, OriginPos.Y - NeighbourPos.Y);

	UE_LOG(LogTemp, Warning, TEXT("Direction: (%d,%d); Origin: %s and Chosen: %s"), Direction.X, Direction.Y, *FString(NeighboursToString(Origin)), *FString(NeighboursToString(NewNeighbour)));

	if (Direction == FRoomPosition(1, 0) && Origin.West && NewNeighbour.East)
		return true;
	if (Direction == FRoomPosition(0, 1) && Origin.North && NewNeighbour.South)
		return true;
	if (Direction == FRoomPosition(-1, 0) && Origin.East && NewNeighbour.West)
		return true;
	if (Direction == FRoomPosition(0, -1) && Origin.South && NewNeighbour.North)
		return true;
	return false;
}

FOSRoomData UMapGeneratorLibrary::GetRoomOfType(TArray<FOSRoomData> rooms, FOSRoomType RoomType)
{
	for (int i = 0; i < rooms.Num(); i++) 
	{ 
		if (rooms[i].Types.Contains(RoomType))
			return rooms[i];
	}
	return FOSRoomData();
}

FString UMapGeneratorLibrary::ToString(const FOSRoomData& roomData)
{
	FString string = "";
	for (auto types : roomData.Types)
	{
		string += *UEnum::GetValueAsString(types);
		string += " ";
	}
	string += NeighboursToString(roomData.PossibleNeighbours);
	return string;
}

FString UMapGeneratorLibrary::NeighboursToString(const FOSRoomPossibleNeighbour& roomNeighbours)
{
	FString string = "(";
	if (roomNeighbours.West)
	{
		string += "W,";
	}
	if (roomNeighbours.East)
	{
		string += "E,";
	}
	if (roomNeighbours.South)
	{
		string += "S,";
	}
	if (roomNeighbours.North)
	{
		string += "N,";
	}
	string += ")";
	return string;
}