// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomStructures.h"
#include "Room.generated.h"

UCLASS()
class OMITTEDSANCTUM_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();
	UStaticMeshComponent* MeshComponent;
	TArray<FOSRoomType> Types;
	FOSRoomPossibleNeighbour PossibleNeighbours;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
