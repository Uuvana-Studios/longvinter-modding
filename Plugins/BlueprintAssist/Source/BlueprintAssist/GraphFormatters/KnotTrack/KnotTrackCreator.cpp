// Copyright 2021 fpwong. All Rights Reserved.

#include "KnotTrackCreator.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistStats.h"
#include "BlueprintAssistUtils.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_Knot.h"
#include "BlueprintAssist/GraphFormatters/BlueprintAssistCommentHandler.h"
#include "BlueprintAssist/GraphFormatters/FormatterInterface.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Stats/StatsMisc.h"

void FKnotTrackCreator::Init(TSharedPtr<FFormatterInterface> InFormatter, TSharedPtr<FBAGraphHandler> InGraphHandler)
{
	Formatter = InFormatter;
	GraphHandler = InGraphHandler;

	const UBASettings* BASettings = GetDefault<UBASettings>();
	NodePadding = BASettings->BlueprintFormatterSettings.Padding;
	PinPadding = BASettings->BlueprintParameterPadding;
	TrackSpacing = BASettings->BlueprintKnotTrackSpacing;
}

void FKnotTrackCreator::FormatKnotNodes()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::FormatKnotNodes"), STAT_KnotTrackCreator_FormatNode, STATGROUP_BA_EdGraphFormatter);
	//UE_LOG(LogKnotTrackCreator, Warning, TEXT("### Format Knot Nodes"));

	MakeKnotTrack();

	MergeNearbyKnotTracks();

	ExpandKnotTracks();

	if (!GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("Useless"))
	{
		RemoveUselessCreationNodes();
	}

	CreateKnotTracks();

	if (GetDefault<UBASettings>()->bAddKnotNodesToComments)
	{
		AddKnotNodesToComments();
	}

	for (auto TrackGroup : TrackGroups)
	{
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("LIST TRACK GROUPS"));
		for (auto Track : TrackGroup->Tracks)
		{
			for (const auto & Creation : Track->KnotCreations)
			{
				if (Creation->CreatedKnot)
				{
					KnotTrackGroup.Add(Creation->CreatedKnot, TrackGroup);
					KnotCreationMap.Add(Creation->CreatedKnot, Creation);
				}
			}

			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%s"), *Track->ToString());
		}
	}

	for (auto Kvp : RelativeCreationMapping)
	{
		RelativeMapping.FindOrAdd(Kvp.Key->CreatedKnot).Add(Kvp.Value);
	}

	for (TSharedPtr<FGroupedTracks> Group : TrackGroups)
	{
		for (TSharedPtr<FKnotNodeTrack> Track : Group->Tracks)
		{
			TArray<UEdGraphNode*> RelatedNodes = Track->GetRelatedNodes();
			for (TSharedPtr<FKnotNodeCreation> Creation : Track->KnotCreations)
			{
				// make all creations in tracks related to each other
				for (TSharedPtr<FKnotNodeCreation> Other : Track->KnotCreations)
				{
					if (Creation == Other)
						continue;

					RelativeMapping.FindOrAdd(Creation->CreatedKnot).Add(Other->CreatedKnot);
				}

				// make all creations related to their nodes
				for (UEdGraphNode* RelatedNode : RelatedNodes)
				{
					RelativeMapping.FindOrAdd(Creation->CreatedKnot).Add(RelatedNode);

					// for looping tracks also make the node related to the track
					if (Track->bIsLoopingTrack)
					{
						RelativeMapping.FindOrAdd(Creation->CreatedKnot).Add(RelatedNode);
					}
				}
			}
		}
	}
}

