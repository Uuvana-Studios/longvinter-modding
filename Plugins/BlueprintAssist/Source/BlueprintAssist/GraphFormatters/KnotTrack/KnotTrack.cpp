// Copyright 2021 fpwong. All Rights Reserved.

#include "KnotTrack.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistUtils.h"
#include "K2Node_Knot.h"
#include "BlueprintAssist/GraphFormatters/FormatterInterface.h"

DEFINE_LOG_CATEGORY(LogKnotTrackCreator);

UEdGraphPin* FKnotNodeCreation::GetPinToConnectTo()
{
	return PinToConnectToHandle.GetPin();
}

TArray<UEdGraphPin*> FKnotNodeCreation::GetPinsToConnectTo() const
{
	TArray<UEdGraphPin*> Pins;// = { GetPinsToConnectTo() };

	for (FBAGraphPinHandle Handle : PinHandlesToConnectTo)
	{
		if (auto Pin = Handle.GetPin())
		{
			Pins.Add(Pin);
		}
	}

	return Pins;
}

UK2Node_Knot* FKnotNodeCreation::CreateKnotNode(const FVector2D InKnotPos, UEdGraphPin* PreviousPin, UK2Node_Knot* KnotNodeToReuse, UEdGraph* Graph)
{
	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Create knot node for pin %s"), *FBAUtils::GetPinName(PreviousPin));

	CreatedKnot = nullptr;

	UEdGraphPin* MainPinToConnectTo = PinToConnectToHandle.GetPin();

	if (KnotNodeToReuse == nullptr)
	{
		CreatedKnot = FBAUtils::CreateKnotNode(Graph, InKnotPos, MainPinToConnectTo, PreviousPin);
	}
	else
	{
		CreatedKnot = KnotNodeToReuse;
		FBAUtils::LinkKnotNodeBetween(KnotNodeToReuse, InKnotPos, MainPinToConnectTo, PreviousPin);
	}

	for (FBAGraphPinHandle& PinHandle : PinHandlesToConnectTo)
	{
		UEdGraphPin* Pin = PinHandle.GetPin();

		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tLinking to pin %s"), *FBAUtils::GetPinName(Pin));

		if (Pin->Direction == EGPD_Input)
		{
			CreatedKnot->GetOutputPin()->MakeLinkTo(Pin);
		}
		else
		{
			CreatedKnot->GetInputPin()->MakeLinkTo(Pin);
		}
	}

	return CreatedKnot;
}

bool FKnotNodeCreation::HasHeightDifference() const
{
	if (KnotToConnectTo.IsValid())
	{
		return KnotToConnectTo->KnotPos.Y == KnotPos.Y;
	}

	return false;
}

FString FKnotNodeCreation::ToString()
{
	if (UEdGraphPin* Pin = GetPinToConnectTo())
	{
		return FBAUtils::GetPinName(Pin);
	}

	return FString("nullptr");
}

