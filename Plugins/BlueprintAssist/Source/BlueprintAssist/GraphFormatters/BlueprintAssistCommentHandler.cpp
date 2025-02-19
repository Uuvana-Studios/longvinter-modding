// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistCommentHandler.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistStats.h"
#include "BlueprintAssistUtils.h"
#include "EdGraphNode_Comment.h"
#include "FormatterInterface.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistGraphOverlay.h"

FCommentHandler::FCommentHandler(TSharedPtr<FBAGraphHandler> InGraphHandler, TSharedPtr<FFormatterInterface> InFormatter)
{
	Init(InGraphHandler, InFormatter);
}

void FCommentHandler::Init(TSharedPtr<FBAGraphHandler> InGraphHandler, TSharedPtr<FFormatterInterface> InFormatter)
{
	if (!InGraphHandler.IsValid() || !InFormatter.IsValid())
	{
		return;
	}

	GraphHandler = InGraphHandler;
	Formatter = InFormatter;
	MasterContainsGraph = InFormatter->GetFormatterParameters().MasterContainsGraph;
}

void FCommentHandler::BuildTree()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FCommentHandler::BuildTree"), STAT_CommentHandler_BuildTree, STATGROUP_BA_EdGraphFormatter);

	if (!MasterContainsGraph)
	{
		return;
	}

	Reset();

	TSet<UEdGraphNode*> FormattedNodes = Formatter->GetFormattedNodes();
	// UE_LOG(LogTemp, Warning, TEXT("Formatted nodes:"));
	// for (UEdGraphNode* FormattedNode : FormattedNodes)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(FormattedNode));
	// }

	TSet<UEdGraphNode_Comment*> Comments;

	{
		DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FCommentHandler::FilterComments"), STAT_CommentHandler_FilterComments, STATGROUP_BA_EdGraphFormatter);
		TArray<UEdGraphNode_Comment*> CommentNodes = MasterContainsGraph->Comments.Array();

		CommentNodes.Sort([&](const UEdGraphNode_Comment& A, const UEdGraphNode_Comment& B)
		{
			return A.GetNodesUnderComment().Num() > B.GetNodesUnderComment().Num();
		});

		for (int i = CommentNodes.Num() - 1; i >= 0; --i)
		{
			UEdGraphNode_Comment* Comment = CommentNodes[i];

			TSharedPtr<FBACommentContainsNode> ContainsNode = MasterContainsGraph->GetNode(Comment);
			const TArray<UEdGraphNode*>& NodesUnderComment = GetMasterContainsGraph()->GetNode(Comment)->AllContainedNodes;

			if (Formatter->ShouldIgnoreComment(ContainsNode))
			{
				CommentNodes.RemoveAt(i);
				IgnoredComments.Add(Comment);
				// UE_LOG(LogTemp, Warning, TEXT("Ignore comment %s"), *FBAUtils::GetNodeName(Comment));

				// check if the comment is related
				const bool bIsRelated = NodesUnderComment.ContainsByPredicate([&FormattedNodes = FormattedNodes](UEdGraphNode* Node)
				{
					return FormattedNodes.Contains(Node);
				});

				if (bIsRelated)
				{
					IgnoredRelatedComments.Add(Comment);
				}
			}
		}

		// remove intersecting comments
		TSet<UEdGraphNode_Comment*> IntersectingComments;
		for (int i = 0; i < CommentNodes.Num(); ++i)
		{
			if (IntersectingComments.Contains(CommentNodes[i]))
			{
				continue;
			}

			TSharedPtr<FBACommentContainsNode> ContainsA = MasterContainsGraph->GetNode(CommentNodes[i]);

			for (int j = i + 1; j < CommentNodes.Num(); ++j)
			{
				if (IntersectingComments.Contains(CommentNodes[j]))
				{
					continue;
				}

				TSharedPtr<FBACommentContainsNode> ContainsB = MasterContainsGraph->GetNode(CommentNodes[j]);

				// if the comment
				if (ContainsA->AllContainedNodesWithComments.Contains(ContainsB->Comment))
				{
					continue;
				}

				// if the comment contain each other continue
				if (ContainsB->AllContainedNodesWithComments.Contains(ContainsA->Comment))
				{
					continue;
				}

				bool bAreIntersecting = ContainsA->AllContainedNodes.ContainsByPredicate([&ContainsB](UEdGraphNode* Node)
				{
					return ContainsB->AllContainedNodes.Contains(Node);
				});

				if (bAreIntersecting)
				{
					IntersectingComments.Add(CommentNodes[j]);
					// UE_LOG(LogTemp, Warning, TEXT("INTERSECTING COMMENTS %s %s"), *FBAUtils::GetNodeName(CommentNodes[i]), *FBAUtils::GetNodeName(CommentNodes[j]));
				}
			}
		}

		IgnoredRelatedComments.Append(IntersectingComments);

		CommentNodes.RemoveAll([&IntersectingComments = IntersectingComments](UEdGraphNode_Comment* Comment)
		{
			return IntersectingComments.Contains(Comment);
		});

		Comments.Append(CommentNodes);
	}

	ContainsGraph = MasterContainsGraph->BuildSubsetGraph(Comments);
	// ContainsGraph->LogGraph();
}