void FKnotTrackCreator::CreateKnotTracks()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::CreateKnotTracks"), STAT_KnotTrackCreator_CreateKnotTracks, STATGROUP_BA_EdGraphFormatter);

	// we sort tracks by
	// 1. exec pin track over parameter track 
	// 2. top-most-track-height 
	// 3. top-most parent pin 
	// 4. left-most
	const auto& TrackSorter = [](const TSharedPtr<FKnotNodeTrack> TrackA, const TSharedPtr<FKnotNodeTrack> TrackB)
	{
		const bool bIsExecPinA = FBAUtils::IsExecPin(TrackA->GetLastPin());
		const bool bIsExecPinB = FBAUtils::IsExecPin(TrackB->GetLastPin());

		if (bIsExecPinA != bIsExecPinB)
		{
			return bIsExecPinA > bIsExecPinB;
		}

		if (TrackA->GetTrackHeight() != TrackB->GetTrackHeight())
		{
			return TrackA->GetTrackHeight() < TrackB->GetTrackHeight();
		}

		if (TrackA->ParentPinPos.Y != TrackB->ParentPinPos.Y)
		{
			return TrackA->ParentPinPos.Y < TrackB->ParentPinPos.Y;
		}

		return TrackA->GetTrackBounds().GetSize().X < TrackB->GetTrackBounds().GetSize().X;
	};
	KnotTracks.Sort(TrackSorter);

	// we need to save the pin height since creating knot nodes for certain nodes (K2Node_LatentAbilityCall) will cause
	// the node to recreate its pins (including their guids) and so we fail to find the cached pin height 
	TMap<TSharedPtr<FKnotNodeTrack>, float> SavedPinHeight;
	for (TSharedPtr<FKnotNodeTrack> KnotTrack : KnotTracks)
	{
		UEdGraphPin* PinToAlignTo = KnotTrack->GetPinToAlignTo();
		if (PinToAlignTo != nullptr)
		{
			SavedPinHeight.Add(KnotTrack, GraphHandler->GetPinY(PinToAlignTo));
		}
	}

	for (TSharedPtr<FKnotNodeTrack> KnotTrack : KnotTracks)
	{
		// sort knot creations
		auto GraphCapture = GraphHandler->GetFocusedEdGraph();
		const auto CreationSorter = [GraphCapture](TSharedPtr<FKnotNodeCreation> CreationA, TSharedPtr<FKnotNodeCreation> CreationB)
		{
			UEdGraphPin* Pin = CreationA->PinToConnectToHandle.GetPin();

			if (FBAUtils::IsExecPin(Pin))
			{
				return CreationA->KnotPos.X > CreationB->KnotPos.X;
			}

			return CreationA->KnotPos.X < CreationB->KnotPos.X;
		};

		if (!KnotTrack->bIsLoopingTrack)
		{
			KnotTrack->KnotCreations.StableSort(CreationSorter);
		}

		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Creating knot track %s"), *FBAUtils::GetPinName(KnotTrack->GetParentPin()));
		// if (KnotTrack->PinToAlignTo.IsValid())
		// {
		// 	UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tShould make aligned track!"));
		// }


		TSharedPtr<FKnotNodeCreation> LastCreation = nullptr;
		const int NumCreations = KnotTrack->KnotCreations.Num();
		for (int i = 0; i < NumCreations; i++)
		{
			TSharedPtr<FKnotNodeCreation> Creation = KnotTrack->KnotCreations[i];

			FVector2D KnotPos = Creation->KnotPos;
			KnotPos.Y = KnotTrack->GetTrackHeight();

			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Making knot creation at %s %d"), *KnotPos.ToString(), i);
			// for (auto Pin : Creation->PinHandlesToConnectTo)
			// {
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tPin %s"), *FBAUtils::GetPinName(FBAUtils::GetPinFromGraph(Pin, GraphHandler->GetFocusedEdGraph())));
			// }

			UEdGraphPin* PinToAlignTo = KnotTrack->GetPinToAlignTo();
			if (PinToAlignTo != nullptr)
			{
				KnotPos.Y = SavedPinHeight.Contains(KnotTrack) ? SavedPinHeight[KnotTrack] : GraphHandler->GetPinY(PinToAlignTo);
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Created knot aligned to %s"), *FBAUtils::GetNodeName(PinToAlignTo->GetOwningNode()));
			}

			if (!LastCreation.IsValid()) // create a knot linked to the first pin (the fallback pin)
			{
				UEdGraphPin* ParentPin = FBAGraphPinHandle(KnotTrack->GetParentPin()).GetPin();
				UK2Node_Knot* KnotNode = CreateKnotNode(Creation.Get(), KnotPos, ParentPin);
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Create initial %s"), *FBAUtils::GetPinName(ParentPin));

				KnotNodesSet.Add(KnotNode);
				LastCreation = Creation;
				RelativeMapping.FindOrAdd(ParentPin->GetOwningNode()).Add(KnotNode);

				if (PinToAlignTo)
				{
					// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Made aligned node"));
					PinAlignedKnots.Add(KnotNode);
				}
			}
			else // create a knot that connects to the last knot
			{
				UK2Node_Knot* ParentKnot = LastCreation->CreatedKnot;

				const bool bCreatePinAlignedKnot = LastCreation->PinHandlesToConnectTo.Num() == 1 && PinToAlignTo != nullptr;
				if (bCreatePinAlignedKnot && NumCreations == 1) // move the parent knot to the aligned x position
				{
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("Create pin aligned!"));
					for (FBAGraphPinHandle& PinHandle : Creation->PinHandlesToConnectTo)
					{
						UEdGraphPin* Pin = PinHandle.GetPin();

						UEdGraphPin* ParentPin = Pin->Direction == EGPD_Input
							? ParentKnot->GetOutputPin()
							: ParentKnot->GetInputPin();
						FBAUtils::TryCreateConnection(ParentPin, Pin);
					}
				}
				else
				{
					// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Create normal"));
					UEdGraphPin* LastPin = KnotTrack->GetLastPin();

					UEdGraphPin* PinOnLastKnot = LastPin->Direction == EGPD_Output
						? ParentKnot->GetInputPin()
						: ParentKnot->GetOutputPin();

					UK2Node_Knot* NewKnot = CreateKnotNode(Creation.Get(), KnotPos, PinOnLastKnot);
					KnotNodesSet.Add(NewKnot);

					RelativeMapping.FindOrAdd(Creation->OwningKnotTrack->GetParentPin()->GetOwningNode()).Add(NewKnot);
					RelativeMapping.FindOrAdd(Creation->OwningKnotTrack->GetLastPin()->GetOwningNode()).Add(NewKnot);

					LastCreation = Creation;
				}
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(GraphHandler->GetBlueprint());

	// Cleanup knot node pool
	for (auto KnotNode : KnotNodePool)
	{
		if (FBAUtils::GetLinkedNodes(KnotNode).Num() == 0)
		{
			FBAUtils::DeleteNode(KnotNode);
		}
	}
	KnotNodePool.Empty();
}

void FKnotTrackCreator::ExpandKnotTracks()
{
	if (GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("Expand"))
	{
		return;
	}

	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::ExpandKnotTracks"), STAT_KnotTrackCreator_ExpandKnotTracks, STATGROUP_BA_EdGraphFormatter);
	// UE_LOG(LogKnotTrackCreator, Error, TEXT("### Expanding Knot Tracks"));
	// for (auto Elem : KnotTracks)
	// {
	// 	UE_LOG(LogKnotTrackCreator, Warning, TEXT("KnotTrack %s"), *FBAUtils::GetPinName(Elem->GetParentPin()));
	// }

	// sort tracks by:
	// 1. exec over parameter
	// 2. Highest track Y
	// 3. Smallest track width
	// 4. Parent pin height
	TSharedPtr<FBAGraphHandler> GraphHandlerCapture = GraphHandler;
	const auto& ExpandTrackSorter = [GraphHandlerCapture](const TSharedPtr<FKnotNodeTrack>& TrackA, const TSharedPtr<FKnotNodeTrack>& TrackB)
	{
		if (TrackA->bIsLoopingTrack != TrackB->bIsLoopingTrack)
		{
			return TrackA->bIsLoopingTrack < TrackB->bIsLoopingTrack;
		}

		const bool bIsExecPinA = FBAUtils::IsExecPin(TrackA->GetLastPin());
		const bool bIsExecPinB = FBAUtils::IsExecPin(TrackB->GetLastPin());
		if (bIsExecPinA != bIsExecPinB)
		{
			return bIsExecPinA > bIsExecPinB;
		}

		// if (bIsExecPinA && TrackA->bIsLoopingTrack != TrackB->bIsLoopingTrack)
		// {
		// 	return TrackA->bIsLoopingTrack < TrackB->bIsLoopingTrack;
		// }

		if (TrackA->GetTrackHeight() != TrackB->GetTrackHeight())
		{
			return TrackA->bIsLoopingTrack
				? TrackA->GetTrackHeight() > TrackB->GetTrackHeight()
				: TrackA->GetTrackHeight() < TrackB->GetTrackHeight();
		}

		const float WidthA = TrackA->GetTrackBounds().GetSize().X;
		const float WidthB = TrackB->GetTrackBounds().GetSize().X;
		if (WidthA != WidthB)
		{
			return TrackA->bIsLoopingTrack
				? WidthA > WidthB
				: WidthA < WidthB;
		}

		return GraphHandlerCapture->GetPinY(TrackA->GetLastPin()) < GraphHandlerCapture->GetPinY(TrackB->GetLastPin());
	};

	const auto& OverlappingTrackSorter = [GraphHandlerCapture](const TSharedPtr<FKnotNodeTrack>& TrackA, const TSharedPtr<FKnotNodeTrack>& TrackB)
	{
		if (TrackA->bIsLoopingTrack != TrackB->bIsLoopingTrack)
		{
			return TrackA->bIsLoopingTrack < TrackB->bIsLoopingTrack;
		}

		const bool bIsExecPinA = FBAUtils::IsExecPin(TrackA->GetLastPin());
		const bool bIsExecPinB = FBAUtils::IsExecPin(TrackB->GetLastPin());
		if (bIsExecPinA != bIsExecPinB)
		{
			return bIsExecPinA > bIsExecPinB;
		}

		const float WidthA = TrackA->GetTrackBounds().GetSize().X;
		const float WidthB = TrackB->GetTrackBounds().GetSize().X;
		if (WidthA != WidthB)
		{
			return TrackA->bIsLoopingTrack
				? WidthA > WidthB
				: WidthA < WidthB;
		}

		return GraphHandlerCapture->GetPinY(TrackA->GetLastPin()) < GraphHandlerCapture->GetPinY(TrackB->GetLastPin());
	};

	TArray<TSharedPtr<FKnotNodeTrack>> SortedTracks = KnotTracks;
	SortedTracks.StableSort(ExpandTrackSorter);

	TArray<TSharedPtr<FKnotNodeTrack>> PendingTracks = SortedTracks;

	// for (auto Track : SortedTracks)
	// {
	// 	UE_LOG(LogKnotTrackCreator, Warning, TEXT("Expanding tracks %s"), *Track->ToString());
	// 	if (Track->bIsLoopingTrack)
	// 	{
	// 		UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tIsLooping"));
	// 	}
	// }

	TSet<TSharedPtr<FGroupedTracks>> PlacedGroups;
	TSet<TSharedPtr<FKnotNodeTrack>> PlacedTracks;
	while (PendingTracks.Num() > 0)
	{
		TSharedPtr<FKnotNodeTrack> CurrentTrack = PendingTracks[0];
		PlacedTracks.Add(CurrentTrack);

		// const float TrackY = CurrentTrack->GetTrackHeight();

		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Process pending Track %s (%s)"), *FBAUtils::GetPinName(CurrentTrack->GetParentPin()), *FBAUtils::GetNodeName(CurrentTrack->GetParentPin()->GetOwningNode()));

		// check against all other tracks, and find ones which overlap with the current track
		TArray<TSharedPtr<FKnotNodeTrack>> OverlappingTracks;
		OverlappingTracks.Add(CurrentTrack);

		float CurrentLowestTrackHeight = CurrentTrack->GetTrackHeight();
		FSlateRect OverlappingBounds = CurrentTrack->GetTrackBounds();
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Current Track bounds %s"), *CurrentTrack->GetTrackBounds().ToString());
		bool bFoundCollision = true;
		do
		{
			bFoundCollision = false;
			for (TSharedPtr<FKnotNodeTrack> Track : SortedTracks)
			{
				if (PlacedTracks.Contains(Track))
				{
					continue;
				}

				if (OverlappingTracks.Contains(Track))
				{
					continue;
				}

				if (Track->bIsLoopingTrack != CurrentTrack->bIsLoopingTrack)
				{
					continue;
				}

				// if looping tracks share the same related nodes then they should count as 'overlapping'
				if (Track->bIsLoopingTrack) // TODO: maybe we should do this for normal tracks too?
				{
					TSet<UEdGraphNode*> OverlappingSet;
					for (TSharedPtr<FKnotNodeTrack> Overlapping : OverlappingTracks)
					{
						OverlappingSet.Append(Overlapping->GetRelatedNodes());
					}

					TSet<UEdGraphNode*> TrackSet(Track->GetRelatedNodes());

					if (OverlappingSet.Intersect(TrackSet).Num() > 0)
					{
						OverlappingTracks.Add(Track);
						PlacedTracks.Add(Track);
						bFoundCollision = true;
						continue;
					}
				}

				FSlateRect TrackBounds = Track->GetTrackBounds();

				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tOverlapping Bounds %s | %s"), *OverlappingBounds.ToString(), *TrackBounds.ToString());
				if (FSlateRect::DoRectanglesIntersect(OverlappingBounds, TrackBounds))
				{
					OverlappingTracks.Add(Track);
					PlacedTracks.Add(Track);
					bFoundCollision = true;

					OverlappingBounds.Top = FMath::Min(Track->GetTrackHeight(), OverlappingBounds.Top);
					OverlappingBounds.Left = FMath::Min(TrackBounds.Left, OverlappingBounds.Left);
					OverlappingBounds.Right = FMath::Max(TrackBounds.Right, OverlappingBounds.Right);
					OverlappingBounds.Bottom = OverlappingBounds.Top + (OverlappingTracks.Num() * TrackSpacing);

					if (CurrentTrack->HasPinToAlignTo())
					{
						CurrentTrack->PinToAlignTo.SetPin(nullptr);
					}

					if (Track->HasPinToAlignTo())
					{
						// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Removed pin to align to for %s"), *Track->ToString());
						Track->PinToAlignTo.SetPin(nullptr);
					}
					// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tTrack %s colliding %s"), *FBAUtils::GetPinName(Track->GetParentPin()), *FBAUtils::GetPinName(CurrentTrack->GetParentPin()));
				}
			}
		}
		while (bFoundCollision);

		// if (OverlappingTracks.Num() == 1)
		// {
		// 	PendingTracks.Remove(CurrentTrack);
		// 	continue;
		// }

		TArray<TSharedPtr<FKnotNodeTrack>> ExecTracks;

		// Group overlapping tracks by node (expect for exec tracks)
		TArray<FGroupedTracks> OverlappingGroupedTracks;
		for (TSharedPtr<FKnotNodeTrack> Track : OverlappingTracks)
		{
			if (FBAUtils::IsExecPin(Track->GetParentPin()) && !Track->bIsLoopingTrack)
			{
				ExecTracks.Add(Track);
				continue;
			}

			const auto MatchesNode = [&Track](const FGroupedTracks& OtherTrack)
			{
				return Track->GetParentPin()->GetOwningNode() == OtherTrack.ParentNode;
			};

			FGroupedTracks* Group = OverlappingGroupedTracks.FindByPredicate(MatchesNode);
			if (Group)
			{
				Group->Tracks.Add(Track);
			}
			else
			{
				FGroupedTracks NewGroup;
				NewGroup.ParentNode = Track->GetParentPin()->GetOwningNode();
				NewGroup.Tracks.Add(Track);
				OverlappingGroupedTracks.Add(NewGroup);
			}
		}

		ExecTracks.StableSort(OverlappingTrackSorter);

		for (auto& Group : OverlappingGroupedTracks)
		{
			Group.Init();
			Group.Tracks.StableSort(OverlappingTrackSorter);
		}

		const auto& GroupSorter = [](const FGroupedTracks& GroupA, const FGroupedTracks& GroupB)
		{
			if (GroupA.bLooping != GroupB.bLooping)
			{
				return GroupA.bLooping < GroupB.bLooping;
			}

			return GroupA.Width < GroupB.Width;
		};
		OverlappingGroupedTracks.StableSort(GroupSorter);

		TSharedPtr<FGroupedTracks> AllGroup = MakeShareable(new FGroupedTracks());
		TrackGroups.Add(AllGroup);

		int TrackCount = 0;
		for (auto Track : ExecTracks)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tSolo track %s"), *Track->ToString());
			Track->UpdateTrackHeight(CurrentLowestTrackHeight + (TrackCount * TrackSpacing));
			TrackCount += 1;
			AllGroup->Tracks.Add(Track);
		}

		for (FGroupedTracks& Group : OverlappingGroupedTracks)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Group %s"), *FBAUtils::GetNodeName(Group.ParentNode));
			for (TSharedPtr<FKnotNodeTrack> Track : Group.Tracks)
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tTrack %s"), *Track->ToString());
				Track->UpdateTrackHeight(CurrentLowestTrackHeight + (TrackCount * TrackSpacing));
				TrackCount += 1;
				AllGroup->Tracks.Add(Track);
			}
		}

		// For looping tracks we need to move the parent nodes down so the track is above the nodes
		if (CurrentTrack->bIsLoopingTrack)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Fix looping!"));
			FSlateRect ExpandedBounds = AllGroup->GetBounds();
			const float Padding = CurrentTrack->bIsLoopingTrack ? TrackSpacing * 2 : TrackSpacing;
			ExpandedBounds.Bottom += Padding;

			TOptional<float> LoopingDelta;
			for (TSharedPtr<FKnotNodeTrack> Track : AllGroup->Tracks)
			{
				// compare the parent node and last node to see which one is the highest
				UEdGraphNode* ParentNode = Track->GetParentPin()->GetOwningNode();
				UEdGraphNode* LastNode = Track->GetLastPin()->GetOwningNode();
				float ParentDelta = ExpandedBounds.Bottom - GraphHandler->GetCachedNodeBounds(ParentNode).Top;
				float LastDelta = ExpandedBounds.Bottom - GraphHandler->GetCachedNodeBounds(LastNode).Top;

				float LargestDelta = FMath::Max(ParentDelta, LastDelta) + 1;

				LoopingDelta = FMath::Max(LoopingDelta.Get(0.0f), LargestDelta);   
			}

			if (LoopingDelta.IsSet())
			{
				if (CurrentTrack->bIsLoopingTrack)
				{
					TSet<UEdGraphNode*> RelatedNodes;
					for (auto Track : AllGroup->Tracks)
					{
						Track->UpdateTrackHeight(Track->GetTrackHeight() - LoopingDelta.GetValue());
						UEdGraphNode* ParentNode = Track->GetParentPin()->GetOwningNode();
						UEdGraphNode* LastNode = Track->GetLastPin()->GetOwningNode();
						RelatedNodes.Add(ParentNode);
						RelatedNodes.Add(LastNode);
					}

					TSet<UEdGraphNode*> VisitedNodes;
					for (UEdGraphNode* Node : RelatedNodes)
					{
						// UE_LOG(LogTemp, Warning, TEXT("Looping moving %s by %f"), *FBAUtils::GetNodeName(Node), LoopingDelta.GetValue());
						Formatter->SetNodeY_KeepingSpacingVisited(Node, Node->NodePosY + LoopingDelta.GetValue(), VisitedNodes);
					}

					// FBAUtils::PrintNodeArray(VisitedNodes.Array(), "Looping Moooved");
				}
			}
		}

		// Collide against other placed tracks
		for (auto Group : PlacedGroups)
		{
			FSlateRect GroupBounds = AllGroup->GetBounds();
			FSlateRect PlacedBounds = Group->GetBounds();
			// PlacedBounds.Bottom += TrackSpacing;

			float Delta = PlacedBounds.Bottom - GroupBounds.Top;
			if (FSlateRect::DoRectanglesIntersect(GroupBounds, PlacedBounds))
			{
				// UE_LOG(LogKnotTrackCreator, Error, TEXT("\tTESTINGMove Group by Delta %f"), Delta);

				if (CurrentTrack->bIsLoopingTrack)
				{
					TSet<UEdGraphNode*> RelatedNodes;
					for (auto Track : AllGroup->Tracks)
					{
						UEdGraphNode* ParentNode = Track->GetParentPin()->GetOwningNode();
						UEdGraphNode* LastNode = Track->GetLastPin()->GetOwningNode();
						RelatedNodes.Append(Formatter->GetRowAndChildren(ParentNode));
						RelatedNodes.Append(Formatter->GetRowAndChildren(LastNode));
					}

					// RELATIVE POS ENABLE THIS NODES
					for (auto Track : Group->Tracks)
					{
						Track->UpdateTrackHeight(Track->GetTrackHeight() - Delta);
					}

					for (auto Node : RelatedNodes)
					{
						Formatter->SetNodePos(Node, Node->NodePosX, Node->NodePosY + Delta);
					}
				}
				else
				{
					for (TSharedPtr<FKnotNodeTrack> KnotNodeTrack : AllGroup->Tracks)
					{
						KnotNodeTrack->UpdateTrackHeight(KnotNodeTrack->GetTrackHeight() + Delta);
					}
				}
			}
		}

		PlacedGroups.Add(AllGroup);

		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("TRACK GROUP START %d"), CurrentTrack->bIsLoopingTrack);
		// for (auto Track : AllGroup->Tracks)
		// {
		// 	UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%s"), *Track->ToString());
		// }
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("TRACK GROUP END"));

		for (TSharedPtr<FKnotNodeTrack> Track : PlacedTracks)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tPlaced track %s"), *Track->ToString());
			PendingTracks.Remove(Track);
		}

		TSet<UEdGraphNode*> TrackNodes = CurrentTrack->GetNodes(GraphHandler->GetFocusedEdGraph());

		FSlateRect ExpandedBounds = AllGroup->GetBounds();// OverlappingBounds;
		const float Padding = CurrentTrack->bIsLoopingTrack ? TrackSpacing * 2 : TrackSpacing;
		ExpandedBounds.Bottom += Padding;

		// collide against each track's related nodes
		TOptional<float> RelatedBottom;
		const FSlateRect ContractedBounds = ExpandedBounds.InsetBy(FMargin(16, 0)); // contract bounds in x slightly
		for (auto Track : AllGroup->Tracks)
		{
			for (UEdGraphNode* RelatedNode : Track->GetRelatedNodes())
			{
				// check bounds of each node for more accurate collision
				if (TSharedPtr<FFormatterInterface> ChildFormatter = Formatter->GetChildFormatter(RelatedNode))
				{
					UEdGraphNode* RootNode = ChildFormatter->GetRootNode();
					for (UEdGraphNode* FormattedNode : ChildFormatter->GetFormattedNodes())
					{
						// skip the root node
						if (FormattedNode == RootNode)
						{
							continue;
						}

						const FSlateRect NodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, FormattedNode);
						if (FSlateRect::DoRectanglesIntersect(NodeBounds, ContractedBounds))
						{
							RelatedBottom = RelatedBottom.IsSet() ? FMath::Max(NodeBounds.Bottom, RelatedBottom.GetValue()) : NodeBounds.Bottom;
						}
					}
				}
			}
		}

		// move expanded bounds down
		if (RelatedBottom.IsSet())
		{
			float RelatedDeltaY = (RelatedBottom.GetValue() + Padding) - ExpandedBounds.Top;
			ExpandedBounds = ExpandedBounds.OffsetBy(FVector2D(0, RelatedDeltaY));

			// move all tracks down
			for (TSharedPtr<FKnotNodeTrack> Track : AllGroup->Tracks)
			{
				Track->UpdateTrackHeight(Track->GetTrackHeight() + RelatedDeltaY);
			}
		}

		// find the top of the tallest node the track block is colliding with
		TOptional<float> CollisionTop;

		// collide against nodes
		for (UEdGraphNode* Node : Formatter->GetFormattedNodes())
		{
			bool bSkipNode = false;
			for (TSharedPtr<FKnotNodeTrack> Track : PlacedTracks)
			{
				if (Node == Track->GetParentPin()->GetOwningNode() || Node == Track->GetLastPin()->GetOwningNode())
				{
					// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Skipping node %s"), *FBAUtils::GetNodeName(Node));
					bSkipNode = true;
					break;
				}

				if (auto AlignedPin = Track->GetPinToAlignTo())
				{
					if (Node == AlignedPin->GetOwningNode())
					{
						// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Skipping node aligned %s"), *FBAUtils::GetNodeName(Node));
						bSkipNode = true;
						break;
					}
				}
			}

			if (bSkipNode && !CurrentTrack->bIsLoopingTrack)
			{
				continue;
			}

			const FSlateRect NodeBounds = GraphHandler->GetCachedNodeBounds(Node);
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Checking collision for %s | %s | %s"), *FBAUtils::GetNodeName(Node), *NodeBounds.ToString(), *ExpandedBounds.ToString());

			if (FSlateRect::DoRectanglesIntersect(NodeBounds, ExpandedBounds))
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tCollision with %s"), *FBAUtils::GetNodeName(Node));
				CollisionTop = CollisionTop.IsSet() ? FMath::Min(NodeBounds.Top, CollisionTop.GetValue()) : NodeBounds.Top;


				// if (CurrentTrack->bIsLoopingTrack)
				// {
				// 	float DeltaY = CollisionTop.GetValue() - ExpandedBounds.Bottom;
				// 	CurrentTrack->UpdateTrackHeight(CurrentTrack->GetTrackHeight() + DeltaY + 1);
				// }
				// else
				{
					float DeltaY = ExpandedBounds.Bottom - CollisionTop.GetValue();
					Formatter->SetNodeY_KeepingSpacing(Node, Node->NodePosY + DeltaY);
				}

				for (TSharedPtr<FKnotNodeTrack> Track : AllGroup->Tracks)
				{
					for (auto Creation : Track->KnotCreations)
					{
						RelativeCreationMapping.Add(Creation, Node);
					}
				}


				// if (CurrentTrack->bIsLoopingTrack)
				// {
				// 	Formatter->SetNodeY_KeepingSpacing(Node, Node->NodePosY + DeltaY);
				// 	UE_LOG(LogKnotTrackCreator, Error, TEXT("Moving looping track? %s"), *CurrentTrack->ToString());
				// 	// for (auto Track : AllGroup->Tracks)
				// 	// {
				// 	// 	Track->UpdateTrackHeight(Track->GetTrackHeight() - DeltaY);
				// 	// }
				// }
			}

			// ExpandedBounds = AllGroup->GetBounds();
			// ExpandedBounds.Bottom += Padding;
		}

		if (!CollisionTop.IsSet())
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t\tSkip no collision top"));
			continue;
		}
	}
}

