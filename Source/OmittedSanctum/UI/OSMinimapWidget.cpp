// Fill out your copyright notice in the Description page of Project Settings.

#include "OSMinimapWidget.h"
#include "Kismet/GameplayStatics.h"

void UOSMinimapWidget::NativeConstruct()
{
  Super::NativeConstruct();
  GI = Cast<UOSGameInstance>(GetGameInstance());
}

int32 UOSMinimapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
  int32 MaxLayerId = LayerId;

  if (!GI || !GI->GetCurrentSaveData()) return MaxLayerId;

  const TMap<FRoomPosition, FMinimapRoomData>& MapData = GI->GetCurrentSaveData()->MinimapData;

  // Find the center of the widget to center the map (or center on player)
  FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;

  // Get Player Position (Simplified, you might want to fetch actual Pawn Grid Position)
  // Assuming the player is at (0,0) relative to the grid for this example, 
  // or you can pass the CurrentRoomGridPosition into the widget.

  for (const auto& Pair : MapData)
  {
    FRoomPosition Pos = Pair.Key;
    const FMinimapRoomData& Data = Pair.Value;

    // Calculate Screen Position relative to center
    // Note: Y in Grid is Down, Y in Slate is Down.
    float X = Center.X + (Pos.X * RoomSize);
    float Y = Center.Y + (Pos.Y * RoomSize);

    FVector2D RoomScreenPos(X, Y);
    FVector2D RoomSizeVec(RoomSize - 4.0f, RoomSize - 4.0f); // -4 for padding

    // Draw Box
    FSlateDrawElement::MakeBox(
      OutDrawElements,
      MaxLayerId,
      AllottedGeometry.ToPaintGeometry(RoomSizeVec, FSlateLayoutTransform(RoomScreenPos)),
      &RoomBrush,
      ESlateDrawEffect::None,
      Data.bIsExplored ? ExploredColor : UnexploredColor
    );

    // Draw Corridors (Optional: Draw lines based on Data.OpenDoors)
    if (Data.bIsExplored) {
      // Logic to draw lines extending from RoomScreenPos based on OpenDoors.North, etc.
    }
  }

  return MaxLayerId + 1;
}