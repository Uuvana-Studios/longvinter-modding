// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistCommentHandler.h"
#include "BlueprintAssistSettings.h"
#include "FormatterInterface.h"
#include "GraphFormatterTypes.h"
#include "EdGraph/EdGraphNode.h"
#include "KnotTrack/KnotTrackCreator.h"

struct FKnotNodeCreation;
class FEdGraphParameterFormatter;
class FBAGraphHandler;
struct FKnotNodeTrack;
struct FNodeInfo;
class FSlateRect;
class UEdGraphNode_Comment;
struct FBACommentContainsNode;

struct FNodeChangeInfo
{
	bool bIsNodeToKeepStill;
	UEdGraphNode* Node;
	TArray<FPinLink> Links;
	int32 NodeX;
	int32 NodeY;

	int32 NodeOffsetX;
	int32 NodeOffsetY;

	FNodeChangeInfo(UEdGraphNode* Node, UEdGraphNode* NodeToKeepStill);

	void UpdateValues(UEdGraphNode* NodeToKeepStill);

	bool HasChanged(UEdGraphNode* NodeToKeepStill);
};

struct ChildBranch
{
	UEdGraphPin* Pin;
	UEdGraphPin* ParentPin;
	TSet<UEdGraphNode*> BranchNodes;

	ChildBranch(UEdGraphPin* InPin, UEdGraphPin* InParentPin, TSet<UEdGraphNode*>& InBranchNodes)
		: Pin(InPin)
		, ParentPin(InParentPin)
		, BranchNodes(InBranchNodes) { }

	bool operator==(const ChildBranch& Other) const
	{
		return FPinLink(Pin, ParentPin) == FPinLink(Other.Pin, Other.ParentPin);
	}

	FString ToString() const;
};