void FKnotTrackCreator::RemoveUselessCreationNodes()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::RemoveUselessCreationNodes"), STAT_KnotTrackCreator_RemoveUselessCreationNodes, STATGROUP_BA_EdGraphFormatter);
	for (TSharedPtr<FKnotNodeTrack> Track : KnotTracks)
	{
		if (Track->bIsLoopingTrack)
		{
			continue;
		}

		TSharedPtr<FKnotNodeCreation> LastCreation;
		TArray<TSharedPtr<FKnotNodeCreation>> CreationsCopy = Track->KnotCreations;
		for (TSharedPtr<FKnotNodeCreation> Creation : CreationsCopy)
		{
			if (Track->KnotCreations.Num() == 1)
			{
				break;
			}

			const bool bHasOneConnection = Creation->PinHandlesToConnectTo.Num() == 1;
			if (bHasOneConnection)
			{
				const float PinHeight = GraphHandler->GetPinY(Creation->GetPinToConnectTo());
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Pin %s %f | %f"), *FBAUtils::GetPinName(MainPin), PinHeight, Track->GetTrackHeight());

				if (PinHeight == Track->GetTrackHeight())
				{
					if (LastCreation)
					{
						LastCreation->PinHandlesToConnectTo.Add(Creation->PinToConnectToHandle);
					}

					Track->KnotCreations.Remove(Creation);
				}
			}

			LastCreation = Creation;
		}
	}
}

