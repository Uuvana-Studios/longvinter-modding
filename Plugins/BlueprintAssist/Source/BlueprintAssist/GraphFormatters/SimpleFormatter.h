// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistCommentHandler.h"
#include "BlueprintAssistGraphHandler.h"
#include "FormatterInterface.h"

class BLUEPRINTASSIST_API FSimpleFormatter
	: public FFormatterInterface
{
public:
	TSharedPtr<FBAGraphHandler> GraphHandler;
	FBAFormatterSettings FormatterSettings;
	FEdGraphFormatterParameters FormatterParameters;
	float TrackSpacing;
	UEdGraphNode* RootNode;
	virtual UEdGraphNode* GetRootNode() override { return RootNode; }
	TSet<UEdGraphNode*> FormattedNodes;
	TMap<UEdGraphNode*, TSharedPtr<FFormatXInfo>> FormatXInfoMap;
	TMap<FPinLink, bool> SameRowMapping;
	TMap<FBAGraphPinHandle, FBAGraphPinHandle> SameRowMappingDirect;

	FNodeRelativeMapping NodeRelativeMapping;

	TSet<TSharedPtr<FFormatXInfo>> NodesToExpand;

	TArray<FPinLink> Path;

	FCommentHandler CommentHandler;

	FSimpleFormatter(TSharedPtr<FBAGraphHandler> InGraphHandler, const FEdGraphFormatterParameters& InFormatterParameters);

	virtual ~FSimpleFormatter() override { }

	void ProcessSameRowMapping(const FPinLink& Link, TSet<UEdGraphNode*>& VisitedNodes);

	virtual void FormatNode(UEdGraphNode* Node) override;

	void FormatX();
	int32 GetChildX(const FPinLink& Link);
	void ExpandPendingNodes();

	void FormatY();

	void FormatY_Recursive(
		UEdGraphNode* CurrentNode,
		UEdGraphPin* CurrentPin,
		UEdGraphPin* ParentPin,
		TSet<UEdGraphNode*>& NodesToCollisionCheck,
		TSet<FPinLink>& VisitedLinks,
		bool bSameRow,
		TSet<UEdGraphNode*>& Children);

	virtual TSet<UEdGraphNode*> GetFormattedNodes() override;

	virtual FCommentHandler* GetCommentHandler() override { return &CommentHandler; }

	virtual FBAFormatterSettings GetFormatterSettings() override;

	virtual const FEdGraphFormatterParameters& GetFormatterParameters() override { return FormatterParameters; }

	virtual void SetNodeY_KeepingSpacingVisited(UEdGraphNode* Node, float NewPosY, TSet<UEdGraphNode*>& VisitedNodes) override;

	TArray<UEdGraphNode*> GetSameRowNodes(UEdGraphNode* Node);

	bool IsSameRow(const FPinLink& Link);

	FSlateRect GetNodeBounds(UEdGraphNode* Node);
	FSlateRect GetNodeArrayBounds(const TArray<UEdGraphNode*>& Nodes);

	void ApplyCommentPaddingX();
	void ApplyCommentPaddingX_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes);

	void ApplyCommentPaddingY();
	void ApplyCommentPaddingY_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes);
};
