// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OmittedSanctum/OSGameInstance.h"
#include "OSMinimapWidget.generated.h"

UCLASS()
class OMITTEDSANCTUM_API UOSMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	float CellSize = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings", meta = (ClampMin = 0.1, ClampMax = 1.0))
	float RoomScale = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	FLinearColor ExploredColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	FLinearColor CurrentRoomColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	FLinearColor UnexploredColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap Style")
	FSlateBrush RoomBrush;

protected:
	UFUNCTION(BlueprintCallable)
	void HandleMinimapUpdate();

};