// Fill out your copyright notice in the Description page of Project Settings.

#include "Room.h"
#include "OmittedSanctum/OSGameInstance.h"

// Sets default values
ARoom::ARoom()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoomMesh"));
	MeshComponent->SetupAttachment(Root);

	PositionText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PositionText"));
	PositionText->SetupAttachment(Root);

	ExplorationTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ExplorationTrigger"));
	ExplorationTrigger->SetupAttachment(Root);

	RootComponent = Root;
}

// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();

	ExplorationTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARoom::OnEntered);
}

void ARoom::Initialize(int& amountLeftWeapon, int& amountLeftSpellbook, TSet<UClass*>& GlobalSpawnedLoreItems, const TArray<FOSItemAvailable*>& GlobalFloorItems)
{
	FRandomStream RNG;
	
	RNG.Initialize(seed);

	GetComponents(ItemSpawns, true);

	TArray<FOSItemAvailable*> AllRows;
	if (ItemsPool == nullptr)
		AllRows = GlobalFloorItems;
	else// Creating a map by item type
		AllRows = GetAllItemRows();
	TMap<EOSItemType, TArray<FOSItemAvailable*>> ItemBuckets;

	for (FOSItemAvailable* Row : AllRows)
	{
		if (!Row) continue;

		//Only add lore items which weren't added in the floor
		if (Row->type == EOSItemType::Lore && GlobalSpawnedLoreItems.Contains(Row->ItemClass))
		{
			continue;
		}

		ItemBuckets.FindOrAdd(Row->type).Add(Row);
	}
	// Shuffle the buckets themselves so we pull random items from them
	for (auto& Pair : ItemBuckets)
	{
		TArray<FOSItemAvailable*>& Bucket = Pair.Value;
		if (Bucket.Num() > 1)
		{
			int32 LastIndex = Bucket.Num() - 1;
			for (int32 i = 0; i <= LastIndex; ++i)
			{
				int32 Index = RNG.RandRange(i, LastIndex);
				if (i != Index) Bucket.Swap(i, Index);
			}
		}
	}


	// Shuffle items spawns to add random
	TArray<UItemSpawnPoint*> ShuffledItemSpawns = ItemSpawns;

	int32 LastIndex = ShuffledItemSpawns.Num() - 1;
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
		bool bShouldSpawn = false;

		// Determine if we should attempt to spawn based on logic
		if (ItemSpawn->AlwaysUsed)
			bShouldSpawn = true;
		else if (amountItem < maxItemGeneratable && RNG.RandRange(0, 100) > chanceStop)
			bShouldSpawn = true;

		if (bShouldSpawn)
		{
			// Calculate type using reference counters (Weapons/Spellbooks)
			int32 itemTypeChance = RNG.RandRange(0, 100);
			EOSItemType itemTypeToFetch = ItemSpawn->GetItemType(itemTypeChance, amountLeftWeapon, amountLeftSpellbook);

			// Fetch from our optimized buckets
			if (ItemBuckets.Contains(itemTypeToFetch) && ItemBuckets[itemTypeToFetch].Num() > 0)
			{
				FOSItemAvailable* ItemFetched = ItemBuckets[itemTypeToFetch].Pop();

				if (ItemFetched && ItemFetched->ItemClass)
				{
					AItem* GeneratedItem = GetWorld()->SpawnActor<AItem>(ItemFetched->ItemClass, ItemSpawn->GetComponentTransform(), SpawnInfo);
					SpawnedItems.Add(GeneratedItem);

					// Track Lore items globally to prevent duplicates
					if (itemTypeToFetch == EOSItemType::Lore)
					{
						GlobalSpawnedLoreItems.Add(ItemFetched->ItemClass);
					}

					// Only increment local room count for non-always-used spawns
					if (!ItemSpawn->AlwaysUsed)
					{
						amountItem++;
						if (amountItem > minItemGeneratable)
						{
							chanceStop = amountItem / maxItemGeneratable * 100;
						}
					}
				}
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

void ARoom::OnEntered(class UPrimitiveComponent* ThisComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (	OtherActor->ActorHasTag(FName("Player"))	)
	{
		UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->MarkRoomAsExplored(GridPosition);
		}
	}
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