float FKnotNodeTrack::GetTrackHeight()
{
	if (UEdGraphPin* Pin = GetPinToAlignTo())
	{
		return GraphHandler->GetPinY(Pin);
	}

	return FBAUtils::GetPinPos(GraphHandler, GetParentPin()).Y + RelativeTrackHeight;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

FKnotNodeTrack::FKnotNodeTrack(
	TSharedPtr<FFormatterInterface> Formatter,
	TSharedPtr<FBAGraphHandler> InGraphHandler,
	UEdGraphPin* InParentPin,
	TArray<UEdGraphPin*> InLinkedTo,
	float InTrackY,
	bool bInIsLoopingTrack)
	: GraphHandler(InGraphHandler)
	, ParentPin(InParentPin)
	, LinkedTo(InLinkedTo)
	, bIsLoopingTrack(bInIsLoopingTrack)
{
	ParentPinPos = FBAUtils::GetPinPos(GraphHandler, InParentPin);

	SetTrackHeight(Formatter);
}

UEdGraphPin* FKnotNodeTrack::GetParentPin()
{
	return ParentPin.GetPin();
}

UEdGraphPin* FKnotNodeTrack::GetLastPin()
{
	return LinkedTo.Num()
		? LinkedTo.Last().GetPin()
		: nullptr;
}

UEdGraphPin* FKnotNodeTrack::GetPinToAlignTo()
{
	return PinToAlignTo.GetPin();
}

FSlateRect FKnotNodeTrack::GetTrackBounds()
{
	const float TrackSpacing = GetDefault<UBASettings>()->BlueprintKnotTrackSpacing;
	const float LocalTrackY = GetTrackHeight();
	const float LastPinX = FBAUtils::GetPinPos(GraphHandler, GetLastPin()).X;
	const float TrackXLeft = FMath::Min(ParentPinPos.X, LastPinX) + 5;
	const float TrackXRight = FMath::Max(ParentPinPos.X, LastPinX) - 5;

	return FSlateRect(FVector2D(TrackXLeft, LocalTrackY - (TrackSpacing - 1) * 0.5f),
					FVector2D(TrackXRight, LocalTrackY + (TrackSpacing - 1) * 0.5f));
}

void FKnotNodeTrack::SetTrackHeight(TSharedPtr<FFormatterInterface> Formatter)
{
	const float TrackSpacing = GetDefault<UBASettings>()->BlueprintKnotTrackSpacing;
	const TArray<UEdGraphNode*>& AllNodes = Formatter->GetFormattedNodes().Array();

	UEdGraphPin* LastPin = GetLastPin();

	const float TrackStart = GetTrackBounds().Left + 10;
	const float TrackEnd = GetTrackBounds().Right - 10;

	// UE_LOG(LogKnotTrackCreator, Error, TEXT("FindTrackHeight for pin (%s) | Start %f | End %f"), *FBAUtils::GetPinName(ParentPin), TrackStart, TrackEnd);

	// use looping track height which was set when track is created
	if (!bIsLoopingTrack)
	{
		// Try align track to the parent pin or last pin
		for (UEdGraphPin* Pin : { ParentPin.GetPin(), LastPin })
		{
			const float PinHeight = GraphHandler->GetPinY(Pin);
			if (TryAlignTrack(Formatter, TrackStart, TrackEnd, PinHeight))
			{
				UpdateTrackHeight(PinHeight);
				return;
			}
		}
	}

	if (bIsLoopingTrack)
	{
		const float NewHeight = FMath::Min(GetParentPin()->GetOwningNode()->NodePosY, LastPin->GetOwningNode()->NodePosY) - 1;
		UpdateTrackHeight(NewHeight);
		return;
	}

	// set starting point
	float TestSolution = bIsLoopingTrack
		? GetTrackHeight()
		: GraphHandler->GetPinY(LastPin);
	
	for (int i = 0; i < 100; ++i)
	{
		bool bNoCollisionInDirection = true;
	
		FVector2D StartPoint(TrackStart, TestSolution);
		FVector2D EndPoint(TrackEnd, TestSolution);
	
		for (UEdGraphNode* NodeToCollisionCheck : AllNodes)
		{
			FSlateRect NodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToCollisionCheck).ExtendBy(FMargin(0, TrackSpacing - 1));
	
			const bool bSkipNode = NodeToCollisionCheck == GetParentPin()->GetOwningNode() || NodeToCollisionCheck == LastPin->GetOwningNode();
			if (!bSkipNode)
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Checking collision for node %s | %f %f  | %f"), *FBAUtils::GetNodeName(NodeToCollisionCheck), TrackStart, TrackEnd, TestSolution);
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%s"), *NodeBounds.ToString());
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%f %f | %f"), TrackStart, TrackEnd, TestSolution);
	
				if (FBAUtils::LineRectIntersection(NodeBounds, StartPoint, EndPoint))
				{
					// UE_LOG(LogKnotTrackCreator, Error, TEXT("\tNode collision  (%s) (%f) | %s"), *FBAUtils::GetNodeName(NodeToCollisionCheck), TestSolution, *NodeBounds.ToString());
					bNoCollisionInDirection = false;
					TestSolution = NodeBounds.Bottom + 1;
				}
			}
		}
	
		if (bNoCollisionInDirection)
		{
			UpdateTrackHeight(TestSolution);
			break;
		}
	}
}

bool FKnotNodeTrack::IsFloatingTrack()
{
	const bool bSameAsParentPin = RelativeTrackHeight == 0;
	const bool bSameAsLastPin = GetTrackHeight() != FBAUtils::GetPinPos(GraphHandler, GetLastPin()).Y;
	return bSameAsParentPin && bSameAsLastPin;
}

void FKnotNodeTrack::UpdateTrackHeight(const float NewTrackY)
{
	RelativeTrackHeight = NewTrackY - FBAUtils::GetPinPos(GraphHandler, GetParentPin()).Y;
	// UE_LOG(LogTemp, Warning, TEXT("Set relative track height %f"), RelativeTrackHeight);

	for (TSharedPtr<FKnotNodeCreation> Creation : KnotCreations)
	{
		Creation->KnotPos.Y = NewTrackY;
	}
}

