#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OSTypes.h"
#include "OSShopkeeper.generated.h"

UCLASS()
class OMITTEDSANCTUM_API AOSShopkeeper : public AActor
{
	GENERATED_BODY()
	
public:	
	AOSShopkeeper();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shopkeeper|Dialogue")
	TArray<FText> GreetingLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shopkeeper|Dialogue")
	TArray<FText> FarewellLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shopkeeper|Dialogue")
	TArray<FText> NotEnoughMoneyLines;

	UFUNCTION(BlueprintCallable, Category = "Shopkeeper")
	void SpeakRandomGreeting();

	UFUNCTION(BlueprintCallable, Category = "Shopkeeper")
	void SpeakRandomFarewell();

	UFUNCTION(BlueprintCallable, Category = "Shopkeeper")
	void SpeakRandomNotEnoughMoney();

	UFUNCTION(BlueprintCallable, Category = "Shopkeeper")
	void OnItemFocused(const FOSItemData& Item);

	UFUNCTION(BlueprintCallable, Category = "Shopkeeper")
	void OnItemPurchased(const FOSItemData& Item);

	UFUNCTION(BlueprintCallable, Category = "Shopkeeper")
	void SpeakLine(FText Line);

private:
	void SpeakRandomFromPool(const TArray<FText>& Pool);
};
