// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistTypes.h"

#include "SGraphPin.h"

class UK2Node_Knot;
struct FFormatterInterface;
class FBAGraphHandler;

BLUEPRINTASSIST_API DECLARE_LOG_CATEGORY_EXTERN(LogKnotTrackCreator, Log, All);

struct BLUEPRINTASSIST_API FKnotNodeCreation
	: public TSharedFromThis<FKnotNodeCreation>
{
	TSharedPtr<struct FKnotNodeTrack> OwningKnotTrack;

	bool bMakeLinkForPrevious = false;

	FVector2D KnotPos;
	TSharedPtr<FKnotNodeCreation> KnotToConnectTo = nullptr;
	UK2Node_Knot* CreatedKnot = nullptr;

	FBAGraphPinHandle PinToConnectToHandle;
	TSet<FBAGraphPinHandle> PinHandlesToConnectTo;

	FKnotNodeCreation() : PinToConnectToHandle(nullptr) { }

	FKnotNodeCreation(
		TSharedPtr<FKnotNodeTrack> InOwningKnotTrack,
		const FVector2D InKnotPos,
		TSharedPtr<FKnotNodeCreation> InKnotToConnectTo,
		UEdGraphPin* InPinToConnectTo)
		: OwningKnotTrack(InOwningKnotTrack)
		, KnotPos(InKnotPos)
		, KnotToConnectTo(InKnotToConnectTo)
		, PinToConnectToHandle(InPinToConnectTo)
	{
		if (InPinToConnectTo != nullptr)
		{
			PinHandlesToConnectTo.Add(PinToConnectToHandle);
		}
	}

	UEdGraphPin* GetPinToConnectTo();
	TArray<UEdGraphPin*> GetPinsToConnectTo() const;

	UK2Node_Knot* CreateKnotNode(FVector2D InKnotPos, UEdGraphPin* PreviousPin, UK2Node_Knot* KnotNodeToReuse, UEdGraph* Graph);

	bool HasHeightDifference() const;

	FString ToString();
};

struct BLUEPRINTASSIST_API FKnotNodeTrack
	: public TSharedFromThis<FKnotNodeTrack>
{
	TSharedPtr<FBAGraphHandler> GraphHandler;

	FBANodePinHandle ParentPin;
	FVector2D ParentPinPos;

	FBANodePinHandle PinToAlignTo;
	float PinAlignedX = 0.0f;

	TArray<FBANodePinHandle> LinkedTo;

	TArray<TSharedPtr<FKnotNodeCreation>> KnotCreations;
	bool bIsLoopingTrack = false;

	FKnotNodeTrack(
		TSharedPtr<FFormatterInterface> Formatter,
		TSharedPtr<FBAGraphHandler> InGraphHandler,
		UEdGraphPin* InParentPin,
		TArray<UEdGraphPin*> InLinkedTo,
		float InTrackY,
		bool bInIsLoopingTrack);

	float GetTrackHeight();

	UEdGraphPin* GetParentPin();

	UEdGraphPin* GetLastPin();

	UEdGraphPin* GetPinToAlignTo();

	FSlateRect GetTrackBounds();

	void SetTrackHeight(TSharedPtr<FFormatterInterface> Formatter);

	bool IsFloatingTrack();

	void UpdateTrackHeight(float NewTrackY);

	TSet<UEdGraphNode*> GetNodes(UEdGraph* Graph);

	bool DoesTrackOverlapNode(UEdGraphNode* Node);

	bool HasPinToAlignTo();

	bool TryAlignTrack(TSharedPtr<FFormatterInterface> Formatter, float TrackStart, float TrackEnd, float TestHeight);

	TArray<UEdGraphNode*> GetRelatedNodes();

	FString ToString();

private:
	float RelativeTrackHeight = 0.0f;
};

struct BLUEPRINTASSIST_API FGroupedTracks
{
	UEdGraphNode* ParentNode;
	TArray<TSharedPtr<FKnotNodeTrack>> Tracks;

	float Width = MIN_flt;

	bool bLooping = false;

	void Init()
	{
		for (auto Track : Tracks)
		{
			Width = FMath::Max(Width, Track->GetTrackBounds().GetSize().X);
			bLooping |= Track->bIsLoopingTrack;
		}
	}

	TArray<UEdGraphNode*> GetKnots() const;
	FSlateRect GetBounds() const;
	TSet<UEdGraphNode*> GetRootNodes() const;
};