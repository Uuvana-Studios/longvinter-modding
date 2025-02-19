#include "BlueprintAssistGraphOverlay.h"

#include "BlueprintAssistGraphHandler.h"
#include "SBASizeProgress.h"
#include "BlueprintAssistStyle.h"
#include "BlueprintAssistUtils.h"
#include "SGraphPanel.h"

void SBlueprintAssistGraphOverlay::Construct(const FArguments& InArgs, TSharedPtr<FBAGraphHandler> InOwnerGraphHandler)
{
	OwnerGraphHandler = InOwnerGraphHandler;
	SetVisibility(EVisibility::HitTestInvisible);

	CachedBorderBrush = FBAStyle::GetBrush("BlueprintAssist.WhiteBorder");

	SetCanTick(true);

	AddSlot().VAlign(VAlign_Fill).HAlign(HAlign_Fill).Padding(0)
	[
		SAssignNew(SizeProgressWidget, SBASizeProgress, OwnerGraphHandler)
	];
}

int32 SBlueprintAssistGraphOverlay::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 OutgoingLayer = SOverlay::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// don't paint anything when the size progress is visible
	if (SizeProgressWidget->bIsVisible)
	{
		return OutgoingLayer;
	}

	// do nothing if the current graph handler is not our owner graph handler (or is null)
	TSharedPtr<FBAGraphHandler> CurrentGraphHandler = FBAUtils::GetCurrentGraphHandler();
	if (!CurrentGraphHandler || CurrentGraphHandler != OwnerGraphHandler)
	{
		return OutgoingLayer;
	}

	TSharedPtr<SGraphPanel> GraphPanel = CurrentGraphHandler->GetGraphPanel();
	if (!GraphPanel)
	{
		return OutgoingLayer;
	}

	// highlight pins
	for (auto Kvp : PinsToHighlight)
	{
		FBAGraphPinHandle PinHandle = Kvp.Key;
		FLinearColor Color = Kvp.Value;

		if (UEdGraphPin* Pin = PinHandle.GetPin())
		{
			if (TSharedPtr<SGraphPin> GraphPin = FBAUtils::GetGraphPin(GraphPanel, Pin))
			{
				const FSlateRect PinBounds = FBAUtils::GetPinBounds(GraphPin);
				if (GraphPanel->IsRectVisible(PinBounds.GetBottomRight(), PinBounds.GetTopLeft()))
				{
					const FPaintGeometry PaintGeometry = GraphPin->GetPaintSpaceGeometry().ToPaintGeometry();

					// Draw a border around the pin
					FSlateDrawElement::MakeBox(
						OutDrawElements,
						OutgoingLayer,
						PaintGeometry,
						CachedBorderBrush,
						ESlateDrawEffect::None,
						Color
					);
				}
			}
		}
	}

	if (CurrentNodeToDraw.IsValid())
	{
		if (TSharedPtr<SGraphNode> GraphNode = FBAUtils::GetGraphNode(GraphPanel, CurrentNodeToDraw.Get()))
		{
			const FSlateRect NodeBounds = FBAUtils::GetNodeBounds(GraphNode);
			if (GraphPanel->IsRectVisible(NodeBounds.GetBottomRight(), NodeBounds.GetTopLeft()))
			{
				const FPaintGeometry PaintGeometry = GraphNode->GetPaintSpaceGeometry().ToPaintGeometry();

				// Draw a border around the pin
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					OutgoingLayer,
					PaintGeometry,
					CachedBorderBrush,
					ESlateDrawEffect::None,
					FLinearColor(1, 1, 0, 0.25f)
				);
			}
		}
	}

	// draw lines
	for (const FBAGraphOverlayLineParams& ToDraw : LinesToDraw)
	{
		const FVector2D Start = FBAUtils::GraphCoordToPanelCoord(GraphPanel, ToDraw.Start);
		const FVector2D End = FBAUtils::GraphCoordToPanelCoord(GraphPanel, ToDraw.End);
		TArray<FVector2D> LinePoints = { Start, End };

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			ToDraw.Color,
			true,
			5.0f);
	}

	// draw bounds
	for (const FBAGraphOverlayBounds& ToDraw : BoundsToDraw)
	{
		if (GraphPanel->IsRectVisible(ToDraw.Bounds.GetBottomRight(), ToDraw.Bounds.GetTopLeft()))
		{
			const FVector2D TL = FBAUtils::GraphCoordToPanelCoord(GraphPanel, ToDraw.Bounds.GetTopLeft());
			const FVector2D TR = FBAUtils::GraphCoordToPanelCoord(GraphPanel, ToDraw.Bounds.GetTopRight());
			const FVector2D BL = FBAUtils::GraphCoordToPanelCoord(GraphPanel, ToDraw.Bounds.GetBottomLeft());
			const FVector2D BR = FBAUtils::GraphCoordToPanelCoord(GraphPanel, ToDraw.Bounds.GetBottomRight());
			TArray<FVector2D> LinePoints = { TL, TR, BR, BL, TL };

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				LinePoints,
				ESlateDrawEffect::None,
				ToDraw.Color,
				true,
				2.0f);
		}
	}


	return OutgoingLayer;
}

void SBlueprintAssistGraphOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	for (int i = LinesToDraw.Num() - 1; i >= 0; --i)
	{
		LinesToDraw[i].TimeRemaining -= InDeltaTime;
		if (LinesToDraw[i].TimeRemaining <= 0)
		{
			LinesToDraw.RemoveAt(i);
		}
	}

	for (int i = BoundsToDraw.Num() - 1; i >= 0; --i)
	{
		BoundsToDraw[i].TimeRemaining -= InDeltaTime;
		if (BoundsToDraw[i].TimeRemaining <= 0)
		{
			BoundsToDraw.RemoveAt(i);
		}
	}

	if (NextItem <= 0)
	{
		if (!NodeQueueToDraw.IsEmpty())
		{
			do
			{
				NodeQueueToDraw.Dequeue(CurrentNodeToDraw);
			}
			while (!CurrentNodeToDraw.IsValid() && !NodeQueueToDraw.IsEmpty());

			NextItem += 0.5f;
		}
		else
		{
			CurrentNodeToDraw.Reset();
		}
	}
	else
	{
		NextItem -= InDeltaTime;
	}
}

void SBlueprintAssistGraphOverlay::AddHighlightedPin(const FBAGraphPinHandle& PinHandle, const FLinearColor& Color)
{
	if (PinHandle.IsValid())
	{
		PinsToHighlight.Add(PinHandle, Color);
	}
}

void SBlueprintAssistGraphOverlay::AddHighlightedPin(UEdGraphPin* Pin, const FLinearColor& Color)
{
	AddHighlightedPin(FBAGraphPinHandle(Pin), Color);
}

void SBlueprintAssistGraphOverlay::RemoveHighlightedPin(const FBAGraphPinHandle& PinHandle)
{
	PinsToHighlight.Remove(PinHandle);
}

void SBlueprintAssistGraphOverlay::RemoveHighlightedPin(UEdGraphPin* Pin)
{
	RemoveHighlightedPin(FBAGraphPinHandle(Pin));
}

void SBlueprintAssistGraphOverlay::DrawNodeInQueue(UEdGraphNode* Node)
{
	NodeQueueToDraw.Enqueue(TWeakObjectPtr<UEdGraphNode>(Node));
}