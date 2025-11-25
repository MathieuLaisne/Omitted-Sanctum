// Fill out your copyright notice in the Description page of Project Settings.

#include "Room.h"

// Sets default values
ARoom::ARoom()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomMesh"));
	MeshComponent->SetupAttachment(Root);

	PositionText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PositionText"));
	PositionText->SetupAttachment(Root);

	RootComponent = Root;
}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();

}

void ARoom::Initialize()
{
	FRandomStream RNG;
	
	RNG.Initialize(seed);

	GetComponents(ItemSpawns, true);

	// Shuffle all items list to add random
	TArray<FOSItemAvailable*> ShuffledRows = GetAllItemRows();

	int32 LastIndex = ShuffledRows.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = RNG.RandRange(i, LastIndex);
		if (i != Index) ShuffledRows.Swap(i, Index);
	}

	// Shuffle items spawns to add random
	TArray<UItemSpawnPoint*> ShuffledItemSpawns = ItemSpawns;

	LastIndex = ShuffledItemSpawns.Num() - 1;
	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 Index = RNG.RandRange(i, LastIndex);
		if (i != Index) ShuffledItemSpawns.Swap(i, Index);
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int amountItem = 0;
	int chanceStop = 0;

	for (UItemSpawnPoint* ItemSpawn : ShuffledItemSpawns)
	{
		if (ItemSpawn->AlwaysUsed)
		{
			int32 itemTypeChance = RNG.RandRange(0, 100);
			EOSItemType itemTypeToFetch = ItemSpawn->GetItemType(itemTypeChance);
			FOSItemAvailable* ItemFetched = ShuffledRows.FindByPredicate([itemTypeToFetch](FOSItemAvailable* item){return item->type == itemTypeToFetch; })[0];
			AItem* GeneratedItem = GetWorld()->SpawnActor<AItem>(ItemFetched->ItemClass, ItemSpawn->GetComponentTransform(), SpawnInfo);
			SpawnedItems.Add(GeneratedItem);
		}
		else if(amountItem < maxItemGeneratable && RNG.RandRange(0,100) > chanceStop)
		{
			int32 itemTypeChance = RNG.RandRange(0, 100);
			EOSItemType itemTypeToFetch = ItemSpawn->GetItemType(itemTypeChance);
			FOSItemAvailable* ItemFetched = ShuffledRows.FindByPredicate([itemTypeToFetch](FOSItemAvailable* item) {return item->type == itemTypeToFetch; })[0];
			AItem* GeneratedItem = GetWorld()->SpawnActor<AItem>(ItemFetched->ItemClass, ItemSpawn->GetComponentTransform(), SpawnInfo);
			SpawnedItems.Add(GeneratedItem);
			amountItem++;
			if (amountItem > minItemGeneratable)
			{
				chanceStop = amountItem / maxItemGeneratable * 100; //the more the spawn points have been used, the less likely we'll use one more.
			}
		}
	}
}

void ARoom::DestroySpawnedItems()
{
	for (AItem* Item : SpawnedItems)
	{
		if (Item && IsValid(Item))
		{
			Item->Destroy();
		}
	}
	SpawnedItems.Empty();
}

TArray<FOSItemAvailable*> ARoom::GetAllItemRows()
{
	TArray<FOSItemAvailable*> Rows;
	if (ItemsPool)
	{
		FString Context;
		ItemsPool->GetAllRows<FOSItemAvailable>(Context, Rows);
	}
	return Rows;
}
