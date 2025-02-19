// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "KnotTrack.h"

struct FCommentHandler;
struct FPinLink;

class FKnotTrackCreator final
{
	TSharedPtr<FFormatterInterface> Formatter;
	TSharedPtr<FBAGraphHandler> GraphHandler;
	TSet<UEdGraphNode*> KnotNodesSet;
	TArray<TSharedPtr<FKnotNodeTrack>> KnotTracks;
	TArray<UK2Node_Knot*> KnotNodePool;
	TMap<UK2Node_Knot*, UEdGraphNode*> KnotNodeOwners;
	TSet<UK2Node_Knot*> PinAlignedKnots;
	TSet<UK2Node_Knot*> KnotsInComments;

	TMap<UK2Node_Knot*, TSharedPtr<FGroupedTracks>> KnotTrackGroup; 
	TMap<UK2Node_Knot*, TSharedPtr<FKnotNodeCreation>> KnotCreationMap; 
	TArray<TSharedPtr<FGroupedTracks>> TrackGroups;

	FVector2D PinPadding;
	FVector2D NodePadding;
	float TrackSpacing;

public:
	TMap<UEdGraphNode*, TArray<UEdGraphNode*>> RelativeMapping;
	TMap<TSharedPtr<FKnotNodeCreation>, UEdGraphNode*> RelativeCreationMapping;

	FKnotTrackCreator() = default;
	void Init(TSharedPtr<FFormatterInterface> InFormatter, TSharedPtr<FBAGraphHandler> InGraphHandler);

	void FormatKnotNodes();
	void RemoveKnotNodes(const TArray<UEdGraphNode*>& NodeTree);
	const TSet<UEdGraphNode*>& GetCreatedKnotNodes() { return KnotNodesSet; }
	void Reset();

	bool IsPinAlignedKnot(const UK2Node_Knot* KnotNode);
	TSharedPtr<FGroupedTracks> GetKnotGroup(const UK2Node_Knot* KnotNode);

	bool IsKnotInsideComment(const UK2Node_Knot* KnotNode) const { return KnotsInComments.Contains(KnotNode); }
	TSharedPtr<FKnotNodeCreation> GetKnotCreation(const UK2Node_Knot* KnotNode) { return KnotCreationMap.FindRef(KnotNode); }

	void AddNomadKnotsIntoComments();

private:
	void MakeKnotTrack();

	TSharedPtr<FKnotNodeTrack> MakeKnotTracksForLinkedExecPins(UEdGraphPin* ParentPin, TArray<UEdGraphPin*> LinkedPins, TArray<TSharedPtr<FKnotNodeTrack>>& PreviousTracks);

	TSharedPtr<FKnotNodeTrack> MakeKnotTracksForParameterPins(UEdGraphPin* ParentPin, TArray<UEdGraphPin*> LinkedPins, TArray<TSharedPtr<FKnotNodeTrack>>& PreviousTracks);

	void MergeNearbyKnotTracks();

	void ExpandKnotTracks();

	void RemoveUselessCreationNodes();

	void CreateKnotTracks();

	bool TryAlignTrackToEndPins(TSharedPtr<FKnotNodeTrack> Track, const TArray<UEdGraphNode*>& AllNodes);

	bool DoesPinNeedTrack(UEdGraphPin* Pin, const TArray<UEdGraphPin*>& LinkedTo);

	bool AnyCollisionBetweenPins(UEdGraphPin* Pin, UEdGraphPin* OtherPin);

	bool NodeCollisionBetweenLocation(FVector2D Start, FVector2D End, TSet<UEdGraphNode*> IgnoredNodes);

	UK2Node_Knot* CreateKnotNode(FKnotNodeCreation* Creation, const FVector2D& Position, UEdGraphPin* ParentPin);

	void AddKnotNodesToComments();

	void PrintKnotTracks();
};