void FKnotTrackCreator::RemoveKnotNodes(const TArray<UEdGraphNode*>& NodeTree)
{
	TArray<UEdGraphNode_Comment*> CommentNodes = FBAUtils::GetCommentNodesFromGraph(GraphHandler->GetFocusedEdGraph());
	for (UEdGraphNode* Node : NodeTree)
	{
		/** Delete all connections for each knot node */
		if (UK2Node_Knot* KnotNode = Cast<UK2Node_Knot>(Node))
		{
			FBAUtils::DisconnectKnotNode(KnotNode);

			for (auto Comment : CommentNodes)
			{
				if (Comment->GetNodesUnderComment().Contains(KnotNode))
				{
					FBAUtils::RemoveNodeFromComment(Comment, KnotNode);
				}
			}

			if (GetDefault<UBASettings>()->bUseKnotNodePool &&
				GetDefault<UBASettings>()->bCreateKnotNodes) // if we don't create knot nodes, no point reusing them
			{
				KnotNodePool.Add(KnotNode);
			}
			else
			{
				FBAUtils::DeleteNode(KnotNode);

				if (FCommentHandler* CH = Formatter->GetCommentHandler())
				{
					CH->DeleteNode(KnotNode);
				}
			}
		}
	}
}

UK2Node_Knot* FKnotTrackCreator::CreateKnotNode(FKnotNodeCreation* Creation, const FVector2D& Position, UEdGraphPin* ParentPin)
{
	if (!Creation)
	{
		return nullptr;
	}

	UK2Node_Knot* OptionalNodeToReuse = nullptr;
	if (GetDefault<UBASettings>()->bUseKnotNodePool && KnotNodePool.Num() > 0)
	{
		OptionalNodeToReuse = KnotNodePool.Pop();
	}
	else
	{
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Failed to find?"));
	}

	auto Graph = GraphHandler->GetFocusedEdGraph();
	UK2Node_Knot* CreatedNode = Creation->CreateKnotNode(Position, ParentPin, OptionalNodeToReuse, Graph);

	UEdGraphPin* MainPinToConnectTo = Creation->PinToConnectToHandle.GetPin();

	KnotNodeOwners.Add(CreatedNode, MainPinToConnectTo->GetOwningNode());
	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Created node %d for %s"), CreatedNode, *FBAUtils::GetNodeName(ParentPin->GetOwningNode()));

	return CreatedNode; //Creation->CreateKnotNode(Position, ParentPin, OptionalNodeToReuse, GraphHandler->GetFocusedEdGraph());
}