TSharedPtr<FBACommentContainsGraph> FCommentHandler::GetMasterContainsGraph()
{
	return MasterContainsGraph;
}

TArray<UEdGraphNode_Comment*> FCommentHandler::GetParentComments(const UEdGraphNode* Node) const
{
	TArray<UEdGraphNode_Comment*> Parents;
	if (!ContainsGraph)
	{
		return Parents;
	}

	if (const UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Node))
	{
		for (TSharedPtr<FBACommentContainsNode> Parent : ContainsGraph->GetNode(Comment)->Parents)
		{
			Parents.Add(Parent->Comment);
		}
	}
	else if (const auto AllContainingNodes = ContainsGraph->NodeContainingMap.Find(Node))
	{
		for (auto ContainsNode : *AllContainingNodes)
		{
			Parents.Add(ContainsNode->Comment);
		}
	}

	return Parents;
}

TArray<UEdGraphNode*> FCommentHandler::GetNodesUnderComments(UEdGraphNode_Comment* Comment) const
{
	if (ContainsGraph)
	{
		if (TSharedPtr<FBACommentContainsNode> Contains = ContainsGraph->GetNode(Comment))
		{
			// maybe this should return AllContainedNodesWithComments
			return Contains->AllContainedNodes;
		}
	}

	return TArray<UEdGraphNode*>();
}

void FCommentHandler::Reset()
{
}

FSlateRect FCommentHandler::GetCommentBounds(UEdGraphNode_Comment* CommentNode, UEdGraphNode* NodeAsking)
{
	TSet<UEdGraphNode*> IgnoredNodes;
	return GetCommentBounds(CommentNode, IgnoredNodes, NodeAsking);
}

FSlateRect FCommentHandler::GetCommentBounds(UEdGraphNode_Comment* CommentNode, TSet<UEdGraphNode*>& IgnoredNodes, UEdGraphNode* NodeAsking)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FCommentHandler::GetCommentBounds"), STAT_EdGraphFormatter_GetCommentBounds, STATGROUP_BA_EdGraphFormatter);

	if (TOptional<FSlateRect> Bounds = ContainsGraph->GetCommentBounds(CommentNode, IgnoredComments, NodeAsking, IgnoredNodes))
	{
		return Bounds.GetValue(); 
	}

	return FSlateRect();
}

FSlateRect FCommentHandler::GetNodeArrayBoundsWithComments(const TArray<UEdGraphNode*>& Nodes, UEdGraphNode* NodeAsking, const TSet<UEdGraphNode*>& IgnoredNodes)
{
	TOptional<FSlateRect> Bounds;

	TSet<UEdGraphNode_Comment*> AllComments;
	for (UEdGraphNode* Node : Nodes)
	{
		if (!Node)
			continue;

		const FSlateRect NodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Node);
		Bounds = !Bounds.IsSet() ? NodeBounds : Bounds.GetValue().Expand(NodeBounds);

		AllComments.Append(GetParentComments(Node));
	}

	for (UEdGraphNode_Comment* Comment : AllComments)
	{
		if (!Comment)
			continue;

		auto OutIgnoredNodes = IgnoredNodes;
		const FSlateRect CommentBounds = GetCommentBounds(Comment, OutIgnoredNodes, NodeAsking);
		Bounds = !Bounds.IsSet() ? CommentBounds : Bounds.GetValue().Expand(CommentBounds);
	}

	return Bounds.Get(FSlateRect());
}

