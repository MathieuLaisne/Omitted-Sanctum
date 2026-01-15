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
	float RoomSize = 32.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	float LineThickness = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	FLinearColor ExploredColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Minimap Settings")
	FLinearColor UnexploredColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap Style")
	FSlateBrush RoomBrush;

	// Cache the GameInstance for performance
	UPROPERTY()
	UOSGameInstance* GI;
};