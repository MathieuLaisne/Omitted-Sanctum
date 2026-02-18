// OSMinimapWidget.cpp

#include "OSMinimapWidget.h"
#include "Kismet/GameplayStatics.h"

void UOSMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance()))
	{
		// Listen for the update event
		GI->OnMinimapUpdate.AddUniqueDynamic(this, &UOSMinimapWidget::HandleMinimapUpdate);
	}
}

int32 UOSMinimapWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MaxLayerId = LayerId;

	UOSGameInstance* GI = Cast<UOSGameInstance>(GetGameInstance());

	// 1. Safety Checks (Prevent Crash)
	if (!GI || !GI->GetCurrentSaveData()) return MaxLayerId;

	FFloorSaveData& CurrentData = GI->GetCurrentSaveData()->GetCurrentFloorData();
	const TMap<FRoomPosition, FMinimapRoomData>& MapData = CurrentData.MinimapData;
	if (MapData.Num() == 0) return MaxLayerId;

	// 2. Setup Dimensions
	FVector2D WidgetCenter = AllottedGeometry.GetLocalSize() * 0.5f;
	FRoomPosition PlayerPos = GI->GetCurrentSaveData()->PlayerPosition;

	// Dimensions for drawing
	float ActualRoomSize = CellSize * RoomScale;
	float CellPadding = (CellSize - ActualRoomSize) * 0.5f;
	FVector2D RoomSizeVec(ActualRoomSize, ActualRoomSize);

	// Offset to keep Player in the Center of the Minimap
	FVector2D GlobalOffset = WidgetCenter - (FVector2D(PlayerPos.X, PlayerPos.Y) * CellSize);

	// 3. Iterate and Draw
	for (const auto& Pair : MapData)
	{
		FRoomPosition Pos = Pair.Key;
		const FMinimapRoomData& Room = Pair.Value;

		if (Room.bIsDiscovered)
		{

			// -- Calculate Position --
			// Note: Grid Y is Down, Slate Y is Down, so direct multiplication works.
			float ScreenX = GlobalOffset.X + (Pos.X * CellSize) + CellPadding;
			float ScreenY = GlobalOffset.Y + (Pos.Y * CellSize) + CellPadding;
			FVector2D RoomScreenPos(ScreenX, ScreenY);

			// -- Determine Color --
			FLinearColor PaintColor = UnexploredColor; // Default: Seen but not entered

			if (Pos == PlayerPos)
			{
				PaintColor = CurrentRoomColor; // We are here
			}
			else if (Room.bIsExplored)
			{
				PaintColor = ExploredColor;    // We have been here

			}

			// -- Draw Room Body --
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				MaxLayerId,
				AllottedGeometry.ToPaintGeometry(RoomSizeVec, FSlateLayoutTransform(RoomScreenPos)),
				&RoomBrush,
				ESlateDrawEffect::None,
				PaintColor
			);

			// -- Draw Connections (Doors) --
			// We draw small "bridges" extending into the padding area if a door exists
			// But Only if the room was explored

			if (Room.bIsExplored)
			{
				// Door Thickness (Visual width of the corridor)
				float DoorThickness = ActualRoomSize * 0.4f;
				float DoorOffset = (ActualRoomSize - DoorThickness) * 0.5f;

				// NORTH Bridge
				if (Room.OpenDoors.North)
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						MaxLayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(DoorThickness, CellPadding + 1.0f), // +1 overlap to prevent hairline cracks
							FSlateLayoutTransform(FVector2D(ScreenX + DoorOffset, ScreenY - CellPadding))
						),
						&RoomBrush,
						ESlateDrawEffect::None,
						PaintColor
					);
				}

				// SOUTH Bridge
				if (Room.OpenDoors.South)
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						MaxLayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(DoorThickness, CellPadding + 1.0f),
							FSlateLayoutTransform(FVector2D(ScreenX + DoorOffset, ScreenY + ActualRoomSize - 1.0f))
						),
						&RoomBrush,
						ESlateDrawEffect::None,
						PaintColor
					);
				}

				// EAST Bridge
				if (Room.OpenDoors.East)
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						MaxLayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(CellPadding + 1.0f, DoorThickness),
							FSlateLayoutTransform(FVector2D(ScreenX + ActualRoomSize - 1.0f, ScreenY + DoorOffset))
						),
						&RoomBrush,
						ESlateDrawEffect::None,
						PaintColor
					);
				}

				// WEST Bridge
				if (Room.OpenDoors.West)
				{
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						MaxLayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(CellPadding + 1.0f, DoorThickness),
							FSlateLayoutTransform(FVector2D(ScreenX - CellPadding, ScreenY + DoorOffset))
						),
						&RoomBrush,
						ESlateDrawEffect::None,
						PaintColor
					);
				}
			}
		}
	}

	return MaxLayerId + 1;
}

void UOSMinimapWidget::HandleMinimapUpdate()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}