bool FCommentHandler::DoesCommentContainNode(UEdGraphNode_Comment* Comment, UEdGraphNode* Node)
{
	if (TSharedPtr<FBACommentContainsNode> Contains = ContainsGraph->GetNode(Comment))
	{
		return Contains->AllContainedNodes.Contains(Node);
	}

	return false;
}

bool FCommentHandler::ShouldIgnoreComment(UEdGraphNode_Comment* Comment)
{
	return !GetComments().Contains(Comment);
}

const TSet<TSharedPtr<FBACommentContainsNode>>& FCommentHandler::GetRootNodes()
{
	return ContainsGraph->RootNodes;
}

bool FCommentHandler::AreCommentsIntersecting(UEdGraphNode_Comment* CommentA, UEdGraphNode_Comment* CommentB)
{
	if (!CommentA || !CommentB)
	{
		return false;
	}

	struct FLocal
	{
		static bool IsContainedInOther(UEdGraphNode_Comment* Comment, UEdGraphNode* Node)
		{
			return FBAUtils::GetNodesUnderComment(Comment).Contains(Node);
		}
	};

	if (FLocal::IsContainedInOther(CommentA, CommentB) || FLocal::IsContainedInOther(CommentB, CommentA))
	{
		return false;
	}

	const TArray<UEdGraphNode*> NodesA = FBAUtils::GetNodesUnderComment(CommentA);
	const TArray<UEdGraphNode*> NodesB = FBAUtils::GetNodesUnderComment(CommentB);

	const TArray<UEdGraphNode*> Intersection = NodesA.FilterByPredicate([&NodesB](UEdGraphNode* Node) { return NodesB.Contains(Node); });
	return Intersection.Num() > 0;
}

TSet<UEdGraphNode*> FCommentHandler::GetMissingNodes(const TArray<UEdGraphNode*>& NodeSet)
{
	TSet<UEdGraphNode*> OutMissingNodes;
	TArray<UEdGraphNode*> PendingNodes = NodeSet;

	while (PendingNodes.Num() > 0)
	{
		UEdGraphNode* CurrentNode = PendingNodes.Pop();

		// do not process knot nodes
		if (FBAUtils::IsKnotNode(CurrentNode))
		{
			continue;
		}

		TSet<UEdGraphNode*> VisitedNodes;
		TArray<FPinLink> VisitedLinks;
		TArray<UEdGraphNode*> LocalMissingNodes;
		FPinLink PinLink(nullptr, nullptr, CurrentNode);
		// UE_LOG(LogTemp, Warning, TEXT("Add missing nodes for %s"), *FBAUtils::GetNodeName(CurrentNode));
		AddMissingNodes_Recursive(PinLink, NodeSet, VisitedNodes, VisitedLinks, LocalMissingNodes, OutMissingNodes);

		for (UEdGraphNode* Node : VisitedNodes)
		{
			PendingNodes.Remove(Node);
		}
	}

	// for (UEdGraphNode* MissingNode : OutMissingNodes)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Missing node %s"), *FBAUtils::GetNodeName(MissingNode));
	// }

	return OutMissingNodes;
}

