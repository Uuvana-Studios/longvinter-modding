// Copyright 2021 fpwong. All Rights Reserved.

#pragma once
#include "BlueprintAssistCommentContainsGraph.h"
#include "EdGraphNode_Comment.h"

struct FPinLink;
struct FFormatterInterface;
class UEdGraphNode_Comment;
class FBAGraphHandler;

struct BLUEPRINTASSIST_API FCommentHandler
	: public TSharedFromThis<FCommentHandler>
{
	TSharedPtr<FBAGraphHandler> GraphHandler;
	TSharedPtr<FFormatterInterface> Formatter;
	TSharedPtr<FBACommentContainsGraph> MasterContainsGraph;
	TSharedPtr<FBACommentContainsGraph> ContainsGraph;

	TSet<UEdGraphNode_Comment*> IgnoredRelatedComments;
	TSet<UEdGraphNode_Comment*> IgnoredComments;

	FCommentHandler() = default;
	FCommentHandler(TSharedPtr<FBAGraphHandler> InGraphHandler, TSharedPtr<FFormatterInterface> InFormatter);

	void Init(TSharedPtr<FBAGraphHandler> InGraphHandler, TSharedPtr<FFormatterInterface> InFormatter);

	bool IsValid() const { return ContainsGraph.IsValid(); }

	void BuildTree();

	TSharedPtr<FBACommentContainsGraph> GetMasterContainsGraph();

	TSharedPtr<FBACommentContainsNode> GetContainsNode(UEdGraphNode* Node)
	{
		if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Node))
		{
			return ContainsGraph->GetNode(Comment);
		}

		return nullptr;
	}

	TSharedPtr<FBACommentContainsNode> GetContainsNode(UEdGraphNode_Comment* Comment) { return ContainsGraph->GetNode(Comment); }

	const TSet<UEdGraphNode_Comment*>& GetComments() const { return ContainsGraph->Comments; }
	TArray<UEdGraphNode_Comment*> GetParentComments(const UEdGraphNode* Node) const;
	TArray<UEdGraphNode*> GetNodesUnderComments(UEdGraphNode_Comment* Comment) const;

	void Reset();

	FSlateRect GetCommentBounds(UEdGraphNode_Comment* CommentNode, UEdGraphNode* NodeAsking = nullptr);

	FSlateRect GetCommentBounds(UEdGraphNode_Comment* CommentNode, TSet<UEdGraphNode*>& IgnoredNodes, UEdGraphNode* NodeAsking = nullptr);

	FSlateRect GetNodeArrayBoundsWithComments(const TArray<UEdGraphNode*>& Nodes, UEdGraphNode* NodeAsking = nullptr, const TSet<UEdGraphNode*>& IgnoredNodes = {});

	bool DoesCommentContainNode(UEdGraphNode_Comment* Comment, UEdGraphNode* Node);

	bool ShouldIgnoreComment(UEdGraphNode_Comment* Comment);

	const TSet<TSharedPtr<FBACommentContainsNode>>& GetRootNodes();

	static bool AreCommentsIntersecting(UEdGraphNode_Comment* CommentA, UEdGraphNode_Comment* CommentB);

	static TSet<UEdGraphNode*> GetMissingNodes(const TArray<UEdGraphNode*>& NodeSet);
	static void AddMissingNodes_Recursive(const FPinLink& CurrentLink, const TArray<UEdGraphNode*>& NodeSet, TSet<UEdGraphNode*>& VisitedNodes, TArray<FPinLink>& VisitedLinks, TArray<UEdGraphNode*> AccumulatedMissingNodes, TSet<UEdGraphNode*>& OutMissingNodes);

	void AddNodeIntoComment(UEdGraphNode_Comment* Comment, UEdGraphNode* Node);
	void DeleteNode(UEdGraphNode* Node);

	FMargin GetCommentPadding(UEdGraphNode_Comment* CommentNode) const;

	void UpdateCommentBounds();
	void DrawBounds(const FLinearColor& Color);
};