bool FKnotTrackCreator::TryAlignTrackToEndPins(TSharedPtr<FKnotNodeTrack> Track, const TArray<UEdGraphNode*>& AllNodes)
{
	const float ParentPinY = GraphHandler->GetPinY(Track->GetParentPin());
	const float LastPinY = GraphHandler->GetPinY(Track->GetLastPin());
	bool bPreferParentPin = ParentPinY > LastPinY;

	if (FBAUtils::IsExecPin(Track->GetParentPin()))
	{
		bPreferParentPin = true;
	}

	for (int i = 0; i < 2; ++i)
	{
		//FString PreferPinStr = bPreferParentPin ? "true" : "false";
		//UE_LOG(LogKnotTrackCreator, Warning, TEXT("AlignTrack ParentY %f (%s) | LastPinY %f (%s) | PreferParent %s"), ParentPinY, *FBAUtils::GetPinName(Track->GetParentPin()), LastPinY,
		//       *FBAUtils::GetPinName(Track->GetLastPin()), *PreferPinStr);

		if (i == 1)
		{
			bPreferParentPin = !bPreferParentPin;
		}

		UEdGraphPin* SourcePin = bPreferParentPin ? Track->GetParentPin() : Track->GetLastPin();
		UEdGraphPin* OtherPin = bPreferParentPin ? Track->GetLastPin() : Track->GetParentPin();

		const FVector2D SourcePinPos = FBAUtils::GetPinPos(GraphHandler, SourcePin);
		const FVector2D OtherPinPos = FBAUtils::GetPinPos(GraphHandler, OtherPin);

		const FVector2D Padding = FBAUtils::IsParameterPin(OtherPin)
			? PinPadding
			: NodePadding;

		const FVector2D Point
			= SourcePin->Direction == EGPD_Output
			? FVector2D(OtherPinPos.X - Padding.X, SourcePinPos.Y)
			: FVector2D(OtherPinPos.X + Padding.X, SourcePinPos.Y);

		// UE_LOG(LogKnotTrackCreator, Error, TEXT("Checking Point %s | %s"), *Point.ToString(), *FBAUtils::GetNodeName(SourcePin->GetOwningNode()));

		bool bAnyCollision = false;

		for (UEdGraphNode* NodeToCollisionCheck : AllNodes)
		{
			FSlateRect CollisionBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToCollisionCheck).ExtendBy(FMargin(0, TrackSpacing - 1));

			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Collision check against %s | %s | %s"), *FBAUtils::GetNodeName(NodeToCollisionCheck), *CollisionBounds.ToString(), *Point.ToString());

			if (NodeToCollisionCheck == SourcePin->GetOwningNode() || NodeToCollisionCheck == OtherPin->GetOwningNode())
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tSkipping node"));
				continue;
			}

			if (FBAUtils::LineRectIntersection(CollisionBounds, SourcePinPos, Point))
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tFound collision"));
				bAnyCollision = true;
				break;
			}
		}

		for (TSharedPtr<FKnotNodeTrack> OtherTrack : KnotTracks)
		{
			if (OtherTrack == Track)
			{
				continue;
			}

			// Possibly revert back to rect collision check
			if (FBAUtils::LineRectIntersection(OtherTrack->GetTrackBounds().ExtendBy(FMargin(0, TrackSpacing * 0.25f)), SourcePinPos, Point))
			{
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Track %s colliding with %s"), *Track->ToString(), *OtherTrack->ToString());
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tStart %s End %s"), *SourcePinPos.ToString(), *Point.ToString());
				// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tRect %s"), *MyTrackBounds.ToString());
				bAnyCollision = true;
				break;
			}
		}

		if (!bAnyCollision)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("sucessfully found easy solution!"));
			Track->PinAlignedX = Point.X;
			Track->UpdateTrackHeight(SourcePinPos.Y);
			Track->PinToAlignTo.SetPin(SourcePin);
			return true;
		}
	}

	return false;
}

bool FKnotTrackCreator::DoesPinNeedTrack(UEdGraphPin* Pin, const TArray<UEdGraphPin*>& LinkedTo)
{
	if (LinkedTo.Num() == 0)
	{
		return false;
	}

	// if the pin is linked to multiple linked nodes, we need a knot track
	if (LinkedTo.Num() > 1)
	{
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Multiple linked to?"));
		return true;
	}

	// otherwise the pin is linked to exactly 1 node, run a collision check
	UEdGraphPin* OtherPin = LinkedTo[0];

	// need pin if there are any collisions
	return AnyCollisionBetweenPins(Pin, OtherPin);
}

bool FKnotTrackCreator::AnyCollisionBetweenPins(UEdGraphPin* Pin, UEdGraphPin* OtherPin)
{
	TSet<UEdGraphNode*> FormattedNodes = Formatter->GetFormattedNodes();

	const FVector2D PinPos = FBAUtils::GetPinPos(GraphHandler, Pin);
	const FVector2D OtherPinPos = FBAUtils::GetPinPos(GraphHandler, OtherPin);

	// UE_LOG(LogTemp, Warning, TEXT("Checking pins %s %s"), *FBAUtils::GetPinName(Pin), *FBAUtils::GetPinName(OtherPin));

	return NodeCollisionBetweenLocation(PinPos, OtherPinPos, { Pin->GetOwningNode(), OtherPin->GetOwningNode() });
}

bool FKnotTrackCreator::NodeCollisionBetweenLocation(FVector2D Start, FVector2D End, TSet<UEdGraphNode*> IgnoredNodes)
{
	TSet<UEdGraphNode*> FormattedNodes = Formatter->GetFormattedNodes();

	for (UEdGraphNode* NodeToCollisionCheck : FormattedNodes)
	{
		if (IgnoredNodes.Contains(NodeToCollisionCheck))
		{
			continue;
		}

		FSlateRect NodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToCollisionCheck);//.ExtendBy(FMargin(0, TrackSpacing - 1));
		if (FBAUtils::LineRectIntersection(NodeBounds, Start, End))
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tNode collision! %s"), *FBAUtils::GetNodeName(NodeToCollisionCheck));
			return true;
		}
	}

	return false;
}

void FKnotTrackCreator::Reset()
{
	KnotNodesSet.Reset();
	KnotTracks.Reset();
	KnotNodeOwners.Reset();
}