void FCommentHandler::AddMissingNodes_Recursive(const FPinLink& CurrentLink, const TArray<UEdGraphNode*>& NodeSet, TSet<UEdGraphNode*>& VisitedNodes, TArray<FPinLink>& VisitedLinks, TArray<UEdGraphNode*> AccumulatedMissingNodes, TSet<UEdGraphNode*>& OutMissingNodes)
{
	UEdGraphNode* CurrentNode = CurrentLink.GetNode();
	VisitedNodes.Add(CurrentLink.GetNode());
	TArray<FPinLink> Links = FBAUtils::GetPinLinks(CurrentNode);

	// UE_LOG(LogTemp, Warning, TEXT("Iterating %s"), *CurrentLink.ToString());

	const bool bInsideNodeSet = NodeSet.Contains(CurrentNode);
	if (NodeSet.Contains(CurrentNode))
	{
		OutMissingNodes.Append(AccumulatedMissingNodes);

		// for (UEdGraphNode* AccumulatedMissingNode : AccumulatedMissingNodes)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("\tAdding accumulated nodes %s"), *FBAUtils::GetNodeName(AccumulatedMissingNode));
		// }
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("\tAccumulating %s"), *FBAUtils::GetNodeName(CurrentNode));
		AccumulatedMissingNodes.Add(CurrentNode);
	}

	for (const FPinLink& Link : Links)
	{
		if (!bInsideNodeSet && Link.GetDirection() != CurrentLink.GetDirection())
		{
			continue;
		}

		if (VisitedLinks.Contains(Link))
		{
			continue;
		}

		VisitedLinks.Add(Link);
		VisitedLinks.Add(Link.MakeOppositeLink());

		// skip knot nodes
		UEdGraphNode* ToNode = Link.To->GetOwningNode();
		if (FBAUtils::IsKnotNode(ToNode))
		{
			continue;
		}

		if (bInsideNodeSet)
		{
			AddMissingNodes_Recursive(Link, NodeSet, VisitedNodes, VisitedLinks, TArray<UEdGraphNode*>(), OutMissingNodes);
		}
		else
		{
			AddMissingNodes_Recursive(Link, NodeSet, VisitedNodes, VisitedLinks, AccumulatedMissingNodes, OutMissingNodes);
		}
	}
}

void FCommentHandler::AddNodeIntoComment(UEdGraphNode_Comment* Comment, UEdGraphNode* Node)
{
	Comment->AddNodeUnderComment(Node);

	if (TSharedPtr<FBACommentContainsGraph> Master = GetMasterContainsGraph())
	{
		Master->AddNodeIntoComment(Comment, Node);
	}

	if (ContainsGraph)
	{
		ContainsGraph->AddNodeIntoComment(Comment, Node);
	}
}

void FCommentHandler::DeleteNode(UEdGraphNode* Node)
{
	if (TSharedPtr<FBACommentContainsGraph> Master = GetMasterContainsGraph())
	{
		Master->DeleteNode(Node);
	}

	if (ContainsGraph)
	{
		ContainsGraph->DeleteNode(Node);
	}
}

FMargin FCommentHandler::GetCommentPadding(UEdGraphNode_Comment* CommentNode) const
{
	const FVector2D Padding = GetDefault<UBASettings>()->CommentNodePadding;

	const float TitlebarHeight = FBAUtils::GetCachedNodeBounds(GraphHandler, CommentNode, false).GetSize().Y;

	return FMargin(
		Padding.X,
		Padding.Y + TitlebarHeight,
		Padding.X,
		Padding.Y);
}

void FCommentHandler::UpdateCommentBounds()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FCommentHandler::UpdateCommentBounds"), STAT_CommentHandler_UpdateCommentBounds, STATGROUP_BA_EdGraphFormatter);
	TArray<UEdGraphNode_Comment*> RemainingComments = GetComments().Array();

	// TODO sort this by comment depth!
	TSet<UEdGraphNode*> Ignored;
	for (UEdGraphNode_Comment* IgnoredRelatedComment : RemainingComments)
	{
		Ignored.Reset();
		if (TOptional<FSlateRect> Bounds = ContainsGraph->GetCommentBounds(IgnoredRelatedComment, IgnoredComments, nullptr, Ignored))
		{
			IgnoredRelatedComment->Modify();
			IgnoredRelatedComment->SetBounds(*Bounds);
		}
	}
}

void FCommentHandler::DrawBounds(const FLinearColor& Color)
{
	TSet<UEdGraphNode*> Ignored;
	for (UEdGraphNode_Comment* IgnoredRelatedComment : GetComments())
	{
		Ignored.Reset();
		if (TOptional<FSlateRect> Bounds = ContainsGraph->GetCommentBounds(IgnoredRelatedComment, IgnoredComments, nullptr, Ignored))
		{
			FBAGraphOverlayBounds Params;
			Params.Bounds = *Bounds;
			Params.Color = Color;
			GraphHandler->GetGraphOverlay()->DrawBounds(Params);
		}
	}
}

