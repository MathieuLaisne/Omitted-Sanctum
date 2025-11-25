// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSpawnPoint.h"

// Sets default values
UItemSpawnPoint::UItemSpawnPoint()
{
  billboard = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("Billboard"));
  billboard->SetupAttachment(this);

  billboard->SetHiddenInGame(true);

  PrimaryComponentTick.bCanEverTick = false;

  static ConstructorHelpers::FObjectFinder<UMaterialInterface> MarkerMat(TEXT("/Script/Engine.Material'/Game/Models/PrototypeRooms/MAT_Key.MAT_Key'"));

  if (MarkerMat.Succeeded())
  {
    // Add the element to the billboard configuration
    FMaterialSpriteElement Element;
    Element.Material = MarkerMat.Object;
    Element.bSizeIsInScreenSpace = false;
    Element.BaseSizeX = 20.0f;
    Element.BaseSizeY = 20.0f;

    billboard->Elements.Add(Element);
  }
}

// Called when the game starts or when spawned
void UItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

EOSItemType UItemSpawnPoint::GetItemType(int32 chance)
{
	float chanceF = (float)chance / 100.0f;
	if (chanceF < chances.LoreChance)
		return EOSItemType::Lore;
	else if (chanceF < chances.LoreChance + chances.ConsumableChance)
		return EOSItemType::Consumable;
	else if (chanceF < 1 - chances.SpellbookChance)
		return EOSItemType::Spellbook;
	else
		return EOSItemType::Weapon;
}
