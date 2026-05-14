// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "OmittedSanctum/UI/OSItemBoxWidget.h"

// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ItemBox"));
	Widget->SetupAttachment(Root);

	InteractionTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionTrigger"));
	InteractionTrigger->SetupAttachment(Root);
	InteractionTrigger->SetCollisionProfileName(TEXT("Item"));
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
}
	
void AItem::SetItemData(FOSItemData data)
{
	ItemData = data;

	UOSItemBoxWidget *box = Cast<UOSItemBoxWidget>(Widget->GetWidget());
	box->UpdateInfoBox(ItemData.ItemName, ItemData.Description);
}

FOSItemData AItem::GetItemData()
{
	return ItemData;
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

