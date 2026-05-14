#include "OSShopkeeper.h"
#include "OSGameInstance.h"

AOSShopkeeper::AOSShopkeeper()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOSShopkeeper::BeginPlay()
{
	Super::BeginPlay();
	SpeakRandomGreeting();
}

void AOSShopkeeper::SpeakRandomGreeting()
{
	SpeakRandomFromPool(GreetingLines);
}

void AOSShopkeeper::SpeakRandomFarewell()
{
	SpeakRandomFromPool(FarewellLines);
}

void AOSShopkeeper::SpeakRandomNotEnoughMoney()
{
	SpeakRandomFromPool(NotEnoughMoneyLines);
}

void AOSShopkeeper::OnItemFocused(const FOSItemData& Item)
{
	SpeakRandomFromPool(Item.ShopkeeperFocusLines);
}

void AOSShopkeeper::OnItemPurchased(const FOSItemData& Item)
{
	SpeakRandomFromPool(Item.ShopkeeperPurchaseLines);
}

void AOSShopkeeper::SpeakLine(FText Line)
{
	if (Line.IsEmpty()) return;

	UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->OnShopkeeperSpeak.Broadcast(Line);
	}
}

void AOSShopkeeper::SpeakRandomFromPool(const TArray<FText>& Pool)
{
	if (Pool.Num() == 0) return;

	int32 Index = FMath::RandRange(0, Pool.Num() - 1);
	SpeakLine(Pool[Index]);
}
