// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistTypes.h"

class SBASizeProgress;
class FBAGraphHandler;

struct FBAGraphOverlayLineParams
{
	float TimeRemaining = 5.0f;
	FVector2D Start;
	FVector2D End;
	FLinearColor Color = FLinearColor::White;
	TWeakPtr<SGraphPin> StartWidget; 
	TWeakPtr<SGraphPin> EndWidget; 
};

struct FBAGraphOverlayBounds
{
	float TimeRemaining = 5.0f;
	FSlateRect Bounds;
	FLinearColor Color = FLinearColor::White;
};

class BLUEPRINTASSIST_API SBlueprintAssistGraphOverlay : public SOverlay
{
	SLATE_BEGIN_ARGS(SBlueprintAssistGraphOverlay) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FBAGraphHandler> InOwnerGraphHandler);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void AddHighlightedPin(const FBAGraphPinHandle& PinHandle, const FLinearColor& Color);
	void AddHighlightedPin(UEdGraphPin* Pin, const FLinearColor& Color);

	void RemoveHighlightedPin(const FBAGraphPinHandle& PinHandle);
	void RemoveHighlightedPin(UEdGraphPin* Pin);

	void DrawLine(const FBAGraphOverlayLineParams& Params)
	{
		LinesToDraw.Add(Params);
	}

	void DrawLine(const FVector2D& Start, const FVector2D& End, FLinearColor Color = FLinearColor::White, float Duration = 5.0f)
	{
		FBAGraphOverlayLineParams Params;
		Params.Start = Start;
		Params.End = End;
		Params.TimeRemaining = Duration;
		Params.Color = Color;
		LinesToDraw.Add(Params);
	}

	void DrawBounds(const FSlateRect& Bounds, FLinearColor Color = FLinearColor::Green, float Duration = 3.0f)
	{
		FBAGraphOverlayBounds Params;
		Params.Bounds = Bounds;
		Params.TimeRemaining = Duration;
		Params.Color = Color;
		DrawBounds(Params);
	}

	void ClearBounds() { BoundsToDraw.Reset(); }

	void DrawBounds(const FBAGraphOverlayBounds& Params)
	{
		BoundsToDraw.Add(Params);
	}

	void DrawNodeInQueue(UEdGraphNode* Node);

	void ClearNodesInQueue() { NodeQueueToDraw.Empty(); }

	TSharedPtr<SBASizeProgress> SizeProgressWidget;

protected:
	TSharedPtr<FBAGraphHandler> OwnerGraphHandler;
	TMap<FBAGraphPinHandle, FLinearColor> PinsToHighlight;

	TArray<FBAGraphOverlayLineParams> LinesToDraw;
	TArray<FBAGraphOverlayBounds> BoundsToDraw;

	TWeakObjectPtr<UEdGraphNode> CurrentNodeToDraw;
	TQueue<TWeakObjectPtr<UEdGraphNode>> NodeQueueToDraw;
	float NextItem = 0.5f;

	const FSlateBrush* CachedBorderBrush = nullptr;
};
