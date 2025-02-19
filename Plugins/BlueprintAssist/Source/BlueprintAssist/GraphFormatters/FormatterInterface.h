// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistSettings.h"
#include "GraphFormatterTypes.h"

struct FCommentHandler;
struct FBACommentContainsNode;
class UEdGraphNode;
class UEdGraphNode_Comment;

struct BLUEPRINTASSIST_API FFormatterInterface
	: public TSharedFromThis<FFormatterInterface>
{
	virtual ~FFormatterInterface() = default;
	virtual void FormatNode(UEdGraphNode* Node) = 0;
	virtual TSet<UEdGraphNode*> GetFormattedNodes() = 0;
	virtual UEdGraphNode* GetRootNode() = 0;
	virtual FBAFormatterSettings GetFormatterSettings() { return FBAFormatterSettings(); }
	virtual const FEdGraphFormatterParameters& GetFormatterParameters() = 0;
	virtual FCommentHandler* GetCommentHandler() { return nullptr; }

	virtual FSlateRect GetClusterBounds(UEdGraphNode* Node) { return FSlateRect(); }

	virtual TSharedPtr<FFormatterInterface> GetChildFormatter(UEdGraphNode* Node) { return nullptr; }
	virtual TArray<TSharedPtr<FFormatterInterface>> GetChildFormatters() { return TArray<TSharedPtr<FFormatterInterface>>(); }

	virtual void SetNodePos(UEdGraphNode* Node, const int X, const int Y)
	{
		Node->NodePosX = X;
		Node->NodePosY = Y;
	};

	virtual void SetNodeY_KeepingSpacing(UEdGraphNode* Node, float NewPosY)
	{
		TSet<UEdGraphNode*> VisitedNodes;
		SetNodeY_KeepingSpacingVisited(Node, NewPosY, VisitedNodes);
	}

	virtual void SetNodeY_KeepingSpacingVisited(UEdGraphNode* Node, float NewPosY, TSet<UEdGraphNode*>& VisitedNodes) {}

	virtual TSet<UEdGraphNode*> GetRowAndChildren(UEdGraphNode* Node) { return TSet<UEdGraphNode*>(); }

	virtual UEdGraphNode* GetClusterRootNode(UEdGraphNode* ChildNode) { return nullptr; }

	virtual bool ShouldIgnoreComment(TSharedPtr<FBACommentContainsNode> ContainsNode) { return false; }
};
