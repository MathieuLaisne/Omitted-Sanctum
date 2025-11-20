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

FString UMapGeneratorLibrary::ToString(const FOSRoomData& roomData)
{
	FString string = "";
	for (auto types : roomData.RoomTags)
	{
		string += *UEnum::GetValueAsString(types);
		string += " ";
	}
	string += NeighboursToString(roomData.Connections);
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