// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "OmittedSanctum/OSItemAvailable.h"
#include "RoomStructures.h"
#include "OmittedSanctum/Items/ItemSpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Room.generated.h"

UCLASS()
class OMITTEDSANCTUM_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* MeshComponent;
	UPROPERTY(EditAnywhere)
	UTextRenderComponent* PositionText;

	UPROPERTY(VisibleAnywhere, Category = "Random")
	int seed;

	TArray<UItemSpawnPoint*> ItemSpawns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Generation", meta = (RequiredAssetDataTags = "RowStructure=/Script/OmittedSanctum.OSItemAvailable"))
	UDataTable* ItemsPool;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Generation")
	int maxItemGeneratable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Generation")
	int minItemGeneratable;

	UPROPERTY()
	TArray<AItem*> SpawnedItems;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Generation")
	TArray<AOSEnemySpawnPoint*> EnemySpawns;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Generation")
	UOSEnemyList* Enemypool;*/


	UPROPERTY(VisibleAnywhere, Category = "MapSystem")
	FRoomPosition GridPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MapSystem")
	UBoxComponent* ExplorationTrigger;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	void Initialize(int& amountLeftWeapon, int& amountLeftSpellbook, TSet<UClass*>& GlobalSpawnedLoreItems, const TArray<FOSItemAvailable*>& GlobalFloorItems);

	void DestroySpawnedItems();

	UFUNCTION(BlueprintCallable)
	void OnEntered(class UPrimitiveComponent* ThisComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	TArray<FOSItemAvailable*> GetAllItemRows();

};