void FKnotTrackCreator::AddNomadKnotsIntoComments()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::AddNomadKnotsIntoComments"), STAT_KnotTrackCreator_AddNomadKnotsIntoComments, STATGROUP_BA_EdGraphFormatter);
	FCommentHandler* CommentHandler = Formatter->GetCommentHandler();
	if (!CommentHandler)
	{
		return;
	}

	// Add nomad knots if they happen to be located in a comment
	for (UEdGraphNode* CreatedKnotNode : GetCreatedKnotNodes())
	{
		for (UEdGraphNode_Comment* Comment : CommentHandler->GetComments())
		{
			const FMargin Padding = CommentHandler->GetCommentPadding(Comment);
			auto CommentBounds = CommentHandler->GetCommentBounds(Comment).InsetBy(Padding);
			if (CommentBounds.ContainsPoint(FVector2D(CreatedKnotNode->NodePosX, CreatedKnotNode->NodePosY)))
			{
				// UE_LOG(LogTemp, Warning, TEXT("Added knot %s into %s"), *FBAUtils::GetNodeName(CreatedKnotNode), *FBAUtils::GetNodeName(Comment));
				CommentHandler->AddNodeIntoComment(Comment, CreatedKnotNode);
			}
		}
	}
}

void FKnotTrackCreator::MakeKnotTrack()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::MakeKnotTrack"), STAT_KnotTrackCreator_MakeKnotTrack, STATGROUP_BA_EdGraphFormatter);
	const TSet<UEdGraphNode*> FormattedNodes = Formatter->GetFormattedNodes();

	const auto& NotFormatted = [FormattedNodes](UEdGraphPin* Pin)
	{
		return !FormattedNodes.Contains(Pin->GetOwningNode());
	};

	// iterate across the pins of all nodes and determine if they require a knot track
	for (UEdGraphNode* MyNode : FormattedNodes)
	{
		// make tracks for input exec pins
		TArray<TSharedPtr<FKnotNodeTrack>> PreviousTracks;
		for (UEdGraphPin* MyPin : FBAUtils::GetExecPins(MyNode, EGPD_Input))
		{
			TArray<UEdGraphPin*> LinkedTo = MyPin->LinkedTo;
			LinkedTo.RemoveAll(NotFormatted);
			if (LinkedTo.Num() == 0)
			{
				continue;
			}

			if (GetDefault<UBASettings>()->ExecutionWiringStyle == EBAWiringStyle::AlwaysMerge)
			{
				MakeKnotTracksForLinkedExecPins(MyPin, LinkedTo, PreviousTracks);
			}
			else
			{
				for (UEdGraphPin* Pin : LinkedTo)
				{
					MakeKnotTracksForLinkedExecPins(MyPin, { Pin }, PreviousTracks);
				}
			}
		}
	}

	for (UEdGraphNode* MyNode : FormattedNodes)
	{
		// make tracks for output parameter pins
		TArray<TSharedPtr<FKnotNodeTrack>> PreviousTracks;
		for (UEdGraphPin* MyPin : FBAUtils::GetParameterPins(MyNode, EGPD_Output))
		{
			TArray<UEdGraphPin*> LinkedTo = MyPin->LinkedTo;
			LinkedTo.RemoveAll(NotFormatted);
			if (LinkedTo.Num() == 0)
			{
				continue;
			}

			if (GetDefault<UBASettings>()->ParameterWiringStyle == EBAWiringStyle::AlwaysMerge)
			{
				MakeKnotTracksForParameterPins(MyPin, LinkedTo, PreviousTracks);
			}
			else
			{
				for (UEdGraphPin* Pin : LinkedTo)
				{
					MakeKnotTracksForParameterPins(MyPin, { Pin }, PreviousTracks);
				}
			}
		}
	}
}

TSharedPtr<FKnotNodeTrack> FKnotTrackCreator::MakeKnotTracksForLinkedExecPins(UEdGraphPin* ParentPin, TArray<UEdGraphPin*> LinkedPins, TArray<TSharedPtr<FKnotNodeTrack>>& PreviousTracks)
{
	FVector2D ParentPinPos = FBAUtils::GetPinPos(GraphHandler, ParentPin);
	UEdGraphNode* ParentNode = ParentPin->GetOwningNode();

	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Processing knot track for parent pin %s"), *FBAUtils::GetPinName(ParentPin));
	// for (auto Pin : LinkedPins)
	// {
	// 	UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t%s"), *FBAUtils::GetPinName(Pin));
	// }

	// check for looping pins, these are pins where 
	// the x position of the pin is less than the x value of the parent pin
	TArray<UEdGraphPin*> LoopingPins;
	for (UEdGraphPin* LinkedPin : LinkedPins)
	{
		const FVector2D LinkedPinPos = FBAUtils::GetPinPos(GraphHandler, LinkedPin);
		if (LinkedPinPos.X > ParentPinPos.X)
		{
			LoopingPins.Add(LinkedPin);
		}
	}

	// create looping tracks
	for (UEdGraphPin* OtherPin : LoopingPins)
	{
		const float OtherNodeTop = FBAUtils::GetNodeBounds(OtherPin->GetOwningNode()).Top;
		const float MyNodeTop = FBAUtils::GetNodeBounds(ParentNode).Top;
		const float AboveNodeWithPadding = FMath::Min(OtherNodeTop, MyNodeTop) - TrackSpacing * 2;

		TArray<UEdGraphPin*> TrackPins = { OtherPin };
		TSharedPtr<FKnotNodeTrack> KnotTrack = MakeShared<FKnotNodeTrack>(Formatter, GraphHandler, ParentPin, TrackPins, AboveNodeWithPadding, true);
		KnotTracks.Add(KnotTrack);

		const FVector2D OtherPinPos = FBAUtils::GetPinPos(GraphHandler, OtherPin);

		const FVector2D FirstKnotPos(ParentPinPos.X + 20, KnotTrack->GetTrackHeight());
		TSharedPtr<FKnotNodeCreation> FirstLoopingKnot = MakeShared<FKnotNodeCreation>(KnotTrack, FirstKnotPos, nullptr, OtherPin);
		KnotTrack->KnotCreations.Add(FirstLoopingKnot);

		const FVector2D SecondKnotPos(OtherPinPos.X - 20, KnotTrack->GetTrackHeight());
		TSharedPtr<FKnotNodeCreation> SecondLoopingKnot = MakeShared<FKnotNodeCreation>(KnotTrack, SecondKnotPos, FirstLoopingKnot, OtherPin);
		KnotTrack->KnotCreations.Add(SecondLoopingKnot);
	}

	LinkedPins.RemoveAll([&LoopingPins](UEdGraphPin* Pin) { return LoopingPins.Contains(Pin); });

	// remove pins which are left or too close to my pin
	const float Threshold = ParentPinPos.X - NodePadding.X * 1.5f;
	TSharedPtr<FBAGraphHandler> GraphHandlerRef = GraphHandler;
	const auto& IsTooCloseToParent = [GraphHandlerRef, Threshold](UEdGraphPin* Pin)
	{
		const FVector2D PinPos = FBAUtils::GetPinPos(GraphHandlerRef, Pin);
		return PinPos.X > Threshold;
	};
	LinkedPins.RemoveAll(IsTooCloseToParent);

	if (LinkedPins.Num() == 0)
	{
		return nullptr;
	}

	// sort pins by node's highest x position first then highest y position
	const auto RightTop = [](const UEdGraphPin& PinA, const UEdGraphPin& PinB)
	{
		if (PinA.GetOwningNode()->NodePosX == PinB.GetOwningNode()->NodePosX)
		{
			return PinA.GetOwningNode()->NodePosY > PinB.GetOwningNode()->NodePosY;
		}

		return PinA.GetOwningNode()->NodePosX > PinB.GetOwningNode()->NodePosX;
	};

	LinkedPins.Sort(RightTop);

	const FVector2D LastPinPos = FBAUtils::GetPinPos(GraphHandler, LinkedPins.Last());

	const float DistX = FMath::Abs(ParentPinPos.X - LastPinPos.X);
	const float DistY = FMath::Abs(ParentPinPos.Y - LastPinPos.Y);

	// skip if we are the same height as our parent pin and we only have 1 pin
	const bool bSkipSameHeight = LinkedPins.Num() == 1 && DistY <= 5.0f; 

	// skip the pin distance check if we are expanding by height
	const bool bPinReallyFar = DistX > GetDefault<UBASettings>()->KnotNodeDistanceThreshold && !GetDefault<UBASettings>()->bExpandNodesByHeight;

	const bool bPinNeedsTrack = DoesPinNeedTrack(ParentPin, LinkedPins);

	const bool bPreviousHasTrack = PreviousTracks.Num() > 0;

	const FVector2D ToLast = LastPinPos - ParentPinPos;
	const bool bTooSteep = FMath::Abs(ToLast.Y) / FMath::Abs(ToLast.X) >= 2.75f;
	if (bTooSteep)
	{
		return nullptr;
	}

	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Need reroute: %d %d %d %d %f %f"), bSkipSameHeight, bPinReallyFar, bPreviousHasTrack, bPinNeedsTrack, DistX, DistY);

	const bool bNeedsReroute = (bPinReallyFar && !bSkipSameHeight) || bPinNeedsTrack || bPreviousHasTrack;
	if (!bNeedsReroute)
	{
		return nullptr;
	}

	TSharedPtr<FKnotNodeTrack> KnotTrack = MakeShared<FKnotNodeTrack>(Formatter, GraphHandler, ParentPin, LinkedPins, ParentPinPos.Y, false);

	TryAlignTrackToEndPins(KnotTrack, Formatter->GetFormattedNodes().Array());

	// remove the first linked pins which has the same height and no collision
	const bool bSameHeightAsParentPin = FMath::Abs(KnotTrack->GetTrackHeight() - ParentPinPos.Y) < 5.f;
	if (!bSameHeightAsParentPin)
	{
		for (UEdGraphPin* LinkedPin : LinkedPins)
		{
			const FVector2D LinkedPinPos = FBAUtils::GetPinPos(GraphHandler, LinkedPin);

			const bool bSameHeight = FMath::Abs(LinkedPinPos.Y - ParentPinPos.Y) < 5.f;
			if (bSameHeight && !AnyCollisionBetweenPins(ParentPin, LinkedPin))
			{
				KnotTrack->LinkedTo.Remove(LinkedPin);
				break;
			}
		}
	}

	if (KnotTrack->LinkedTo.Num() == 0)
	{
		return nullptr;
	}

	KnotTracks.Add(KnotTrack);

	// if the track is not at the same height as the pin, then we need an
	// initial knot right of the inital pin, at the track height
	const FVector2D MyKnotPos = FVector2D(ParentPinPos.X - NodePadding.X, KnotTrack->GetTrackHeight());
	TSharedPtr<FKnotNodeCreation> PreviousKnot = MakeShared<FKnotNodeCreation>(KnotTrack, MyKnotPos, nullptr, KnotTrack->GetParentPin());
	KnotTrack->KnotCreations.Add(PreviousKnot);

	// create a knot node for each of the pins remaining in linked to
	for (FBANodePinHandle& OtherPinHandle : KnotTrack->LinkedTo)
	{
		UEdGraphPin* OtherPin = OtherPinHandle.GetPin();
		ParentPin->BreakLinkTo(OtherPin);

		const FVector2D OtherPinPos = FBAUtils::GetPinPos(GraphHandler, OtherPin);
		const float KnotX = FMath::Min(OtherPinPos.X + NodePadding.X, ParentPinPos.X - NodePadding.X);
		const FVector2D KnotPos(KnotX, KnotTrack->GetTrackHeight());

		// if the x position is very close to the previous knot's x position, 
		// we should not need to create a new knot instead we merge the locations
		if (PreviousKnot.IsValid() && FMath::Abs(KnotX - PreviousKnot->KnotPos.X) < 50)
		{
			PreviousKnot->KnotPos.X = KnotX;
			PreviousKnot->PinHandlesToConnectTo.Add(OtherPin);
			continue;
		}

		PreviousKnot = MakeShared<FKnotNodeCreation>(KnotTrack, KnotPos, PreviousKnot, OtherPin);
		KnotTrack->KnotCreations.Add(PreviousKnot);
	}

	PreviousTracks.Add(KnotTrack);

	return KnotTrack;
}

