// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DrawSphereComponent.h"
#include "OmittedSanctum/OSTypes.h"
#include "Components/WidgetComponent.h"
#include "Item.generated.h"

UCLASS()
class OMITTEDSANCTUM_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	USphereComponent* InteractionTrigger;

	UFUNCTION(BlueprintCallable)
	void SetItemData(FOSItemData data);

	UFUNCTION(BlueprintCallable)
	FOSItemData GetItemData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UWidgetComponent* Widget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FOSItemData ItemData;
};