TSet<UEdGraphNode*> FKnotNodeTrack::GetNodes(UEdGraph* Graph)
{
	TSet<UEdGraphNode*> OutNodes;
	OutNodes.Add(GetParentPin()->GetOwningNode());

	for (auto& PinHandle : LinkedTo)
	{
		UEdGraphPin* Pin = PinHandle.GetPin();
		if (UEdGraphPin* SafePin = FBAUtils::GetPinFromGraph(Pin, Graph))
		{
			OutNodes.Add(SafePin->GetOwningNode());
		}
	}

	return OutNodes;
}

bool FKnotNodeTrack::DoesTrackOverlapNode(UEdGraphNode* Node)
{
	const FSlateRect Bounds = GetTrackBounds();

	return FBAUtils::LineRectIntersection(
		FBAUtils::GetNodeBounds(Node),
		Bounds.GetTopLeft(),
		Bounds.GetBottomRight());
}

bool FKnotNodeTrack::HasPinToAlignTo()
{
	return PinToAlignTo.GetPin() != nullptr;
}

bool FKnotNodeTrack::TryAlignTrack(TSharedPtr<FFormatterInterface> Formatter, float TrackStart, float TrackEnd, float TestHeight)
{
	const float TrackSpacing = GetMutableDefault<UBASettings>()->BlueprintKnotTrackSpacing;

	UEdGraphPin* MyPin = GetParentPin();
	UEdGraphPin* LastPin = GetLastPin();

	const TArray<UEdGraphNode*>& AllNodes = Formatter->GetFormattedNodes().Array();
	for (UEdGraphNode* NodeToCollisionCheck : AllNodes)
	{
		FSlateRect NodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToCollisionCheck).ExtendBy(FVector2D(0, TrackSpacing - 1));

		FVector2D StartPoint(TrackStart, TestHeight);
		FVector2D EndPoint(TrackEnd, TestHeight);

		const bool bSkipNode = NodeToCollisionCheck == MyPin->GetOwningNode() || NodeToCollisionCheck == LastPin->GetOwningNode();
		if (!bSkipNode)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Checking collision for node %s | %f %f  | %f"), *FBAUtils::GetNodeName(NodeToCollisionCheck), TrackStart, TrackEnd, TestHeight);
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%s"), *NodeBounds.ToString());
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%f %f | %f"), TrackStart, TrackEnd, TestHeight);

			if (FBAUtils::LineRectIntersection(NodeBounds, StartPoint, EndPoint))
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t\tCollision!"));
				return false;
			}
		}
	}

	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("No collision!"));
	return true;
}

TArray<UEdGraphNode*> FKnotNodeTrack::GetRelatedNodes()
{
	TArray<UEdGraphNode*> Nodes;
	for (const auto& KnotNodeCreation : KnotCreations)
	{
		for (auto Pin : KnotNodeCreation->GetPinsToConnectTo())
		{
			Nodes.Add(Pin->GetOwningNode());
		}
	}
	return Nodes;
}

FString FKnotNodeTrack::ToString()
{
	return FString::Printf(TEXT("%s (%s) -> %s (%s) (%f)"),
		*FBAUtils::GetPinName(GetParentPin()),
		*FBAUtils::GetNodeName(GetParentPin()->GetOwningNode()),
		*FBAUtils::GetPinName(GetLastPin()),
		*FBAUtils::GetNodeName(GetLastPin()->GetOwningNode()),
		GetTrackHeight());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

TArray<UEdGraphNode*> FGroupedTracks::GetKnots() const
{
	TArray<UEdGraphNode*> Knots;
	for (const auto& Track : Tracks)
	{
		for (const auto & KnotNodeCreation : Track->KnotCreations)
		{
			if (KnotNodeCreation->CreatedKnot)
			{
				Knots.Add(Cast<UEdGraphNode>(KnotNodeCreation->CreatedKnot));
			}
		}
	}

	return Knots;
}

FSlateRect FGroupedTracks::GetBounds() const
{
	TOptional<FSlateRect> Bounds;
	for (const auto& KnotNodeTrack : Tracks)
	{
		const FSlateRect TrackBounds = KnotNodeTrack->GetTrackBounds();
		Bounds = Bounds.IsSet() ? Bounds->Expand(TrackBounds) : TrackBounds;
	}

	return Bounds.Get(FSlateRect());
}

TSet<UEdGraphNode*> FGroupedTracks::GetRootNodes() const
{
	TSet<UEdGraphNode*> RootNodes;
	for (auto KnotNodeTrack : Tracks)
	{
		RootNodes.Add(KnotNodeTrack->GetParentPin()->GetOwningNode());
		RootNodes.Add(KnotNodeTrack->GetLastPin()->GetOwningNode());
	}
	return RootNodes;
}