TSharedPtr<FKnotNodeTrack> FKnotTrackCreator::MakeKnotTracksForParameterPins(UEdGraphPin* ParentPin, TArray<UEdGraphPin*> LinkedPins, TArray<TSharedPtr<FKnotNodeTrack>>& PreviousTracks)
{
	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Make knot tracks for parameter pin %s"), *FBAUtils::GetPinName(ParentPin));

	FVector2D ParentPinPos = FBAUtils::GetPinPos(GraphHandler, ParentPin);

	// remove pins which are left or too close to my pin
	const float Threshold = ParentPinPos.X + NodePadding.X * 2.0f;
	TSharedPtr<FBAGraphHandler> GraphHandlerRef = GraphHandler;

	const auto& IsTooCloseToParent = [GraphHandlerRef, Threshold](UEdGraphPin* Pin)
	{
		const FVector2D PinPos = FBAUtils::GetPinPos(GraphHandlerRef, Pin);
		return PinPos.X < Threshold;
	};

	LinkedPins.RemoveAll(IsTooCloseToParent);

	if (LinkedPins.Num() == 0)
	{
		return nullptr;
	}

	const auto LeftTop = [](const UEdGraphPin& PinA, const UEdGraphPin& PinB)
	{
		if (PinA.GetOwningNode()->NodePosX == PinB.GetOwningNode()->NodePosX)
		{
			return PinA.GetOwningNode()->NodePosY > PinB.GetOwningNode()->NodePosY;
		}

		return PinA.GetOwningNode()->NodePosX < PinB.GetOwningNode()->NodePosX;
	};
	LinkedPins.Sort(LeftTop);

	const FVector2D LastPinPos = FBAUtils::GetPinPos(GraphHandler, LinkedPins.Last());

	const float Dist = FMath::Abs(ParentPinPos.X - LastPinPos.X);

	const bool bLastPinFarAway = Dist > GetDefault<UBASettings>()->KnotNodeDistanceThreshold && !GetDefault<UBASettings>()->bExpandNodesByHeight;

	const bool bPinNeedsTrack = DoesPinNeedTrack(ParentPin, LinkedPins);

	const bool bPreviousHasTrack = PreviousTracks.Num() > 0;

	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Needs track: %d %d %d"), bLastPinFarAway, bPreviousHasTrack, bPinNeedsTrack);

	const FVector2D ToLast = LastPinPos - ParentPinPos;
	const bool bTooSteep = FMath::Abs(ToLast.Y) / FMath::Abs(ToLast.X) >= 2.75f;
	if (bTooSteep)
	{
		return nullptr;
	}

	const bool bNeedsReroute = bPinNeedsTrack || bPreviousHasTrack || bLastPinFarAway;
	if (!bNeedsReroute)
	{
		return nullptr;
	}

	// init the knot track
	TSharedPtr<FKnotNodeTrack> KnotTrack = MakeShared<FKnotNodeTrack>(Formatter, GraphHandler, ParentPin, LinkedPins, ParentPinPos.Y, false);

	// check if the track height can simply be set to one of it's pin's height
	if (TryAlignTrackToEndPins(KnotTrack, Formatter->GetFormattedNodes().Array()))
	{
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Found a pin to align to for %s"), *FBAUtils::GetPinName(KnotTrack->GetParentPin()));
	}
	else
	{
		// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Failed to find pin to align to"));
	}

	// remove any pins which has the same height and no collision
	for (UEdGraphPin* LinkedPin : LinkedPins)
	{
		const FVector2D LinkedPinPos = FBAUtils::GetPinPos(GraphHandler, LinkedPin);

		const bool bSameHeight = FMath::Abs(LinkedPinPos.Y - ParentPinPos.Y) < 5.f;
		if (bSameHeight && !AnyCollisionBetweenPins(ParentPin, LinkedPin))
		{
			KnotTrack->LinkedTo.Remove(LinkedPin);
		}
	}

	if (KnotTrack->LinkedTo.Num() == 0)
	{
		return nullptr;
	}

	// Add a knot creation which links to the parent pin
	const FVector2D InitialKnotPos = FVector2D(ParentPinPos.X + PinPadding.X, KnotTrack->GetTrackHeight());
	TSharedPtr<FKnotNodeCreation> PreviousKnot = MakeShared<FKnotNodeCreation>(KnotTrack, InitialKnotPos, nullptr, KnotTrack->GetParentPin());
	ParentPin->BreakLinkTo(KnotTrack->GetLastPin());
	KnotTrack->KnotCreations.Add(PreviousKnot);

	for (FBANodePinHandle& OtherPinHandle : KnotTrack->LinkedTo)
	{
		UEdGraphPin* OtherPin = OtherPinHandle.GetPin();

		// break link to parent pin
		ParentPin->BreakLinkTo(OtherPin);

		const FVector2D OtherPinPos = FBAUtils::GetPinPos(GraphHandler, OtherPin);
		const float KnotX = FMath::Max(OtherPinPos.X - PinPadding.X, ParentPinPos.X + PinPadding.X);

		const FVector2D KnotPos = FVector2D(KnotX, KnotTrack->GetTrackHeight());

		// if the x position is very close to the previous knot's x position, 
		// we should not need to create a new knot instead we merge the locations
		if (PreviousKnot.IsValid() && FMath::Abs(KnotX - PreviousKnot->KnotPos.X) < 50)
		{
			PreviousKnot->KnotPos.X = KnotX;
			PreviousKnot->PinHandlesToConnectTo.Add(OtherPin);
			continue;
		}

		// Add a knot creation for each linked pin
		PreviousKnot = MakeShared<FKnotNodeCreation>(KnotTrack, KnotPos, PreviousKnot, OtherPin);
		KnotTrack->KnotCreations.Add(PreviousKnot);
	}

	PreviousTracks.Add(KnotTrack);

	KnotTracks.Add(KnotTrack);

	return KnotTrack;
}