class FEdGraphFormatter final
	: public FFormatterInterface
{
	TSharedPtr<FBAGraphHandler> GraphHandler;
	UEdGraphNode* RootNode;
	virtual UEdGraphNode* GetRootNode() override { return RootNode; }
	FEdGraphFormatterParameters FormatterParameters;
	FKnotTrackCreator KnotTrackCreator;
	FCommentHandler CommentHandler;

public:
	FEdGraphFormatter(
		TSharedPtr<FBAGraphHandler> InGraphHandler,
		FEdGraphFormatterParameters InFormatterParameters);

	virtual ~FEdGraphFormatter() override { }

	virtual void FormatNode(UEdGraphNode* Node) override;

	virtual TSet<UEdGraphNode*> GetFormattedNodes() override;

	virtual FCommentHandler* GetCommentHandler() override { return &CommentHandler; }

	TArray<UEdGraphNode*> GetNodePool() const { return NodePool; }

	UEdGraphNode* GetRootNode() const { return RootNode; }

	TSet<UEdGraphNode*> GetFormattedGraphNodes();

	void RefreshParameters(UEdGraphNode* Node);

	virtual const FEdGraphFormatterParameters& GetFormatterParameters() override { return FormatterParameters; }

	TSharedPtr<FEdGraphParameterFormatter> GetParameterFormatter(UEdGraphNode* Node);

	virtual TSharedPtr<FFormatterInterface> GetChildFormatter(UEdGraphNode* Node) override;

	virtual TArray<TSharedPtr<FFormatterInterface>> GetChildFormatters() override;

	virtual FBAFormatterSettings GetFormatterSettings() override;

	virtual void SetNodePos(UEdGraphNode* Node, const int X, const int Y) override;

	virtual void SetNodeY_KeepingSpacingVisited(UEdGraphNode* Node, float NewPosY, TSet<UEdGraphNode*>& VisitedNodes) override;

	virtual FSlateRect GetClusterBounds(UEdGraphNode* Node) override;

	virtual UEdGraphNode* GetClusterRootNode(UEdGraphNode* ChildNode) override;

	virtual TSet<UEdGraphNode*> GetRowAndChildren(UEdGraphNode* Node) override;

	virtual bool ShouldIgnoreComment(TSharedPtr<FBACommentContainsNode> Comment) override;

private:
	FVector2D PinPadding;
	FVector2D NodePadding;
	float TrackSpacing;
	float VerticalPinSpacing;
	bool bCenterBranches;
	int NumRequiredBranches;

	TArray<UEdGraphNode*> NodePool;
	TArray<UEdGraphNode*> NodeTree;

	TMap<UEdGraphNode*, TSharedPtr<FEdGraphParameterFormatter>> ParameterFormatterMap;

	UEdGraphNode* NodeToKeepStill = nullptr;
	int32 LastFormattedX;
	int32 LastFormattedY;

	TMap<UEdGraphNode*, FVector2D> LastNodePositions;

	TMap<UEdGraphNode*, FNodeChangeInfo> NodeChangeInfos;

	TMap<UEdGraphNode*, TSharedPtr<FFormatXInfo>> FormatXInfoMap;

	TArray<FPinLink> Path;

	TSharedPtr<FEdGraphParameterFormatter> MainParameterFormatter;

	TMap<FPinLink, bool> SameRowMapping;
	TMap<FBAGraphPinHandle, FBAGraphPinHandle> SameRowMappingDirect;

	TMap<UEdGraphNode*, TSharedPtr<FEdGraphParameterFormatter>> ParameterParentMap;

	TArray<TSharedPtr<FFormatXInfo>> NodesToExpand;

	FNodeRelativeMapping NodeRelativeMapping;

	void ExpandPendingNodes(bool bUseParameter);

	void SimpleRelativeFormatting();

	bool IsFormattingRequired(const TArray<UEdGraphNode*>& NewNodeTree);

	void SaveFormattingEndInfo();

	TArray<UEdGraphNode*> GetNodeTree(UEdGraphNode* InitialNode) const;

	bool IsInitialNodeValid(UEdGraphNode* Node) const;

	void InitNodePool();

	void ExpandByHeight();

	void ExpandNodesAheadOfParameters();

	void FormatX(bool bUseParameter);

	void FormatY_Recursive(
		const FPinLink& CurrentLink,
		TSet<UEdGraphNode*>& NodesToCollisionCheck,
		TSet<FPinLink>& VisitedLinks,
		bool bSameRow,
		TSet<UEdGraphNode*>& Children);

	void FormatY();

	void CenterBranches(UEdGraphNode* CurrentNode, TArray<ChildBranch>& ChildBranches, TSet<UEdGraphNode*>& NodesToCollisionCheck);

	bool AnyCollisionBetweenPins(UEdGraphPin* Pin, UEdGraphPin* OtherPin);

	bool NodeCollisionBetweenLocation(FVector2D Start, FVector2D End, TSet<UEdGraphNode*> IgnoredNodes);

	void FormatParameterNodes();

	void ResetRelativeToNodeToKeepStill(const FVector2D& SavedLocation);

	void RemoveKnotNodes();

	void GetPinsOfSameHeight();
	void GetPinsOfSameHeight_Recursive(
		UEdGraphNode* CurrentNode,
		UEdGraphPin* CurrentPin,
		UEdGraphPin* ParentPin,
		TSet<UEdGraphNode*>& NodesToCollisionCheck,
		TSet<FPinLink>& VisitedLinks);

	bool LinkToSort(UEdGraphPin& PinA, UEdGraphPin& PinB, TSet<UEdGraphNode*>& VisitedNodes);

	void WrapNodes();

	void ApplyCommentPaddingX();

	void ApplyCommentPaddingX_Recursive(
		TArray<UEdGraphNode*> NodeSet,
		TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes,
		TArray<FPinLink>& OutLeafLinks);

	void ApplyCommentPaddingY();
	void ApplyCommentPaddingY_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes);

	void ApplyCommentPaddingAfterKnots();
	void ApplyCommentPaddingAfterKnots_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes);

	void StraightenRow(UEdGraphNode* Node);
	void StraightenRowWithFilter(UEdGraphNode* Node, TFunctionRef<bool(const FPinLink&)> Pred);

	bool IsSameRow(const FPinLink& PinLink);
	bool IsSameRow(UEdGraphNode* NodeA, UEdGraphNode* NodeB);

	TArray<UEdGraphNode*> GetNodesInRow(UEdGraphNode* Node);

	bool AreCommentsIntersecting(UEdGraphNode_Comment* CommentA, UEdGraphNode_Comment* CommentB);

	TSharedPtr<FEdGraphParameterFormatter> GetParameterParent(UEdGraphNode* Node);

	TSharedPtr<FFormatXInfo> GetFormatXInfo(UEdGraphNode* Node);

	TArray<UEdGraphNode*> GetChildTree(TSharedPtr<FFormatXInfo> FormatXInfo);

	TArray<UEdGraphNode*> GetSameRowNodes(UEdGraphNode* Node);

	float DecideNewParent(UEdGraphNode* Node, UEdGraphNode* NewParent);

	void MoveChildrenX_Recursive(TSharedPtr<FFormatXInfo> Node, float DeltaX, TSet<TSharedPtr<FFormatXInfo>>& Visited);

protected:
	int32 GetChildX(const FPinLink& Link, bool bUseClusterNodes);
	int32 GetChildX(UEdGraphNode* From, UEdGraphNode* To, EEdGraphPinDirection Direction, bool bUseClusterNodes);

	FSlateRect GetClusterBoundsForNodes(const TArray<UEdGraphNode*>& Nodes);

	FSlateRect GetNodeBounds(UEdGraphNode* Node, bool bUseClusterBounds);
	FSlateRect GetNodeArrayBounds(const TArray<UEdGraphNode*>& Nodes, bool bUseClusterBounds);

	// FSlateRect GetCommentNodeSetBounds(UEdGraphNode_Comment* CommentNode, TArray<UEdGraphNode*> NodeSet, bool bAddParameterNodes = true, UEdGraphNode* NodeAsking = nullptr);

	friend class FEdGraphParameterFormatter;
	friend struct FNodeInfo;
	friend struct FKnotNodeTrack;
};
