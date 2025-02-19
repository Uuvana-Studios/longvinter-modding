// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Layout/SlateRect.h"

class UEdGraphNode;
class FBAGraphHandler;
class UEdGraphNode_Comment;

struct BLUEPRINTASSIST_API FBACommentContainsNode : public TSharedFromThis<FBACommentContainsNode>
{
	UEdGraphNode_Comment* Comment = nullptr;
	TArray<TSharedPtr<FBACommentContainsNode>> Parents;
	TArray<TSharedPtr<FBACommentContainsNode>> Children;
	TArray<UEdGraphNode*> AllContainedNodesWithComments;
	TArray<UEdGraphNode*> AllContainedNodes;
	TArray<UEdGraphNode*> OwnedNodes;
	int Height = -1;

	TArray<UEdGraphNode*> GetAllOwnedNodesWithoutComments();

	void GetSubTree(TSet<UEdGraphNode*>& OutNodes, const bool bIncludeSelf = true);
	FString ToString();

	bool Traverse_DepthFirst(const TFunctionRef<bool(FBACommentContainsNode*)>& Predicate)
	{
		if (!Predicate(this))
		{
			return false;
		}

		for (const TSharedPtr<FBACommentContainsNode>& Child : Children)
		{
			Child->Traverse_DepthFirst(Predicate);
		}

		return true;
	}
};

struct BLUEPRINTASSIST_API FBACommentContainsGraph
	: public TSharedFromThis<FBACommentContainsGraph>
{
	using FContainsNodeSet = TSet<TSharedPtr<FBACommentContainsNode>>;
	using FContainsNodeArray = TArray<TSharedPtr<FBACommentContainsNode>>;

	TSharedPtr<FBAGraphHandler> GraphHandler;
	FVector2D Padding;

	TSet<UEdGraphNode_Comment*> Comments;
	FContainsNodeArray SortedCommentNodes;
	FContainsNodeSet RootNodes;

	TMap<UEdGraphNode_Comment*, TSharedPtr<FBACommentContainsNode>> ContainsGraph;
	TMap<UEdGraphNode*, FContainsNodeArray> NodeContainingMap;

	void Init(TSharedPtr<FBAGraphHandler> InGraphHandler);
	void BuildCommentTree();

	TSharedPtr<FBACommentContainsNode> GetNode(const UEdGraphNode_Comment* Comment) { return ContainsGraph.FindRef(Comment); }
	TOptional<FSlateRect> GetCommentBounds(UEdGraphNode_Comment* CommentNode, TSet<UEdGraphNode_Comment*>& IgnoredComments, UEdGraphNode* NodeAsking, TSet<UEdGraphNode*>& VisitedNodes);
	void DrawBounds();

	void AddNodeIntoComment(UEdGraphNode_Comment* Comment, UEdGraphNode* Node);
	void DeleteNode(UEdGraphNode* Node);

	TSharedPtr<FBACommentContainsGraph> BuildSubsetGraph(const TSet<UEdGraphNode_Comment*>& CommentSubset);

	void LogGraph();

protected:
	void AssignParentsAndChildren(TSharedPtr<FBACommentContainsNode> CurrentNode, TSet<TSharedPtr<FBACommentContainsNode>>& PendingNodes);
	void AssignOwnedNodes(TSharedPtr<FBACommentContainsNode> CurrentNode, TSet<TSharedPtr<FBACommentContainsNode>>& PendingNodes, TSet<UEdGraphNode*>& VisitedNodes);
	void SetHeight();
	int SetHeight(TSharedPtr<FBACommentContainsNode> ContainsNode, FContainsNodeSet& Visited);

	void AssignSubsetParentsAndChildren(
		TSharedPtr<FBACommentContainsNode> CurrentNode, 
		TSharedPtr<FBACommentContainsNode> LastValidParent,
		TArray<TSharedPtr<FBACommentContainsNode>>& PendingNodes,
		TSharedPtr<FBACommentContainsGraph> SubsetGraph,
		FContainsNodeSet& VisitedNodes);
};