void FKnotTrackCreator::MergeNearbyKnotTracks()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::MergeNearbyKnotTracks"), STAT_KnotTrackCreator_MergeNearbyKnotTracks, STATGROUP_BA_EdGraphFormatter);
	// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Merging knot track"));

	TArray<TSharedPtr<FKnotNodeTrack>> PendingTracks = KnotTracks;

	if (GetDefault<UBASettings>()->ExecutionWiringStyle != EBAWiringStyle::MergeWhenNear)
	{
		PendingTracks.RemoveAll([](TSharedPtr<FKnotNodeTrack> Track)
		{
			return FBAUtils::IsExecPin(Track->GetParentPin());
		});
	}

	if (GetDefault<UBASettings>()->ParameterWiringStyle != EBAWiringStyle::MergeWhenNear)
	{
		PendingTracks.RemoveAll([](TSharedPtr<FKnotNodeTrack> Track)
		{
			return FBAUtils::IsParameterPin(Track->GetParentPin());
		});
	}

	// TODO: Handle merging of looping tracks
	PendingTracks.RemoveAll([](TSharedPtr<FKnotNodeTrack> Track)
	{
		return Track->bIsLoopingTrack;
	});

	while (PendingTracks.Num() > 0)
	{
		auto CurrentTrack = PendingTracks.Pop();
		auto Tracks = PendingTracks;

		for (TSharedPtr<FKnotNodeTrack> Track : Tracks)
		{
			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Merging track %s"), *FBAUtils::GetPinName(Track->GetParentPin()));

			// merge if they have the same parent and same height
			if (Track->GetParentPin() == CurrentTrack->GetParentPin() &&
				Track->GetTrackHeight() == CurrentTrack->GetTrackHeight())
			{
				for (TSharedPtr<FKnotNodeCreation> Creation : Track->KnotCreations)
				{
					bool bShouldAddCreation = true;
					for (TSharedPtr<FKnotNodeCreation> CurrentCreation : CurrentTrack->KnotCreations)
					{
						if (FMath::Abs(CurrentCreation->KnotPos.X - Creation->KnotPos.X) < 50)
						{
							bShouldAddCreation = false;
							CurrentCreation->PinHandlesToConnectTo.Append(Creation->PinHandlesToConnectTo);
						}
					}

					if (bShouldAddCreation)
					{
						CurrentTrack->KnotCreations.Add(Creation);
						CurrentTrack->PinToAlignTo.SetPin(nullptr);

						// UE_LOG(LogKnotTrackCreator, Warning, TEXT("Cancelled pin to align to for track %s"), *FBAUtils::GetPinName(CurrentTrack->GetParentPin()));
					}
				}

				KnotTracks.Remove(Track);
				PendingTracks.Remove(Track);
			}
		}
	}
}

void FKnotTrackCreator::AddKnotNodesToComments()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FKnotTrackCreator::AddKnotNodesToComments"), STAT_KnotTrackCreator_AddKnotNodesToComments, STATGROUP_BA_EdGraphFormatter);

	FCommentHandler* CommentHandler = Formatter->GetCommentHandler();
	if (!CommentHandler)
	{
		return;
	}

	if (CommentHandler->GetComments().Num() == 0)
	{
		return;
	}

	// Add knots if their parents are both contained in the comment
	for (TSharedPtr<FKnotNodeTrack> Track : KnotTracks)
	{
		TArray<UEdGraphNode*> TrackNodes = Track->GetNodes(GraphHandler->GetFocusedEdGraph()).Array();

		int NumKnots = 0;
		UK2Node_Knot* SingleKnot = nullptr;
		for (auto Creation : Track->KnotCreations)
		{
			if (Creation->CreatedKnot != nullptr)
			{
				NumKnots += 1;
				SingleKnot = Creation->CreatedKnot;
			}
		}

		for (const auto& Comment : CommentHandler->GetComments())
		{
			TArray<UEdGraphNode*> Containing = CommentHandler->GetNodesUnderComments(Comment);
			FSlateRect CommentBounds = CommentHandler->GetCommentBounds(Comment, nullptr); // .ExtendBy(30);

			bool bContainsSingleKnot = NumKnots == 1 && CommentBounds.ContainsPoint(FVector2D(SingleKnot->NodePosX, SingleKnot->NodePosY));
			const bool bContainsAllNodes = FBAUtils::DoesArrayContainsAllItems(Containing, TrackNodes);

			// UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t\tAllKnots %d | AllNodes %d"), bContainsAllKnots, bContainsAllNodes);

			if (bContainsAllNodes)
			{
				if (!(NumKnots == 1 && bContainsSingleKnot))
				{
					auto NodesUnderComment = Comment->GetNodesUnderComment();
					for (auto Creation : Track->KnotCreations)
					{
						if (!NodesUnderComment.Contains(Creation->CreatedKnot))
						{
							// if we should ignore this comment then don't track the comment in the comment handler
							if (CommentHandler->ShouldIgnoreComment(Comment))
							{
								Comment->AddNodeUnderComment(Creation->CreatedKnot);
							}
							else
							{
								CommentHandler->AddNodeIntoComment(Comment, Creation->CreatedKnot);
							}
							KnotsInComments.Add(Creation->CreatedKnot);
						}
					}
				}
			}
		}
	}
}

void FKnotTrackCreator::PrintKnotTracks()
{
	UE_LOG(LogKnotTrackCreator, Warning, TEXT("### All Knot Tracks"));
	for (TSharedPtr<FKnotNodeTrack> Track : KnotTracks)
	{
		FString Aligned = Track->GetPinToAlignTo() != nullptr ? FString("True") : FString("False");
		FString Looping = Track->bIsLoopingTrack ? FString("True") : FString("False");
		UE_LOG(LogKnotTrackCreator, Warning, TEXT("\tKnot Tracks (%d) %s | %s | %s | %s | Aligned %s (%s) | Looping %s"),
			Track->KnotCreations.Num(),
			*FBAUtils::GetPinName(Track->GetParentPin()),
			*FBAUtils::GetNodeName(Track->GetParentPin()->GetOwningNodeUnchecked()),
			*FBAUtils::GetPinName(Track->GetLastPin()),
			*FBAUtils::GetNodeName(Track->GetLastPin()->GetOwningNodeUnchecked()),
			*Aligned, *FBAUtils::GetPinName(Track->GetPinToAlignTo()),
			*Looping);

		for (TSharedPtr<FKnotNodeCreation> Elem : Track->KnotCreations)
		{
			if (auto MyPin = Elem->PinToConnectToHandle.GetPin())
			{
				UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t\t%s %s"), *FBAUtils::GetPinName(MyPin), *Elem->KnotPos.ToString());
			}

			for (auto PinHandle : Elem->PinHandlesToConnectTo)
			{
				if (auto MyPin = PinHandle.GetPin())
				{
					UE_LOG(LogKnotTrackCreator, Warning, TEXT("\t\t\t%s"), *FBAUtils::GetPinName(MyPin));
				}
			}
		}
	}
}

bool FKnotTrackCreator::IsPinAlignedKnot(const UK2Node_Knot* KnotNode)
{
	return PinAlignedKnots.Contains(KnotNode);
}

TSharedPtr<FGroupedTracks> FKnotTrackCreator::GetKnotGroup(const UK2Node_Knot* KnotNode)
{
	return KnotTrackGroup.FindRef(KnotNode);
}
