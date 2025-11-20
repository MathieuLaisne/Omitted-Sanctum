// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Room.h"
#include "RoomStructures.generated.h"


USTRUCT(BlueprintType)
struct FOSRoomPossibleNeighbour
{
	GENERATED_USTRUCT_BODY()

public:
	FOSRoomPossibleNeighbour() : West(false), East(false), South(false), North(false) {};
	FOSRoomPossibleNeighbour(bool pos) : West(pos), East(pos), South(pos), North(pos) {};

	/** Negative X	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool West;

	/** Positive X	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool East;

	/** Positive Y	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool South;

	/** Negative Y	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool North;

};

UENUM(BlueprintType)
enum FOSRoomType
{
	/** No Room here */
	Empty				UMETA(DisplayName = "Empty"),
	/** Room with enemies */
	Enemy				UMETA(DisplayName = "Enemy"),
	/** Room with puzzle */
	Puzzle      UMETA(DisplayName = "Puzzle"),
	/** Room with platforming */
	Plaformer   UMETA(DisplayName = "Platformer"),
	/** Room banning words (magic) */
	WordBan			UMETA(DisplayName = "Word Ban"),
	/** Room banning sound (level) */
	SoundBan		UMETA(DisplayName = "Sound Ban"),
	/** Room with a boss fight */
	Boss				UMETA(DisplayName = "Boss"),
	/** Room containing a key to progress */
	KeyRoom			UMETA(DisplayName = "Key Room"),
	/** Room Linked to previous floor */
	Entrance		UMETA(DisplayName = "Entrance"),
	/** Room linked to the next floor */
	Next				UMETA(DisplayName = "Next")
};

/**
 *
 */
USTRUCT(BlueprintType)
struct OMITTEDSANCTUM_API FOSRoomData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	/** The Blueprint of the room to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room")
	TSubclassOf<ARoom> RoomClass;

	/** Which openings does this room have? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FOSRoomPossibleNeighbour Connections;

	/** What type of room this is (Enemy, Puzzle, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TArray<TEnumAsByte<FOSRoomType>> RoomTags;

	/** * Difficulty weighting. 1.0 = Normal.
	 * Higher values make it less likely to spawn on early floors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float Weight = 1.0f;

};

// Helper struct to represent a room position in a 2D grid
USTRUCT(BlueprintType)
struct FRoomPosition
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Y;

	FRoomPosition(int InX, int InY) : X(InX), Y(InY) {};
	FRoomPosition() : X(-1), Y(-1) {};

	bool operator==(const FRoomPosition& Other) const { return X == Other.X && Y == Other.Y; }
	FRoomPosition operator-(const FRoomPosition& Other) const { return FRoomPosition(X - Other.X, Y - Other.Y); }
	FRoomPosition operator+(const FRoomPosition& Other) const { return FRoomPosition(X + Other.X, Y + Other.Y); }
};

FORCEINLINE uint32 GetTypeHash(const FRoomPosition& Pos)
{
	return HashCombine(GetTypeHash(Pos.X), GetTypeHash(Pos.Y));
};