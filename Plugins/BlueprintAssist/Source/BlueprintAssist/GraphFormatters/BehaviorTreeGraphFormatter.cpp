// Copyright 2021 fpwong. All Rights Reserved.

#include "BehaviorTreeGraphFormatter.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistUtils.h"
#include "Containers/Map.h"

FBehaviorTreeGraphFormatter::FBehaviorTreeGraphFormatter(
	TSharedPtr<FBAGraphHandler> InGraphHandler,
	const FEdGraphFormatterParameters& InFormatterParameters)
	: GraphHandler(InGraphHandler)
	, FormatterParameters(InFormatterParameters)
{
	FormatterSettings = UBASettings::GetFormatterSettings(InGraphHandler->GetFocusedEdGraph());
}

void FBehaviorTreeGraphFormatter::FormatNode(UEdGraphNode* InNode)
{
	RootNode = InNode;
	RootNode->Modify();

	while (true)
	{
		TArray<UEdGraphNode*> LinkedInputNodes = FBAUtils::GetLinkedNodes(RootNode, EGPD_Input);
		if (LinkedInputNodes.Num() == 0)
		{
			break;
		}

		RootNode = LinkedInputNodes[0];
	}

	if (RootNode == nullptr)
	{
		return;
	}

	const float OldNodeX = RootNode->NodePosX;
	FormatNodes(RootNode, FormattedNodes);
	const float DeltaX = RootNode->NodePosX - OldNodeX;

	// make it look like the root node doesn't move
	for (UEdGraphNode* Node : FormattedNodes)
	{
		Node->NodePosX -= DeltaX;
	}
}

FBAFormatterSettings FBehaviorTreeGraphFormatter::GetFormatterSettings()
{
	return UBASettings::GetFormatterSettings(GraphHandler->GetFocusedEdGraph());
}

void FBehaviorTreeGraphFormatter::FormatNodes(UEdGraphNode* InNode, TSet<UEdGraphNode*>& VisitedNodes) const
{
	// TODO: Improve this algorithm as it can have overlapping nodes. With an x-padding of 100 it requires a very large BT though
	TArray<UEdGraphPin*> LinkedOutPins = FBAUtils::GetLinkedPins(InNode, EGPD_Output);

	FSlateRect InNodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, InNode);

	float PreviousNodeX = InNode->NodePosX;
	if (VisitedNodes.Num() > 0)
	{
		const FSlateRect VisitedNodeBounds = FBAUtils::GetCachedNodeArrayBounds(
			GraphHandler,
			VisitedNodes.Array());

		PreviousNodeX = FMath::Max(PreviousNodeX, VisitedNodeBounds.Right);
	}

	const auto& LeftMostSorter = [](UEdGraphPin& A, UEdGraphPin& B)
	{
		return A.GetOwningNode()->NodePosX < B.GetOwningNode()->NodePosX;
	};

	TArray<UEdGraphNode*> ChildNodes;
	for (UEdGraphPin* Pin : LinkedOutPins)
	{
		TArray<UEdGraphPin*> LinkedToPins = Pin->LinkedTo;

		// must sort the pins by left most, since order in the behavior tree matters
		LinkedToPins.Sort(LeftMostSorter);

		for (UEdGraphPin* LinkedPin : LinkedToPins)
		{
			UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();

			if (VisitedNodes.Contains(LinkedNode))
			{
				continue;
			}

			LinkedNode->Modify(false);

			// vaguely position the child node
			LinkedNode->NodePosX = PreviousNodeX + FormatterSettings.Padding.X;
			LinkedNode->NodePosY = InNodeBounds.Bottom + FormatterSettings.Padding.Y;

			FormatNodes(LinkedNode, VisitedNodes);

			ChildNodes.Add(LinkedNode);

			// center the root node to the children
			const FSlateRect ChildrenBounds = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, ChildNodes);
			InNode->NodePosX = ChildrenBounds.GetCenter().X - InNodeBounds.GetSize().X / 2;

			InNodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, InNode);

			PreviousNodeX = FBAUtils::GetCachedNodeBounds(GraphHandler, LinkedNode).Right;
		}
	}

	VisitedNodes.Emplace(InNode);
}
