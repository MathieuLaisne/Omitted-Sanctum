// Fill out your copyright notice in the Description page of Project Settings.


#include "MapGeneratorLibrary.h"
#include "RoomStructures.h"

FOSRoomData UMapGeneratorLibrary::GetRoomOfType(TArray<FOSRoomData> rooms, FOSRoomType RoomType)
{
	for (int i = 0; i < rooms.Num(); i++) 
	{ 
		if (rooms[i].RoomTags.Contains(RoomType))
			return rooms[i];
	}
	return FOSRoomData();
}

TArray<FOSRoomData*> UMapGeneratorLibrary::GetDeadEnds(TArray<FOSRoomData*> rooms)
{
	TArray<FOSRoomData*> GoodRooms;
	for (FOSRoomData* room : rooms)
	{
		if (room->Connections.West + room->Connections.East + room->Connections.North + room->Connections.South == 1)
			GoodRooms.Add(room);
	}
	return GoodRooms;
}

FString UMapGeneratorLibrary::ToString(const FOSRoomData& roomData)
{
	FString string = "";
	for (auto types : roomData.RoomTags)
	{
		string += *UEnum::GetValueAsString(types);
		string += " ";
	}
	string += RoomToString(roomData.Connections);
	return string;
}

FString UMapGeneratorLibrary::RoomToString(const FOSRoomPossibleNeighbour& roomNeighbours)
{
	int32 Index = 0;
	if (roomNeighbours.North) Index += 1;
	if (roomNeighbours.South) Index += 2;
	if (roomNeighbours.East)  Index += 4;
	if (roomNeighbours.West)  Index += 8;

	static const TCHAR Symbols[] = {
				L' ', L'╵', L'╷', L'│', L'╶', L'└', L'┌', L'├', L'╴', L'┘', L'┐', L'┤', L'─', L'┴', L'┬', L'┼'
	};

	return FString::Printf(TEXT("%c"),Symbols[Index]);
}