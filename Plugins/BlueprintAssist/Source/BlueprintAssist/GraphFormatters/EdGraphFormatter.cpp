// Copyright 2021 fpwong. All Rights Reserved.

#include "EdGraphFormatter.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistSettings.h"
#include "BlueprintAssistStats.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintAssistCommentContainsGraph.h"
#include "EdGraphNode_Comment.h"
#include "EdGraphParameterFormatter.h"
#include "GraphFormatterTypes.h"
#include "SGraphNodeComment.h"
#include "SGraphPanel.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistGraphOverlay.h"
#include "EdGraph/EdGraphNode.h"
#include "Editor/BlueprintGraph/Classes/K2Node_Knot.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KnotTrack/KnotTrackCreator.h"
#include "Stats/StatsMisc.h"

FNodeChangeInfo::FNodeChangeInfo(UEdGraphNode* InNode, UEdGraphNode* InNodeToKeepStill)
	: Node(InNode)
{
	bIsNodeToKeepStill = Node == InNodeToKeepStill;
	UpdateValues(InNodeToKeepStill);
}

void FNodeChangeInfo::UpdateValues(UEdGraphNode* NodeToKeepStill)
{
	NodeX = Node->NodePosX;
	NodeY = Node->NodePosY;

	NodeOffsetX = Node->NodePosX - NodeToKeepStill->NodePosX;
	NodeOffsetY = Node->NodePosY - NodeToKeepStill->NodePosY;

	Links.Empty();
	for (UEdGraphPin* Pin : Node->Pins)
	{
		for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
		{
			Links.Add(FPinLink(Pin, LinkedPin));
		}
	}
}

bool FNodeChangeInfo::HasChanged(UEdGraphNode* NodeToKeepStill)
{
	// check pin links
	TSet<FPinLink> NewLinks;
	for (UEdGraphPin* Pin : Node->Pins)
	{
		for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
		{
			NewLinks.Add(FPinLink(Pin, LinkedPin));
		}
	}

	if (NewLinks.Num() != Links.Num())
	{
		//UE_LOG(LogBlueprintAssist, Warning, TEXT("Num links changed"));
		return true;
	}

	for (const FPinLink& Link : Links)
	{
		if (!NewLinks.Contains(Link))
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("New links does not contain %s"), *Link.ToString());
			return true;
		}
	}

	return false;
}

FString ChildBranch::ToString() const
{
	return FString::Printf(TEXT("%s | %s"), *FBAUtils::GetPinName(Pin), *FBAUtils::GetPinName(ParentPin));
}

FEdGraphFormatter::FEdGraphFormatter(
	TSharedPtr<FBAGraphHandler> InGraphHandler,
	FEdGraphFormatterParameters InFormatterParameters)
	: GraphHandler(InGraphHandler)
	, RootNode(nullptr)
	, FormatterParameters(InFormatterParameters)
	, KnotTrackCreator()
{
	const UBASettings* BASettings = GetDefault<UBASettings>();

	NodePadding = BASettings->BlueprintFormatterSettings.Padding;
	PinPadding = BASettings->BlueprintParameterPadding;
	TrackSpacing = BASettings->BlueprintKnotTrackSpacing;
	VerticalPinSpacing = BASettings->VerticalPinSpacing;
	bCenterBranches = BASettings->bCenterBranches;
	NumRequiredBranches = BASettings->NumRequiredBranches;

	LastFormattedX = 0;
	LastFormattedY = 0;
}

void FEdGraphFormatter::FormatNode(UEdGraphNode* InitialNode)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::FormatNode"), STAT_EdGraphFormatter_FormatNode, STATGROUP_BA_EdGraphFormatter);

	if (!IsInitialNodeValid(InitialNode))
	{
		return;
	}

	KnotTrackCreator.Init(SharedThis(this), GraphHandler);

	RootNode = InitialNode;

	TArray<UEdGraphNode*> NewNodeTree = GetNodeTree(InitialNode);

	NodeTree = NewNodeTree;

	const auto& SelectedNodes = GraphHandler->GetSelectedNodes();
	const bool bAreAllNodesSelected = !NewNodeTree.ContainsByPredicate([&SelectedNodes](UEdGraphNode* Node)
	{
		return !SelectedNodes.Contains(Node);
	});

	GraphHandler->GetFocusedEdGraph()->Modify();

	KnotTrackCreator.Reset();
	CommentHandler.Reset();
	NodeChangeInfos.Reset();
	NodePool.Reset();
	MainParameterFormatter.Reset();
	ParameterFormatterMap.Reset();
	FormatXInfoMap.Reset();
	Path.Reset();
	SameRowMapping.Reset();
	SameRowMappingDirect.Reset();
	NodesToExpand.Reset();
	ParameterParentMap.Reset();

	CommentHandler.Init(GraphHandler, AsShared());

	// check if we can do simple relative formatting
	if (GetDefault<UBASettings>()->bEnableFasterFormatting && !IsFormattingRequired(NewNodeTree))
	{
		SimpleRelativeFormatting();
		return;
	}

	if (FBAUtils::GetLinkedPins(RootNode).Num() == 0)
	{
		NodePool = { RootNode };
		CommentHandler.BuildTree();
		return;
	}

	RemoveKnotNodes();

	NodeToKeepStill = FormatterParameters.NodeToKeepStill ? FormatterParameters.NodeToKeepStill : RootNode;

	if (FBAUtils::IsEventNode(RootNode) || FBAUtils::IsExtraRootNode(RootNode))
	{
		NodeToKeepStill = RootNode;
	}
	// UE_LOG(LogTemp, Warning, TEXT("Node to keep still %s | Root %s"), *FBAUtils::GetNodeName(NodeToKeepStill), *FBAUtils::GetNodeName(RootNode));

	if (FBAUtils::IsNodePure(RootNode))
	{
		MainParameterFormatter = MakeShared<FEdGraphParameterFormatter>(GraphHandler, RootNode, SharedThis(this), NodeToKeepStill);
		MainParameterFormatter->FormatNode(RootNode);
		CommentHandler.BuildTree();
		return;
	}

	// Align the root node to the 8x8 grid
	NodeToKeepStill->NodePosX = FBAUtils::AlignTo8x8Grid(NodeToKeepStill->NodePosX); 
	NodeToKeepStill->NodePosY = FBAUtils::AlignTo8x8Grid(NodeToKeepStill->NodePosY); 

	const FVector2D SavedLocation = FVector2D(NodeToKeepStill->NodePosX, NodeToKeepStill->NodePosY);

	// initialize the node pool from the root node
	InitNodePool();

	// if (GetDefault<UBASettings>()->bApplyCommentPadding)
	// {
	// 	CommentHandler.Init(GraphHandler, SharedThis(this));
	// }

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Selected Root Node as %s | NodeToKeepStill as %s"), *FBAUtils::GetNodeName(RootNode), *FBAUtils::GetNodeName(NodeToKeepStill));
	//for (UEdGraphNode* Node : FormatterParameters.NodesToFormat)
	//{
	//	UE_LOG(LogBlueprintAssist, Warning, TEXT("\tSelective Formatting %s"), *FBAUtils::GetNodeName(Node));
	//}

	// for (UEdGraphNode* Node : NodePool)
	// {
	// 	// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tNodePool %s"), *FBAUtils::GetNodeName(Node));
	// }

	FormatX(false);

	//UE_LOG(LogBlueprintAssist, Warning, TEXT("Path: "));
	//for (FPinLink& PinLink : Path)
	//{
	//	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *PinLink.ToString());
	//}

	//UE_LOG(LogBlueprintAssist, Warning, TEXT("NodeInfos: "));
	//for (UEdGraphNode* Node : NodePool)
	//{
	//	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	//	if (!FormatXInfoMap.Contains(Node))
	//	{
	//		UE_LOG(LogBlueprintAssist, Error, TEXT("ERROR FormatXInfo does not contain %s"), *FBAUtils::GetNodeName(Node));
	//	}
	//	else
	//	{
	//		for (TSharedPtr<FFormatXInfo> Info : FormatXInfoMap[Node]->Children)
	//		{
	//			UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t%s"), *FBAUtils::GetNodeName(Info->GetNode()));
	//		}
	//	}
	//}

	/** Format the input nodes before we format the X position so we can get the column bounds */
	FormatParameterNodes();

	CommentHandler.BuildTree();

	if (GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("X1"))
	{
		return;
	}

	Path.Empty();
	FormatXInfoMap.Empty();
	FormatX(true);

	if (GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("X2"))
	{
		return;
	}

	GetPinsOfSameHeight();

	// UE_LOG(LogTemp, Warning, TEXT("Same row mapping"));
	// for (auto Kvp : SameRowMapping)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Kvp.Key.ToString());
	// }

	// UE_LOG(LogTemp, Warning, TEXT("Format x children"));
	// for (auto Kvp : FormatXInfoMap)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Kvp.Key));
	// 	for (auto FormatXInfo : Kvp.Value->Children)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("\t\t%s"), *FBAUtils::GetNodeName(FormatXInfo->GetNode()));
	// 	}
	// }

	if (GetDefault<UBASettings>()->bExpandNodesAheadOfParameters)
	{
		ExpandNodesAheadOfParameters();
	}

	if (GetDefault<UBASettings>()->bApplyCommentPadding && !GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("PaddingX"))
	{
		ApplyCommentPaddingX();
	}

	/** Format Y (Rows) */
	FormatY();

	// TODO: Do we need to do this before calling ApplyCommentPaddingX?
	if (GetMutableDefault<UBASettings>()->bExpandNodesByHeight)
	{
		ExpandByHeight();
	}

	if (GetDefault<UBASettings>()->bApplyCommentPadding && !GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("PaddingY"))
	{
		ApplyCommentPaddingY();
	}

	// TODO: Finish logic for wrapping nodes
	// WrapNodes();

	/** Format knot nodes */
	if (GetDefault<UBASettings>()->bCreateKnotNodes)
	{
		KnotTrackCreator.FormatKnotNodes();

		if (GetDefault<UBASettings>()->bApplyCommentPadding && !GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("AfterKnots"))
		{
			ApplyCommentPaddingAfterKnots();
		}

		KnotTrackCreator.AddNomadKnotsIntoComments();
	}

	/** Formatting may move nodes, move all nodes back using the root as a baseline */
	ResetRelativeToNodeToKeepStill(SavedLocation);

	if (GetDefault<UBASettings>()->bSnapToGrid)
	{
		/** Snap all nodes to the grid (only on the x-axis) */
		TSet<UEdGraphNode*> FormattedNodes = GetFormattedGraphNodes();
		for (UEdGraphNode* Node : FormattedNodes)
		{
			Node->NodePosX = FBAUtils::SnapToGrid(Node->NodePosX);
		}
	}

	SaveFormattingEndInfo();

	// Check if formatting is required checks the difference between the node trees, so we must set it here
	NodeTree = GetNodeTree(InitialNode);

	//for (UEdGraphNode* Nodes : GetFormattedGraphNodes())
	//{
	//	UE_LOG(LogBlueprintAssist, Warning, TEXT("Formatted node %s"), *FBAUtils::GetNodeName(Nodes));
	//}
	//

	if (bAreAllNodesSelected)
	{
		auto& SelectionManager = GraphHandler->GetGraphPanel()->SelectionManager;
		for (auto Node : KnotTrackCreator.GetCreatedKnotNodes())
		{
			SelectionManager.SetNodeSelection(Node, true);
		}
	}

	// NodeRelativeMapping.PrintInfo();

	// for (TTuple<FPinLink, bool> RowMapping : SameRowMapping)
	// {
	// 	GraphHandler->GetGraphOverlay()->DrawLine(
	// 		FBAUtils::GetPinPos(GraphHandler, RowMapping.Key.From),
	// 		FBAUtils::GetPinPos(GraphHandler, RowMapping.Key.To),
	// 		FLinearColor::MakeRandomColor());
	// }

	// for (auto Kvp : ParameterFormatterMap)
	// {
	// 	for (TTuple<FPinLink, bool> RowMapping : Kvp.Value->SameRowMapping)
	// 	{
	// 		GraphHandler->GetGraphOverlay()->DrawLine(
	// 			FBAUtils::GetPinPos(GraphHandler, RowMapping.Key.From),
	// 			FBAUtils::GetPinPos(GraphHandler, RowMapping.Key.To),
	// 			FLinearColor::MakeRandomColor());
	// 	}
	// }
}

void FEdGraphFormatter::InitNodePool()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::InitNodePool"), STAT_EdGraphFormatter_InitNodePool, STATGROUP_BA_EdGraphFormatter);
	NodePool.Empty();
	TArray<UEdGraphNode*> InputNodeStack;
	TArray<UEdGraphNode*> OutputNodeStack;
	OutputNodeStack.Push(RootNode);
	RootNode->Modify();

	while (InputNodeStack.Num() > 0 || OutputNodeStack.Num() > 0)
	{
		UEdGraphNode* CurrentNode
			= OutputNodeStack.Num() > 0
			? OutputNodeStack.Pop()
			: InputNodeStack.Pop();

		if (!GraphHandler->FilterSelectiveFormatting(CurrentNode, FormatterParameters.NodesToFormat))
		{
			continue;
		}

		if (NodePool.Contains(CurrentNode) || FBAUtils::IsNodePure(CurrentNode))
		{
			continue;
		}

		NodePool.Add(CurrentNode);

		TArray<EEdGraphPinDirection> Directions = { EGPD_Input, EGPD_Output };

		for (EEdGraphPinDirection& Dir : Directions)
		{
			TArray<UEdGraphPin*> ExecPins = FBAUtils::GetLinkedPins(CurrentNode, Dir).FilterByPredicate(FBAUtils::IsExecOrDelegatePin);

			for (int32 MyPinIndex = ExecPins.Num() - 1; MyPinIndex >= 0; MyPinIndex--)
			{
				UEdGraphPin* Pin = ExecPins[MyPinIndex];

				for (int32 i = Pin->LinkedTo.Num() - 1; i >= 0; i--)
				{
					UEdGraphPin* LinkedPin = Pin->LinkedTo[i];
					UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();

					if (NodePool.Contains(LinkedNode) ||
						FBAUtils::IsNodePure(LinkedNode) ||
						!GraphHandler->FilterSelectiveFormatting(LinkedNode, FormatterParameters.NodesToFormat))
					{
						continue;
					}

					LinkedNode->Modify();

					FBAUtils::StraightenPin(GraphHandler, Pin, LinkedPin);

					if (Dir == EGPD_Output)
					{
						OutputNodeStack.Push(LinkedNode);
					}
					else
					{
						InputNodeStack.Push(LinkedNode);
					}
				}
			}
		}
	}
}

void FEdGraphFormatter::ExpandPendingNodes(bool bUseParameter)
{
	for (TSharedPtr<FFormatXInfo> Info : NodesToExpand)
	{
		if (!Info->Parent.IsValid())
		{
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("Expand X Invalid %s"), *FBAUtils::GetNodeName(Info->GetNode()));
			return;
		}

		UEdGraphNode* Node = Info->GetNode();
		UEdGraphNode* Parent = Info->Parent->GetNode();
		TArray<UEdGraphNode*> InputChildren = Info->GetChildren(EGPD_Input);

		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Expand X %s | %s"), *FBAUtils::GetNodeName(Info->GetNode()), *FBAUtils::GetNodeName(Parent));

		if (InputChildren.Num() > 0)
		{
			const FSlateRect InputBounds = GetNodeArrayBounds(InputChildren, bUseParameter);
			const FSlateRect ParentBounds = GetNodeBounds(Parent, bUseParameter);

			if (ParentBounds.Right > InputBounds.Left)
			{
				const float Delta = ParentBounds.Right - InputBounds.Left + NodePadding.X;

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Expanding node %s by %f"), *FBAUtils::GetNodeName(Node), Delta);

				Node->NodePosX += Delta;

				if (bUseParameter)
				{
					RefreshParameters(Node);
				}

				TArray<UEdGraphNode*> AllChildren = Info->GetChildren();
				for (UEdGraphNode* Child : AllChildren)
				{
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tChild %s"), *FBAUtils::GetNodeName(Child));

					Child->NodePosX += Delta;

					if (bUseParameter)
					{
						RefreshParameters(Child);
					}
				}
			}
		}
	}
}

void FEdGraphFormatter::SimpleRelativeFormatting()
{
	CommentHandler.BuildTree();

	for (UEdGraphNode* Node : GetFormattedNodes())
	{
		// check(NodeChangeInfos.Contains(Node))
		if (NodeChangeInfos.Contains(Node))
		{
			Node->NodePosX = NodeToKeepStill->NodePosX + NodeChangeInfos[Node].NodeOffsetX;
			Node->NodePosY = NodeToKeepStill->NodePosY + NodeChangeInfos[Node].NodeOffsetY;
		}
		else
		{
			UE_LOG(LogBlueprintAssist, Error, TEXT("No ChangeInfo for %s"), *FBAUtils::GetNodeName(Node));
		}
	}

	SaveFormattingEndInfo();

}

void FEdGraphFormatter::FormatX(const bool bUseParameter)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::FormatX"), STAT_EdGraphFormatter_FormatX, STATGROUP_BA_EdGraphFormatter);
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("----- Format X -----"));

	TSet<UEdGraphNode*> VisitedNodes;
	TSet<UEdGraphNode*> PendingNodes;
	PendingNodes.Add(RootNode);
	TSet<FPinLink> VisitedLinks;
	const FPinLink RootNodeLink(nullptr, nullptr, RootNode);
	TSharedPtr<FFormatXInfo> RootInfo = MakeShared<FFormatXInfo>(RootNodeLink, nullptr);

	TArray<TSharedPtr<FFormatXInfo>> OutputStack;
	TArray<TSharedPtr<FFormatXInfo>> InputStack;
	OutputStack.Push(RootInfo);
	FormatXInfoMap.Add(RootNode, RootInfo);

	EEdGraphPinDirection LastDirection = EGPD_Output;

	NodesToExpand.Reset();
	Path.Reset();

	while (OutputStack.Num() > 0 || InputStack.Num() > 0)
	{
		// try to get the current info from the pending input
		TSharedPtr<FFormatXInfo> CurrentInfo = nullptr;

		TArray<TSharedPtr<FFormatXInfo>>& FirstStack = LastDirection == EGPD_Output ? OutputStack : InputStack;
		TArray<TSharedPtr<FFormatXInfo>>& SecondStack = LastDirection == EGPD_Output ? InputStack : OutputStack;

		if (FirstStack.Num() > 0)
		{
			CurrentInfo = FirstStack.Pop();
		}
		else
		{
			CurrentInfo = SecondStack.Pop();
		}

		LastDirection = CurrentInfo->Link.GetDirection();

		UEdGraphNode* CurrentNode = CurrentInfo->GetNode();
		VisitedNodes.Add(CurrentNode);

		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Processing %s | %s"), *FBAUtils::GetNodeName(CurrentNode), *CurrentInfo->Link.ToString());
		const int32 NewX = GetChildX(CurrentInfo->Link, bUseParameter);

		if (!FormatXInfoMap.Contains(CurrentNode))
		{
			if (CurrentNode != RootNode)
			{
				CurrentInfo->SetParent(CurrentInfo->Parent);
				CurrentNode->NodePosX = NewX;

				if (bUseParameter)
				{
					RefreshParameters(CurrentNode);
				}

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tInitial Set node pos x %d %s"), NewX, *FBAUtils::GetNodeName(CurrentNode));

				Path.Add(CurrentInfo->Link);
			}
			FormatXInfoMap.Add(CurrentNode, CurrentInfo);
		}
		else
		{
			TSharedPtr<FFormatXInfo> OldInfo = FormatXInfoMap[CurrentNode];

			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tInfo map contains %s | %s (%s) | Parent %s (%s) | %d"),
			//        *FBAUtils::GetNodeName(CurrentInfo->Link.To->GetOwningNode()),
			//        *FBAUtils::GetNodeName(CurrentInfo->GetNode()),
			//        *FBAUtils::GetPinName(CurrentInfo->Link.To),
			//        *FBAUtils::GetNodeName(CurrentInfo->Link.From->GetOwningNode()),
			//        *FBAUtils::GetPinName(CurrentInfo->Link.From),
			//        NewX);

			const bool bHasNoParent = CurrentInfo->Link.From == nullptr;

			bool bHasCycle = false;
			if (!bHasNoParent) // if we have a parent, check if there is a cycle
			{
				// bHasCycle = OldInfo->GetChildren(EGPD_Output).Contains(CurrentInfo->Parent->GetNode());
				bHasCycle = OldInfo->GetChildren().Contains(CurrentInfo->Parent->GetNode());

				// if (bHasCycle)
				// {
				// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tHas cycle! Skipping"));
				// 	for (UEdGraphNode* Child : OldInfo->GetChildren(EGPD_Output))
				// 	{
				// 		UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tChild: %s"), *FBAUtils::GetNodeName(Child));
				// 	}
				// }

				// for (UEdGraphNode* Child : OldInfo->GetChildren(EGPD_Output))
				// // for (UEdGraphNode* Child : OldInfo->GetChildren())
				// {
				// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tParent %s | Child: %s"), *FBAUtils::GetNodeName(CurrentInfo->Parent->GetNode()), *FBAUtils::GetNodeName(Child));
				// }
			}

			if (bHasNoParent || !bHasCycle)
			{
				if (OldInfo->Parent.IsValid())
				{
					bool bTakeNewParent = bHasNoParent;

					if (!bTakeNewParent)
					{
						const int32 OldX = CurrentInfo->GetNode()->NodePosX;

						const bool bPositionIsBetter
							= CurrentInfo->Link.From->Direction == EGPD_Output
							? NewX > OldX
							: NewX < OldX;

						// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t Comparing parents Old: %s (%d) New: %s (%d)"),
						//        *FBAUtils::GetNodeName(OldInfo->Link.From->GetOwningNode()), OldX,
						//        *FBAUtils::GetNodeName(CurrentInfo->Link.From->GetOwningNode()), NewX);

						const bool bSameDirection = OldInfo->Link.To->Direction == CurrentInfo->Link.To->Direction;
						// if (!bSameDirection) UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tNot same direction"));
						//
						// if (!bPositionIsBetter) UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tPosition is worse?"));

						bTakeNewParent = bPositionIsBetter && bSameDirection;
					}

					// take the new parent by updating the old info
					if (bTakeNewParent)
					{
						// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tTOOK PARENT"));

						OldInfo->Link = CurrentInfo->Link;
						OldInfo->SetParent(CurrentInfo->Parent);

						CurrentInfo = OldInfo;

						CurrentNode->NodePosX = NewX;
						if (bUseParameter)
						{
							RefreshParameters(CurrentNode);
						}

						for (TSharedPtr<FFormatXInfo> ChildInfo : CurrentInfo->Children)
						{
							if (ChildInfo->Link.GetDirection() == EGPD_Output)
							{
								OutputStack.Push(ChildInfo);
							}
							else
							{
								InputStack.Push(ChildInfo);
							}
						}

						Path.Add(CurrentInfo->Link);
					}
				}
			}
		}

		TArray<UEdGraphPin*> LinkedPins = FBAUtils::GetLinkedPins(CurrentInfo->GetNode()).FilterByPredicate(FBAUtils::IsExecOrDelegatePin);

		for (int i = LinkedPins.Num() - 1; i >= 0; --i)
		{
			UEdGraphPin* ParentPin = LinkedPins[i];

			for (UEdGraphPin* LinkedPin : ParentPin->LinkedTo)
			{
				UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();

				const FPinLink PinLink(ParentPin, LinkedPin, LinkedNode);

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tTrying to iterate link %s"), *PinLink.ToString());

				if (VisitedLinks.Contains(PinLink))
				{
					continue;
				}

				VisitedLinks.Add(PinLink);
				if (!NodePool.Contains(LinkedNode))
				{
					continue;
				}

				if (FBAUtils::IsNodePure(LinkedNode))
				{
					continue;
				}

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tIterating pin link %s"), *PinLink.ToString());

				TSharedPtr<FFormatXInfo> LinkedInfo = MakeShared<FFormatXInfo>(PinLink, CurrentInfo);

				if (ParentPin->Direction == EGPD_Output)
				{
					OutputStack.Push(LinkedInfo);
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\t\t\tAdded to output stack"));
				}
				else
				{
					if (GetMutableDefault<UBASettings>()->FormattingStyle == EBANodeFormattingStyle::Expanded)
					{
						const bool bHasCycle = PendingNodes.Contains(LinkedNode) || FBAUtils::GetExecTree(LinkedNode, EGPD_Input).Contains(CurrentInfo->GetNode());
						//FBAUtils::GetExecTree(LinkedNode, EGPD_Input).Array().FilterByPredicate(OnlySelected).Contains(CurrentInfo->GetNode());
						// const bool bHasCycle = FBAUtils::GetExecTree(LinkedNode, EGPD_Input).Array().FilterByPredicate(OnlySelected).Contains(CurrentInfo->GetNode());
						if (!bHasCycle)
						{
							if (CurrentInfo->Link.GetDirection() == EGPD_Output)
							{
								if (!CurrentInfo->Parent.IsValid() || LinkedNode != CurrentInfo->Parent->GetNode())
								{
									NodesToExpand.Add(CurrentInfo);
									// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\t\t\tExpanding node %s"), *FBAUtils::GetNodeName(LinkedNode));
								}
							}
						}
					}

					InputStack.Push(LinkedInfo);
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\t\t\tAdded to input stack"));
				}

				PendingNodes.Add(LinkedNode);
			}
		}
	}

	if (GetMutableDefault<UBASettings>()->FormattingStyle == EBANodeFormattingStyle::Expanded)
	{
		ExpandPendingNodes(bUseParameter);
	}
}

void FEdGraphFormatter::ExpandByHeight()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ExpandByHeight"), STAT_EdGraphFormatter_ExpandByHeight, STATGROUP_BA_EdGraphFormatter);
	// expand nodes in the output direction for centered branches
	for (UEdGraphNode* Node : NodePool)
	{
		TSharedPtr<FFormatXInfo> Info = FormatXInfoMap[Node];

		const TArray<FPinLink> PinLinks = Info->GetChildrenAsLinks(EGPD_Output);

		if (bCenterBranches && PinLinks.Num() < NumRequiredBranches)
		{
			continue;
		}

		float LargestExpandX = 0;
		for (const FPinLink& Link : PinLinks)
		{
			const FVector2D ToPos = FBAUtils::GetPinPos(GraphHandler, Link.To);
			const FVector2D FromPos = FBAUtils::GetPinPos(GraphHandler, Link.From);

			const float PinDeltaY = FMath::Abs(ToPos.Y - FromPos.Y);
			const float PinDeltaX = FMath::Abs(ToPos.X - FromPos.X);

			// expand to move the node to form a 45 degree angle for the wire (delta x == delta y)
			const float ExpandX = PinDeltaY * 0.75f - PinDeltaX;

			LargestExpandX = FMath::Max(ExpandX, LargestExpandX);
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("Delta X %f | DeltaY %f | ExpandX %f"), PinDeltaX, PinDeltaY, ExpandX);
		}

		if (LargestExpandX <= 0)
		{
			continue;
		}

		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Expanding %s"), *FBAUtils::GetNodeName(Node));
		TArray<UEdGraphNode*> Children = Info->GetChildren(EGPD_Output);
		for (UEdGraphNode* Child : Children)
		{
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tChild %s"), *FBAUtils::GetNodeName(Child));
			Child->NodePosX += LargestExpandX;
			RefreshParameters(Child);
		}
	}
}

void FEdGraphFormatter::ExpandNodesAheadOfParameters()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ExpandNodesAheadOfParameters"), STAT_EdGraphFormatter_ExpandNodesAheadOfParameters, STATGROUP_BA_EdGraphFormatter);
	for (UEdGraphNode* Node : NodePool)
	{
		TSharedPtr<FFormatXInfo> Info = FormatXInfoMap[Node];
		const TArray<FPinLink> PinLinks = Info->GetChildrenAsLinks(EGPD_Output);

		int32 LargestExpandX = 0;
		TArray<UEdGraphNode*> ParameterNodes = FBAUtils::GetLinkedNodes(Node, EGPD_Input).FilterByPredicate(FBAUtils::IsNodePure);

		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Check %s %d"), *FBAUtils::GetNodeName(Node), LargestExpandX);
		for (UEdGraphNode* Param : ParameterNodes)
		{
			if (ParameterParentMap.Contains(Param))
			{
				const auto& ParamFormatter = ParameterParentMap[Param];

				// we only want to move ahead of nodes which aren't our children
				const bool bIsChild = ParamFormatter->GetRootNode() == Node;
				if (!bIsChild && !ParamFormatter->IsUsingHelixing())
				{
					const FSlateRect ParamNodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Param);
					const int32 Delta = FMath::RoundToInt(ParamNodeBounds.Right + PinPadding.X - Node->NodePosX);
					if (Delta > 0)
					{
						LargestExpandX = FMath::Max(Delta, LargestExpandX);
						// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tExpand %s | Param %s, %d"), *FBAUtils::GetNodeName(Node), *FBAUtils::GetNodeName(Param), Delta);
					}
				}
			}
		}

		if (LargestExpandX <= 0)
		{
			continue;
		}

		// UE_LOG(LogTemp, Warning, TEXT("Moving root %s"), *FBAUtils::GetNodeName(Info->GetNode()));
		TSet<TSharedPtr<FFormatXInfo>> TempVisited;
		MoveChildrenX_Recursive(Info, LargestExpandX, TempVisited);
	}
}

void FEdGraphFormatter::FormatY_Recursive(
	const FPinLink& CurrentLink,
	TSet<UEdGraphNode*>& NodesToCollisionCheck,
	TSet<FPinLink>& VisitedLinks,
	const bool bSameRow,
	TSet<UEdGraphNode*>& Children)
{
	// const FString NodeNameA = CurrentNode == nullptr
	// 	? FString("nullptr")
	// 	: FBAUtils::GetNodeName(CurrentNode);
	// const FString PinNameA = CurrentPin == nullptr ? FString("nullptr") : FBAUtils::GetPinName(CurrentPin);
	// const FString NodeNameB = ParentPin == nullptr
	// 	? FString("nullptr")
	// 	: FBAUtils::GetNodeName(ParentPin->GetOwningNode());
	// const FString PinNameB = ParentPin == nullptr ? FString("nullptr") : FBAUtils::GetPinName(ParentPin);
	//
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("FormatY Next : %s | %s || %s | %s"),
	//        *NodeNameA, *PinNameA,
	//        *NodeNameB, *PinNameB);

	UEdGraphNode* CurrentNode = CurrentLink.GetNode();

	if (UEdGraphNode* ParentNode = CurrentLink.GetFromNodeUnsafe())
	{
		NodeRelativeMapping.UpdateRelativeY(CurrentNode, ParentNode);
	}

	for (int CollisionLimit = 0; CollisionLimit < 30; CollisionLimit++)
	{
		bool bNoCollision = true;

		TArray<UEdGraphNode*> NodesCopy = NodesToCollisionCheck.Array();
		while (NodesCopy.Num() > 0)
		{
			UEdGraphNode* NodeToCollisionCheck = NodesCopy.Pop();

			if (NodeToCollisionCheck == CurrentNode)
			{
				continue;
			}

			if (CurrentLink.GetFromNodeUnsafe() == NodeToCollisionCheck)
			{
				continue;
			}

			FSlateRect MyBounds = GetClusterBounds(CurrentNode);
			const FMargin CollisionPadding(0, 0, 0, NodePadding.Y);

			FSlateRect OtherBounds = GetClusterBounds(NodeToCollisionCheck);

			OtherBounds = OtherBounds.ExtendBy(CollisionPadding);

			if (FSlateRect::DoRectanglesIntersect(MyBounds, OtherBounds))
			{
				bNoCollision = false;
				const int32 Delta = OtherBounds.Bottom - MyBounds.Top;

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Collision between %d | %s and %s"),
				// 	Delta + 1,
				// 	*FBAUtils::GetNodeName(CurrentNode),
				// 	*FBAUtils::GetNodeName(NodeToCollisionCheck));
				//
				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *MyBounds.ToString());
				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *OtherBounds.ToString());

					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tMoved node single %s"), *FBAUtils::GetNodeName(CurrentNode));
				CurrentNode->NodePosY += Delta + 1;

				CurrentNode->NodePosY = FBAUtils::AlignTo8x8Grid(CurrentNode->NodePosY);

				RefreshParameters(CurrentNode);
				NodeRelativeMapping.UpdateRelativeY(CurrentNode, NodeToCollisionCheck);
			}
		}

		if (bNoCollision)
		{
			break;
		}
	}

	NodesToCollisionCheck.Emplace(CurrentNode);

	const EEdGraphPinDirection ParentDirection = CurrentLink.GetDirection();

	bool bFirstPin = true;

	UEdGraphPin* MainPin = CurrentLink.To;

	bool bCenteredParent = false;

	TArray<EEdGraphPinDirection> Directions = { ParentDirection, UEdGraphPin::GetComplementaryDirection(ParentDirection) };
	for (EEdGraphPinDirection CurrentDirection : Directions)
	{
		TArray<FPinLink> PinLinks = FBAUtils::GetPinLinks(CurrentNode, CurrentDirection);
		PinLinks.StableSort([&GraphHandler = GraphHandler](const FPinLink& A, const FPinLink& B)
		{
			return GraphHandler->GetPinY(A.From) < GraphHandler->GetPinY(B.From);
		});

		UEdGraphPin* LastLinked = CurrentLink.To;
		UEdGraphPin* LastProcessed = nullptr;

		TArray<ChildBranch> ChildBranches;

		for (FPinLink& Link : PinLinks)
		{
			UEdGraphNode* ToNode = Link.GetToNodeUnsafe();

			const bool bIsSameLink = Path.Contains(Link);

			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tIter Child %s"), *FBAUtils::GetNodeName(OtherNode));
			//
			// if (!bIsSameLink)
			// {
			// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tNot same link!"));
			// }

			if (VisitedLinks.Contains(Link)
				|| !NodePool.Contains(ToNode)
				|| FBAUtils::IsNodePure(ToNode)
				|| NodesToCollisionCheck.Contains(ToNode)
				|| !bIsSameLink)
			{
				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tSkipping child"));
				continue;
			}
			VisitedLinks.Add(Link);

			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tTaking Child %s"), *FBAUtils::GetNodeName(OtherNode));

			FBAUtils::StraightenPin(GraphHandler, Link.From, Link.To);

			// bool bChildIsSameRow = false;
			bool bChildIsSameRow = IsSameRow(Link);

			if (bFirstPin && (CurrentLink.From == nullptr || Link.GetDirection() == CurrentLink.GetDirection()))
			{
				// bChildIsSameRow = true;
				bFirstPin = false;
				// UE_LOG(LogBlueprintAssist, Error, TEXT("\t\tNode %s is same row as %s"),
				//        *FBAUtils::GetNodeName(OtherNode),
				//        *FBAUtils::GetNodeName(CurrentNode));
			}
			else
			{
				if (LastProcessed != nullptr)
				{
					//UE_LOG(LogBlueprintAssist, Warning, TEXT("Moved node %s to %s"), *FBAUtils::GetNodeName(OtherNode), *FBAUtils::GetNodeName(LastPinOther->GetOwningNode()));
					int32 NewNodePosY = FMath::Max(ToNode->NodePosY, LastProcessed->GetOwningNode()->NodePosY);
					NewNodePosY = FBAUtils::SnapToGrid(NewNodePosY, EBARoundingMethod::Round, 8); 
					FBAUtils::SetNodePosY(GraphHandler, ToNode, NewNodePosY);
				}
			}

			RefreshParameters(ToNode);

			TSet<UEdGraphNode*> LocalChildren;
			FormatY_Recursive(Link, NodesToCollisionCheck, VisitedLinks, bChildIsSameRow, LocalChildren);
			Children.Append(LocalChildren);

			if (FormatXInfoMap[CurrentNode]->GetImmediateChildren().Contains(ToNode))
			{
				ChildBranches.Add(ChildBranch(Link.To, Link.From, LocalChildren));
			}

			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Local children for %s"), *FBAUtils::GetNodeName(CurrentNode));
			//for (UEdGraphNode* Node : LocalChildren)
			//{
			//	UE_LOG(LogBlueprintAssist, Warning, TEXT("\tChild %s"), *FBAUtils::GetNodeName(Node));
			//}

			if (!bChildIsSameRow && LocalChildren.Num() > 0)
			{
				UEdGraphPin* PinToAvoid = LastLinked;
				if (MainPin != nullptr)
				{
					PinToAvoid = MainPin;
					MainPin = nullptr;
				}

				if (PinToAvoid != nullptr)
				{
					FSlateRect Bounds = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, LocalChildren.Array());

					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tPin to avoid %s (%s)"), *FBAUtils::GetPinName(PinToAvoid), *FBAUtils::GetPinName(OtherPin));
					const float PinPos = GraphHandler->GetPinY(PinToAvoid) + VerticalPinSpacing;
					const float Delta = PinPos - Bounds.Top;

					if (Delta > 0)
					{
						for (UEdGraphNode* Child : LocalChildren)
						{
							Child->NodePosY += Delta;
							RefreshParameters(Child);
						}
					}
				}
			}

			LastProcessed = Link.To;
			LastLinked = Link.From;
		}

		if (bCenterBranches && ChildBranches.Num() >= NumRequiredBranches && ParentDirection == EGPD_Output)
		{
			if (CurrentDirection != ParentDirection)
			{
				bCenteredParent = true;
			}

			CenterBranches(CurrentNode, ChildBranches, NodesToCollisionCheck);
		}
	}

	Children.Add(CurrentNode);

	if (bSameRow && CurrentLink.From != nullptr && !bCenteredParent)
	{
		// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tStraightening pin from %s to %s"),
		//        *FBAUtils::GetPinName(CurrentPin),
		//        *FBAUtils::GetPinName(ParentPin));

		FBAUtils::StraightenPin(GraphHandler, CurrentLink.To, CurrentLink.From);
		RefreshParameters(CurrentLink.GetFromNodeUnsafe());
	}
}

void FEdGraphFormatter::GetPinsOfSameHeight_Recursive(
	UEdGraphNode* CurrentNode,
	UEdGraphPin* CurrentPin,
	UEdGraphPin* ParentPin,
	TSet<UEdGraphNode*>& NodesToCollisionCheck,
	TSet<FPinLink>& VisitedLinks)
{
	NodesToCollisionCheck.Emplace(CurrentNode);

	TArray<TArray<UEdGraphPin*>> OutputInput;

	bool bFirstPin = true;

	auto& GraphHandlerCapture = GraphHandler;

	auto LinkedToSorter = [&GraphHandlerCapture, &NodesToCollisionCheck](UEdGraphPin& PinA, UEdGraphPin& PinB)
	{
		struct FLocal
		{
			static void GetPins(UEdGraphPin* NextPin, TSet<UEdGraphNode*>& VisitedNodes, TArray<UEdGraphPin*>& OutPins, bool& bHasEventNode, int32& DepthToEventNode, int32 TempDepth)
			{
				if (FBAUtils::IsEventNode(NextPin->GetOwningNode()))
				{
					DepthToEventNode = TempDepth;
					bHasEventNode = true;
				}

				if (VisitedNodes.Contains(NextPin->GetOwningNode()))
				{
					OutPins.Add(NextPin);
					return;
				}

				VisitedNodes.Add(NextPin->GetOwningNode());

				auto NextPins = FBAUtils::GetLinkedToPins(NextPin->GetOwningNode(), EGPD_Input);

				for (UEdGraphPin* Pin : NextPins)
				{
					GetPins(Pin, VisitedNodes, OutPins, bHasEventNode, DepthToEventNode, TempDepth + 1);
				}
			}

			static UEdGraphPin* HighestPin(TSharedPtr<FBAGraphHandler> GraphHandler, UEdGraphPin* Pin, TSet<UEdGraphNode*>& VisitedNodes, bool& bHasEventNode, int32& DepthToEventNode)
			{
				TArray<UEdGraphPin*> OutPins;
				GetPins(Pin, VisitedNodes, OutPins, bHasEventNode, DepthToEventNode, 0);

				if (OutPins.Num() == 0)
				{
					return nullptr;
				}

				OutPins.StableSort([GraphHandler](UEdGraphPin& PinA, UEdGraphPin& PinB)
				{
					const FVector2D PinPosA = FBAUtils::GetPinPos(GraphHandler, &PinA);
					const FVector2D PinPosB = FBAUtils::GetPinPos(GraphHandler, &PinB);

					if (PinPosA.X != PinPosB.X)
					{
						return PinPosA.X < PinPosB.X;
					}

					return PinPosA.Y < PinPosB.Y;
				});

				return OutPins[0];
			}
		};

		bool bHasEventNodeA = false;
		int32 DepthToEventNodeA = 0;

		auto VisitedNodesCopyA = NodesToCollisionCheck;
		UEdGraphPin* HighestPinA = FLocal::HighestPin(GraphHandlerCapture, &PinA, VisitedNodesCopyA, bHasEventNodeA, DepthToEventNodeA);
		bool bHasEventNodeB = false;
		int32 DepthToEventNodeB = 0;
		auto VisitedNodesCopyB = NodesToCollisionCheck;
		UEdGraphPin* HighestPinB = FLocal::HighestPin(GraphHandlerCapture, &PinB, VisitedNodesCopyB, bHasEventNodeB, DepthToEventNodeB);

		if (HighestPinA == nullptr || HighestPinB == nullptr)
		{
			if (bHasEventNodeA != bHasEventNodeB)
			{
				return bHasEventNodeA > bHasEventNodeB;
			}

			return DepthToEventNodeA > DepthToEventNodeB;
		}

		const FVector2D PinPosA = FBAUtils::GetPinPos(GraphHandlerCapture, HighestPinA);
		const FVector2D PinPosB = FBAUtils::GetPinPos(GraphHandlerCapture, HighestPinB);

		if (PinPosA.X != PinPosB.X)
		{
			return PinPosA.X < PinPosB.X;
		}

		return PinPosA.Y < PinPosB.Y;
	};

	const EEdGraphPinDirection ParentDirection = ParentPin == nullptr ? EGPD_Output : ParentPin->Direction.GetValue();
	TArray<EEdGraphPinDirection> Directions = { ParentDirection, UEdGraphPin::GetComplementaryDirection(ParentDirection) };
	for (auto CurrentDirection : Directions)
	{
		TArray<UEdGraphPin*> Pins = FBAUtils::GetLinkedPins(CurrentNode, CurrentDirection)
			.FilterByPredicate(FBAUtils::IsExecOrDelegatePin)
			.FilterByPredicate(FBAUtils::IsPinLinked);

		Pins.StableSort([&GraphHandler = GraphHandler](const UEdGraphPin& A, const UEdGraphPin& B)
		{
			return GraphHandler->GetPinY(&A) < GraphHandler->GetPinY(&B);
		});

		TArray<FPinLink> LinksToIterate;
		for (UEdGraphPin* MyPin : Pins)
		{
			TArray<UEdGraphPin*> LinkedPins = MyPin->LinkedTo;

			if (MyPin->Direction == EGPD_Input && GetMutableDefault<UBASettings>()->FormattingStyle == EBANodeFormattingStyle::Expanded)
			{
				LinkedPins.StableSort(LinkedToSorter);
			}

			for (int i = 0; i < LinkedPins.Num(); ++i)
			{
				UEdGraphPin* OtherPin = LinkedPins[i];
				UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
				FPinLink Link(MyPin, OtherPin);


				// UE_LOG(LogBlueprintAssist, Warning, TEXT("TRY Iterating (%s) %s"), *FBAUtils::GetNodeName(CurrentNode), *Link.ToString());
				if (VisitedLinks.Contains(Link)
					|| !NodePool.Contains(OtherNode)
					|| FBAUtils::IsNodePure(OtherNode))
				{
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tSkipping"));
					continue;
				}

				if (NodesToCollisionCheck.Contains(OtherNode))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping visited"));
					continue;
				}

				if (!Path.Contains(Link))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping path"));
					continue;
				}

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("INITIAL Iterating (%s) %s"), *FBAUtils::GetNodeName(CurrentNode), *Link.ToString());

				VisitedLinks.Add(Link);
				if (bFirstPin && (ParentPin == nullptr || MyPin->Direction == ParentPin->Direction))
				{
					// for (auto& VisitedLink : VisitedLinks)
					// {
					// 	UE_LOG(LogTemp, Warning, TEXT("\t\t\t\t%s"), *VisitedLink.ToString());
					// }
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tSame row? %s"), *Link.ToString());
					SameRowMapping.Add(Link, true);
					SameRowMapping.Add(FPinLink(OtherPin, MyPin), true);
					SameRowMappingDirect.Add(OtherPin, MyPin);
					SameRowMappingDirect.Add(MyPin, OtherPin);
					bFirstPin = false;
				}

				GetPinsOfSameHeight_Recursive(OtherNode, OtherPin, MyPin, NodesToCollisionCheck, VisitedLinks);
			}
		}
	}
}

bool FEdGraphFormatter::LinkToSort(UEdGraphPin& PinA, UEdGraphPin& PinB, TSet<UEdGraphNode*>& VisitedNodes)
{
	struct FLocal
	{
		static void GetPins(UEdGraphPin* NextPin, TSet<UEdGraphNode*>& VisitedNodes, TArray<UEdGraphPin*>& OutPins, bool& bHasEventNode, int32& DepthToEventNode, int32 TempDepth)
		{
			if (FBAUtils::IsEventNode(NextPin->GetOwningNode()))
			{
				DepthToEventNode = TempDepth;
				bHasEventNode = true;
			}

			if (VisitedNodes.Contains(NextPin->GetOwningNode()))
			{
				OutPins.Add(NextPin);
				return;
			}

			VisitedNodes.Add(NextPin->GetOwningNode());

			auto NextPins = FBAUtils::GetLinkedToPins(NextPin->GetOwningNode(), EGPD_Input);

			for (UEdGraphPin* Pin : NextPins)
			{
				GetPins(Pin, VisitedNodes, OutPins, bHasEventNode, DepthToEventNode, TempDepth + 1);
			}
		}

		static UEdGraphPin* HighestPin(TSharedPtr<FBAGraphHandler> GraphHandler, UEdGraphPin* Pin, TSet<UEdGraphNode*>& VisitedNodes, bool& bHasEventNode, int32& DepthToEventNode)
		{
			TArray<UEdGraphPin*> OutPins;
			GetPins(Pin, VisitedNodes, OutPins, bHasEventNode, DepthToEventNode, 0);

			if (OutPins.Num() == 0)
			{
				return nullptr;
			}

			OutPins.StableSort([GraphHandler](UEdGraphPin& PinA, UEdGraphPin& PinB)
			{
				const FVector2D PinPosA = FBAUtils::GetPinPos(GraphHandler, &PinA);
				const FVector2D PinPosB = FBAUtils::GetPinPos(GraphHandler, &PinB);

				if (PinPosA.X != PinPosB.X)
				{
					return PinPosA.X < PinPosB.X;
				}

				return PinPosA.Y < PinPosB.Y;
			});

			return OutPins[0];
		}
	};

	bool bHasEventNodeA = false;
	int32 DepthToEventNodeA = 0;

	auto VisitedNodesCopyA = VisitedNodes;
	UEdGraphPin* HighestPinA = FLocal::HighestPin(GraphHandler, &PinA, VisitedNodesCopyA, bHasEventNodeA, DepthToEventNodeA);
	bool bHasEventNodeB = false;
	int32 DepthToEventNodeB = 0;
	auto VisitedNodesCopyB = VisitedNodes;
	UEdGraphPin* HighestPinB = FLocal::HighestPin(GraphHandler, &PinB, VisitedNodesCopyB, bHasEventNodeB, DepthToEventNodeB);

	if (HighestPinA == nullptr || HighestPinB == nullptr)
	{
		if (bHasEventNodeA != bHasEventNodeB)
		{
			return bHasEventNodeA > bHasEventNodeB;
		}

		return DepthToEventNodeA > DepthToEventNodeB;
	}

	const FVector2D PinPosA = FBAUtils::GetPinPos(GraphHandler, HighestPinA);
	const FVector2D PinPosB = FBAUtils::GetPinPos(GraphHandler, HighestPinB);

	if (PinPosA.X != PinPosB.X)
	{
		return PinPosA.X < PinPosB.X;
	}

	return PinPosA.Y < PinPosB.Y;;
}

void FEdGraphFormatter::WrapNodes()
{
	TArray<UEdGraphNode*> PendingNodes;
	PendingNodes.Push(RootNode);

	TSet<UEdGraphNode*> VisitedNodes;

	const float RootPos = RootNode->NodePosX;

	while (PendingNodes.Num() > 0)
	{
		UEdGraphNode* NextNode = PendingNodes.Pop();
		if (NextNode->NodePosX - RootPos > 1000)
		{
			TSharedPtr<FFormatXInfo> Info = FormatXInfoMap[NextNode];
			TArray<UEdGraphNode*> Children = Info->GetChildren(EGPD_Output);

			float Offset = RootPos - NextNode->NodePosX;
			NextNode->NodePosX += Offset;
			NextNode->NodePosY += 500;

			for (UEdGraphNode* Child : Children)
			{
				Child->NodePosX += Offset;
				Child->NodePosY += 500;
			}
		}

		TArray<UEdGraphNode*> OutputNodes = FBAUtils::GetLinkedNodes(NextNode, EGPD_Output);

		for (UEdGraphNode* Node : OutputNodes)
		{
			if (VisitedNodes.Contains(Node))
			{
				continue;
			}

			VisitedNodes.Add(Node);
			PendingNodes.Add(Node);
		}
	}
}

void FEdGraphFormatter::ApplyCommentPaddingY()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ApplyCommentPaddingY"), STAT_EdGraphFormatter_ApplyCommentPaddingY, STATGROUP_BA_EdGraphFormatter);

	if (CommentHandler.GetComments().Num() == 0)
	{
		return;
	}

	// UE_LOG(LogTemp, Error, TEXT("APPLY COMMENT PADDING Y"));
	TArray<UEdGraphNode*> NodeSet = GetNodePool();
	TArray<FPinLink> LeafLinks;

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : CommentHandler.GetRootNodes())
	{
		for (UEdGraphNode* Node : ContainsNode->AllContainedNodes)
		{
			NodeSet.RemoveSwap(Node);
		}
	}

	ApplyCommentPaddingY_Recursive(NodeSet, CommentHandler.GetRootNodes().Array());
	// UE_LOG(LogTemp, Error, TEXT("END EXPAND COMMENTS Y"));
}

void FEdGraphFormatter::ApplyCommentPaddingY_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes)
{
	// TODO this is slow, nodepool should probably be a TSet
	NodeSet.RemoveAll([&NodePool = NodePool](UEdGraphNode* Node)
	{
		return !NodePool.Contains(Node);
	});

	for (TSharedPtr<FBACommentContainsNode> Contains : ContainsNodes)
	{
		NodeSet.Add(Contains->Comment);
	}

	NodeSet.StableSort([&](UEdGraphNode& NodeA, UEdGraphNode& NodeB)
	{
		float TopA = GetNodeBounds(&NodeA, true).Top;
		if (auto Comment = Cast<UEdGraphNode_Comment>(&NodeA))
		{
			auto Nodes = FBAUtils::GetNodesUnderComment(Comment);
			Nodes.RemoveAll(FBAUtils::IsCommentNode);
			TopA = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes).Top;
		}

		float TopB = GetNodeBounds(&NodeB, true).Top;
		if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(&NodeB))
		{
			auto Nodes = FBAUtils::GetNodesUnderComment(Comment);
			Nodes.RemoveAll(FBAUtils::IsCommentNode);
			TopB = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes).Top;
		}
		return TopA < TopB;
	});

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
	{
		ApplyCommentPaddingY_Recursive(ContainsNode->OwnedNodes, ContainsNode->Children);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Format SubGraph"));
	// for (UEdGraphNode* Node : NodeSet)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	// }

	// collision check each node in the NodeSet
	for (int IndexA = 0; IndexA < NodeSet.Num(); ++IndexA)
	{
		for (int IndexB = IndexA + 1; IndexB < NodeSet.Num(); ++IndexB)
		{
			UEdGraphNode* NodeA = NodeSet[IndexA];
			UEdGraphNode* NodeB = NodeSet[IndexB];

			UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA);
			UEdGraphNode_Comment* CommentB = Cast<UEdGraphNode_Comment>(NodeB);

			if (CommentA)
			{
				if (CommentB) // check if the comments are intersecting
				{
					if (AreCommentsIntersecting(CommentA, CommentB))
					{
						continue;
					}
				}
				else // check if CommentA contains any of NodeB's parameter nodes
				{
					if (TSharedPtr<FEdGraphParameterFormatter> ParamFormatter = GetParameterFormatter(NodeB))
					{
						TSet<UEdGraphNode*> NodeAContains(CommentHandler.GetNodesUnderComments(CommentA));
						const TSet<UEdGraphNode*> Intersection = NodeAContains.Intersect(ParamFormatter->GetFormattedNodes());

						// FBAUtils::PrintNodeArray(NodeAContains.Array());
						// FBAUtils::PrintNodeArray(ParamFormatter->GetFormattedNodes().Array());
						
						if (Intersection.Num() > 0)
						{
							continue;
						}
					}
				}
			}

			FSlateRect BoundsA = GetNodeBounds(NodeA, true);
			BoundsA.Bottom += NodePadding.Y;

			FSlateRect BoundsB = GetNodeBounds(NodeB, true);

			// UE_LOG(LogTemp, Warning, TEXT("{%s} Checking {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

			// if we collide, move NodeB down to resolve the collision
			if (FSlateRect::DoRectanglesIntersect(BoundsA, BoundsB))
			{
				TSet<UEdGraphNode*> Visited;

				// Don't move NodeA
				if (CommentA)
				{
					Visited.Append(CommentHandler.ContainsGraph->GetNode(CommentA)->AllContainedNodes);
				}
				else
				{
					Visited.Add(NodeA);
				}

				const float Delta = BoundsA.Bottom + 1.0f - BoundsB.Top;

				SetNodeY_KeepingSpacingVisited(NodeB, NodeB->NodePosY + Delta, Visited);

				// UE_LOG(LogTemp, Warning, TEXT("COLLIDING: {%s} {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));
				// FBAUtils::PrintNodeArray(Visited.Array(), "Moved");
			}
		}
	}
}

void FEdGraphFormatter::ApplyCommentPaddingAfterKnots()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ApplyCommentPaddingAfterKnots"), STAT_EdGraphFormatter_ApplyCommentPaddingAfterKnots, STATGROUP_BA_EdGraphFormatter);

	if (CommentHandler.GetComments().Num() == 0)
	{
		return;
	}

	// UE_LOG(LogTemp, Error, TEXT("APPLY COMMENT PADDING AFTER KNOTS"));

	TArray<UEdGraphNode*> NodeSet = GetNodePool();
	TArray<FPinLink> LeafLinks;

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : CommentHandler.GetRootNodes())
	{
		for (UEdGraphNode* Node : ContainsNode->AllContainedNodes)
		{
			NodeSet.RemoveSwap(Node);
		}
	}

	ApplyCommentPaddingAfterKnots_Recursive(NodeSet, CommentHandler.GetRootNodes().Array());
}

void FEdGraphFormatter::ApplyCommentPaddingAfterKnots_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes)
{
	NodeSet.RemoveAll([&NodePool = NodePool](UEdGraphNode* Node)
	{
		return !NodePool.Contains(Node);
	});

	for (TSharedPtr<FBACommentContainsNode> Contains : ContainsNodes)
	{
		NodeSet.Add(Contains->Comment);
	}

	NodeSet.StableSort([&](UEdGraphNode& NodeA, UEdGraphNode& NodeB)
	{
		float TopA = GetNodeBounds(&NodeA, true).Top;
		if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(&NodeA))
		{
			auto Nodes = FBAUtils::GetNodesUnderComment(Comment);
			Nodes.RemoveAll(FBAUtils::IsCommentNode);
			TopA = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes).Top;
		}

		float TopB = GetNodeBounds(&NodeB, true).Top;
		if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(&NodeB))
		{
			auto Nodes = FBAUtils::GetNodesUnderComment(Comment);
			Nodes.RemoveAll(FBAUtils::IsCommentNode);
			TopB = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes).Top;
		}
		return TopA < TopB;
	});

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
	{
		ApplyCommentPaddingAfterKnots_Recursive(ContainsNode->OwnedNodes, ContainsNode->Children);
	}

	auto AllNodes = NodeSet;

	NodeSet.RemoveAll([&](UEdGraphNode* Node)
	{
		// don't collision check aligned nodes
		const UK2Node_Knot* Knot = Cast<UK2Node_Knot>(Node);
		if (Knot)
		{
			if (KnotTrackCreator.IsPinAlignedKnot(Knot))
			{
				return true;
			}
		}

		return false;
	});

	// UE_LOG(LogTemp, Warning, TEXT("Format SubGraph"));
	// for (UEdGraphNode* Node : NodeSet)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	// }

	TSet<UEdGraphNode*> IgnoredNodes;
	for (UEdGraphNode* NodeA : NodeSet)
	{
		IgnoredNodes.Add(NodeA);
		for (UEdGraphNode* NodeB : NodeSet)
		{
			if (NodeA == NodeB)
			{
				continue;
			}

			// UE_LOG(LogTemp, Warning, TEXT("Checking %s | %s"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

			UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA);
			UEdGraphNode_Comment* CommentB = Cast<UEdGraphNode_Comment>(NodeB);
			UK2Node_Knot* KnotA = Cast<UK2Node_Knot>(NodeA);
			UK2Node_Knot* KnotB = Cast<UK2Node_Knot>(NodeB);

			// if (!(CommentA && KnotB || CommentB && KnotA))
			if (!CommentA && !CommentB)
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tSkipping"));
				continue;
			}

			if (CommentA && CommentB)
			{
				if (AreCommentsIntersecting(CommentA, CommentB))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping"));
					continue;
				}
			}

			// UE_LOG(LogTemp, Warning, TEXT("Checking %s | %s"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

			FSlateRect BoundsA = GetNodeBounds(NodeA, true);
			FSlateRect BoundsB = GetNodeBounds(NodeB, true);

			const auto KnotOrNodeSet = [&](UEdGraphNode* Node)
			{
				return FBAUtils::IsKnotNode(Node) || AllNodes.Contains(Node);
			};

			if (CommentA)
			{
				const TArray<UEdGraphNode*>& Contains = CommentHandler.ContainsGraph->GetNode(CommentA)->AllContainedNodes;
				IgnoredNodes.Append(Contains);
				BoundsA = CommentHandler.GetCommentBounds(CommentA);
			}

			if (KnotA)
			{
				if (auto Group = KnotTrackCreator.GetKnotGroup(KnotA))
				{
					const auto Knots = Group->GetKnots();
					IgnoredNodes.Append(Group->GetKnots());
					BoundsA = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Knots);
				}
			}

			BoundsA = BoundsA.ExtendBy(FMargin(0, 0, 0, NodePadding.Y));

			if (CommentB)
			{
				BoundsB = CommentHandler.GetCommentBounds(CommentB);
			}

			if (KnotB)
			{
				if (auto Group = KnotTrackCreator.GetKnotGroup(KnotB))
				{
					BoundsB = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Group->GetKnots());
				}
			}

			struct FLocal
			{
				static bool KnotAndCommentOverlap(UK2Node_Knot* Knot, UEdGraphNode_Comment* Comment, FKnotTrackCreator& KnotCreator)
				{
					if (!Knot || !Comment)
					{
						return false;
					}

					if (KnotCreator.IsKnotInsideComment(Knot))
					{
						return false;
					}

					TArray<UEdGraphNode*> RelatedNodes = KnotCreator.GetKnotCreation(Knot)->OwningKnotTrack->GetRelatedNodes();

					const auto NodesInComment = FBAUtils::GetNodesUnderComment(Comment);

					return NodesInComment.ContainsByPredicate([&RelatedNodes](UEdGraphNode* Node)
					{
						return RelatedNodes.Contains(Node);
					});
				}
			};

			if (FLocal::KnotAndCommentOverlap(KnotA, CommentB, KnotTrackCreator) || FLocal::KnotAndCommentOverlap(KnotB, CommentA, KnotTrackCreator))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tSkipping knot overlaps"));
				continue;
			}

			// UE_LOG(LogTemp, Warning, TEXT("{%s} %d Checking {%s} %d"),
			// 	*FBAUtils::GetNodeName(NodeA),
			// 	CommentA != nullptr,
			// 	*FBAUtils::GetNodeName(NodeB),
			// 	CommentB != nullptr
			// );

			if (FSlateRect::DoRectanglesIntersect(BoundsA, BoundsB))
			{
				const float Delta = BoundsA.Bottom + 1.0f - BoundsB.Top;

				// UE_LOG(LogTemp, Warning, TEXT("\tCollision!"));
				// UE_LOG(LogTemp, Warning, TEXT("\t%s"), *BoundsA.ToString());
				// UE_LOG(LogTemp, Warning, TEXT("\t%s"), *BoundsB.ToString());
				// UE_LOG(LogTemp, Warning, TEXT("\t%s"), *BoundsA.GetSize().ToString());
				// UE_LOG(LogTemp, Warning, TEXT("\t%s"), *BoundsB.GetSize().ToString());
				// UE_LOG(LogTemp, Warning, TEXT("\tMove %s %f"), *FBAUtils::GetNodeName(NodeB), Delta);

				TSet<UEdGraphNode*> Visited;
				SetNodeY_KeepingSpacingVisited(NodeB, NodeB->NodePosY + Delta, Visited);

				// FBAUtils::PrintNodeArray(Visited.Array(), "VisitedNodes");
			}
		}
	}
}

void FEdGraphFormatter::ApplyCommentPaddingX()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ApplyCommentPaddingX"), STAT_EdGraphFormatter_ApplyCommentPaddingX, STATGROUP_BA_EdGraphFormatter);
	// UE_LOG(LogTemp, Error, TEXT("EXPAND COMMENTS X"));

	TArray<FPinLink> LeafLinks;
	TArray<UEdGraphNode*> Contains = GetNodePool();
	for (TSharedPtr<FBACommentContainsNode> ContainsNode : CommentHandler.GetRootNodes())
	{
		for (UEdGraphNode* Node : ContainsNode->AllContainedNodes)
		{
			Contains.RemoveSwap(Node);
		}
	}

	ApplyCommentPaddingX_Recursive(Contains, CommentHandler.GetRootNodes().Array(), LeafLinks);
	// UE_LOG(LogTemp, Error, TEXT("END EXPAND COMMENTS X"));
}

void FEdGraphFormatter::ApplyCommentPaddingX_Recursive(
	TArray<UEdGraphNode*> NodeSet,
	TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes,
	TArray<FPinLink>& OutLeafLinks)
{
	NodeSet.RemoveAll([&NodePool = NodePool](UEdGraphNode* Node)
	{
		return !NodePool.Contains(Node);
	});

	for (TSharedPtr<FBACommentContainsNode> Contains : ContainsNodes)
	{
		NodeSet.Add(Contains->Comment);
	}

	const auto LeftMost = [&](UEdGraphNode& NodeA, UEdGraphNode& NodeB)
	{
		const float LeftA = GetNodeBounds(&NodeA, true).Left;
		const float LeftB = GetNodeBounds(&NodeB, true).Left;
		return LeftA < LeftB;
	};

	NodeSet.StableSort(LeftMost);

	TArray<FPinLink> LeafLinks;
	for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
	{
		ApplyCommentPaddingX_Recursive(ContainsNode->OwnedNodes, ContainsNode->Children, LeafLinks);
	}

	TArray<UEdGraphNode*> AllNodes = NodeSet;

	// UE_LOG(LogTemp, Warning, TEXT("Format SubGraph"));
	// for (UEdGraphNode* Node : NodeSet)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\tNodeSet %s"), *FBAUtils::GetNodeName(Node));
	// }
	//
	// for (auto& Node : ContainsNodes)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\tComments %s"), *FBAUtils::GetNodeName(Node->Comment));
	// }

	for (UEdGraphNode* NodeA : NodeSet)
	{
		// collide only with our children
		TSet<TSharedPtr<FFormatXInfo>> Children;
		if (UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA))
		{
			for (UEdGraphNode* Node : CommentHandler.ContainsGraph->GetNode(CommentA)->AllContainedNodes)
			{
				if (TSharedPtr<FFormatXInfo> FormatXInfo = GetFormatXInfo(Node))
				{
					Children.Append(FormatXInfo->Children);
				}
			}
		}
		else
		{
			if (TSharedPtr<FFormatXInfo> FormatXInfo = GetFormatXInfo(NodeA))
			{
				Children.Append(FormatXInfo->Children);
			}
		}

		// gather leaf links
		TArray<FPinLink> LinksInNodeSet;
		TArray<FPinLink> PotentialLeafLinks;
		for (TSharedPtr<FFormatXInfo> Info : Children)
		{
			if (!IsSameRow(Info->Link))
			{
				continue;
			}

			if (AllNodes.Contains(Info->Link.GetNode()))
			{
				LinksInNodeSet.Add(Info->Link);
			}
			else
			{
				PotentialLeafLinks.Add(Info->Link);
			}
		}

		if (LinksInNodeSet.Num() == 0)
		{
			for (FPinLink& Link : PotentialLeafLinks)
			{
				OutLeafLinks.Add(Link);
				OutLeafLinks.Add(Link.MakeOppositeLink());
				// UE_LOG(LogTemp, Warning, TEXT("Leaf link %s"), *Link.ToString());
			}
		}

		// UE_LOG(LogTemp, Warning, TEXT("Checking collision for %s against %d children"), *FBAUtils::GetNodeName(NodeA), Children.Num());

		for (TSharedPtr<FFormatXInfo> Info : Children)
		{
			auto NodeB = Info->GetNode();

			// UE_LOG(LogTemp, Warning, TEXT("TRY {%s}"), *Info->Link.ToString());

			if (!IsSameRow(Info->Link) && !LeafLinks.Contains(Info->Link))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tSkipping not same row"));
				continue;
			}

			if (!NodeSet.Contains(NodeB))
			{
				bool bHasContainingComment = false;
				for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
				{
					// UE_LOG(LogTemp, Warning, TEXT("%s not in node set checking %s"), *FBAUtils::GetNodeName(NodeB), *FBAUtils::GetNodeName(ContainsNode->Comment));

					// if (CommentHandler.DoesCommentContainNode(ContainsNode->Comment, NodeB))
					if (ContainsNode->AllContainedNodes.Contains(NodeB))
					{
						NodeB = ContainsNode->Comment;
						bHasContainingComment = true;
						break;
					}
				}

				if (!bHasContainingComment)
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkip not in nodeset"));
					continue;
				}
			}
			
			if (NodeA == NodeB)
			{
				continue;
			}

			UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA);
			UEdGraphNode_Comment* CommentB = Cast<UEdGraphNode_Comment>(NodeB);

			// only collision check comment nodes
			if (!CommentA && !CommentB)
			{
				continue;
			}
			
			if (CommentA && CommentB)
			{
				if (AreCommentsIntersecting(CommentA, CommentB))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkip comments intersecting"));
					continue;
				}
			}

			FSlateRect BoundsA = GetNodeBounds(NodeA, true).ExtendBy(FMargin(NodePadding.X, 0.f));
			FSlateRect BoundsB = GetNodeBounds(NodeB, true);

			if (CommentA)
			{
				BoundsA = CommentHandler.GetCommentBounds(CommentA).ExtendBy(FMargin(NodePadding.X, 0.f));
			}
			
			if (CommentB)
			{
				BoundsB = CommentHandler.GetCommentBounds(CommentB);
			}

			// UE_LOG(LogTemp, Warning, TEXT("{%s} Checking {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));
			
			if (FSlateRect::DoRectanglesIntersect(BoundsA, BoundsB))
			{
				const float Delta = Info->Link.GetDirection() == EGPD_Output ?
					BoundsA.Right + 1.0f - BoundsB.Left :
					BoundsA.Left - BoundsB.Right;
				
				if (CommentB)
				{
					TSet<UEdGraphNode*> AllChildren;
					for (auto Node : CommentHandler.ContainsGraph->GetNode(CommentB)->AllContainedNodes)
					{
						if (!FormatXInfoMap.Contains(Node))
						{
							continue;
						}

						AllChildren.Add(Node);
						AllChildren.Append(FormatXInfoMap[Node]->GetChildren());
					}

					// UE_LOG(LogTemp, Warning, TEXT("Node {%s} Colliding with COMMENT {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

					for (auto Child : AllChildren)
					{
						Child->NodePosX += Delta;

						Child->NodePosX = FBAUtils::AlignTo8x8Grid(Child->NodePosX);

						RefreshParameters(Child);
						// UE_LOG(LogTemp, Warning, TEXT("\tMove child %s"), *FBAUtils::GetNodeName(Child));
					}

					// UE_LOG(LogTemp, Warning, TEXT("CommentBounds %s"), *BoundsB.ToString());
					// UE_LOG(LogTemp, Warning, TEXT("RegularBounds %s"), *FBAUtils::GetCachedNodeArrayBounds(GraphHandler, CommentHandler.CommentNodesContains[CommentB]).ToString());
				}
				else
				{
					if (!FormatXInfoMap.Contains(NodeB))
					{
						continue;
					}

					// UE_LOG(LogTemp, Warning, TEXT("COMMENT {%s} Colliding with Node {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

					NodeB->NodePosX += Delta;

					NodeB->NodePosX = FBAUtils::AlignTo8x8Grid(NodeB->NodePosX);

					RefreshParameters(NodeB);
					for (auto Child : FormatXInfoMap[NodeB]->GetChildren())
					{
						Child->NodePosX += Delta;

						Child->NodePosX = FBAUtils::AlignTo8x8Grid(Child->NodePosX);

						RefreshParameters(Child);
						// UE_LOG(LogTemp, Warning, TEXT("\tMove child %s"), *FBAUtils::GetNodeName(Child));
					}
				}
			}
		}
	}
}

void FEdGraphFormatter::StraightenRow(UEdGraphNode* Node)
{
	StraightenRowWithFilter(Node, [](const FPinLink& Link) { return true; });
}

void FEdGraphFormatter::StraightenRowWithFilter(UEdGraphNode* Node, TFunctionRef<bool(const FPinLink&)> Pred)
{
	TQueue<FPinLink> PendingLinks;
	for (const FPinLink& Link : FBAUtils::GetPinLinks(Node))
	{
		PendingLinks.Enqueue(Link);
	}

	TSet<FPinLink> StraightenedLinks;
	while (!PendingLinks.IsEmpty())
	{
		FPinLink Link;
		PendingLinks.Dequeue(Link);

		if (!Pred(Link))
		{
			continue;
		}

		if (StraightenedLinks.Contains(Link))
		{
			continue;
		}

		StraightenedLinks.Add(Link);
		StraightenedLinks.Add(Link.MakeOppositeLink());

		if (IsSameRow(Link))
		{
			FBAUtils::StraightenPin(GraphHandler, Link);
			RefreshParameters(Link.GetToNode());

			for (const FPinLink& NewLink : FBAUtils::GetPinLinks(Link.GetToNode()))
			{
				PendingLinks.Enqueue(NewLink);
			}
		}
	}
}

bool FEdGraphFormatter::IsSameRow(const FPinLink& PinLink)
{
	if (bool* FoundSameRow = SameRowMapping.Find(PinLink))
	{
		return *FoundSameRow;
	}

	return false;
}

bool FEdGraphFormatter::IsSameRow(UEdGraphNode* NodeA, UEdGraphNode* NodeB)
{
	TSet<FPinLink> VisitedLinks;
	TQueue<UEdGraphNode*> PendingNodes;
	PendingNodes.Enqueue(NodeA);

	while (!PendingNodes.IsEmpty())
	{
		UEdGraphNode* Node = nullptr;
		PendingNodes.Dequeue(Node);

		if (Node == NodeB)
		{
			return true;
		}

		for (const FPinLink& PinLink : FBAUtils::GetPinLinks(Node))
		{
			if (VisitedLinks.Contains(PinLink))
			{
				continue;
			}

			VisitedLinks.Add(PinLink);
			VisitedLinks.Add(PinLink.MakeOppositeLink());

			if (!IsSameRow(PinLink))
			{
				continue;
			}

			PendingNodes.Enqueue(PinLink.GetNode());
		}
	}

	return false;
}

TArray<UEdGraphNode*> FEdGraphFormatter::GetNodesInRow(UEdGraphNode* Node)
{
	TArray<UEdGraphNode*> NodesInRow;
	NodesInRow.Add(Node);
	TSet<FPinLink> VisitedLinks;
	TQueue<UEdGraphNode*> PendingNodes;
	PendingNodes.Enqueue(Node);
	while (!PendingNodes.IsEmpty())
	{
		UEdGraphNode* NextNode = nullptr;
		PendingNodes.Dequeue(NextNode);

		for (const FPinLink& PinLink : FBAUtils::GetPinLinks(NextNode))
		{
			if (VisitedLinks.Contains(PinLink))
			{
				continue;
			}

			VisitedLinks.Add(PinLink);
			VisitedLinks.Add(PinLink.MakeOppositeLink());

			if (!IsSameRow(PinLink))
			{
				continue;
			}

			NodesInRow.Add(PinLink.GetNode());
			PendingNodes.Enqueue(PinLink.GetNode());
		}
	}

	return NodesInRow;
}

bool FEdGraphFormatter::AreCommentsIntersecting(UEdGraphNode_Comment* CommentA, UEdGraphNode_Comment* CommentB)
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

	TArray<UEdGraphNode*> NodesA = FBAUtils::GetNodesUnderComment(CommentA);
	TArray<UEdGraphNode*> NodesB = FBAUtils::GetNodesUnderComment(CommentB);

	TArray<UEdGraphNode*> Intersection = NodesA.FilterByPredicate([&NodesB](UEdGraphNode* Node) { return NodesB.Contains(Node); });
	if (Intersection.Num() > 0)
	{
		return true;
	}

	return false;
}

TSharedPtr<FEdGraphParameterFormatter> FEdGraphFormatter::GetParameterParent(UEdGraphNode* Node)
{
	if (TSharedPtr<FEdGraphParameterFormatter> Formatter = ParameterParentMap.FindRef(Node))
	{
		return Formatter;
	}

	return nullptr;
}

TSharedPtr<FFormatXInfo> FEdGraphFormatter::GetFormatXInfo(UEdGraphNode* Node)
{
	return FormatXInfoMap.FindRef(Node);
}

TArray<UEdGraphNode*> FEdGraphFormatter::GetChildTree(TSharedPtr<FFormatXInfo> FormatXInfo)
{
	auto Filter = [&](TSharedPtr<FFormatXInfo> Info)
	{
		UEdGraphPin* FromPin = Info->Link.From;
		UEdGraphPin* ToPin = Info->Link.To;

		// UE_LOG(LogTemp, Warning, TEXT("GET CHILD TREE: Checking %s"), *Info->Link.ToString());
		if (!FromPin || !ToPin)
		{
			// UE_LOG(LogTemp, Warning, TEXT("\tOne is nullptr"));
			return false;
		}

		if (!SameRowMappingDirect.Contains(ToPin))
		{
			// UE_LOG(LogTemp, Warning, TEXT("\tSame row mapping does not contain"));
			return false;
		}

		// skip if we have a same row mapping but it is not the same as the child mapping from format x info  
		const bool bSkip = SameRowMappingDirect[FBAGraphPinHandle(ToPin)] != FBAGraphPinHandle(FromPin);
		// UE_LOG(LogTemp, Warning, TEXT("\tbSkip %d"), bSkip);
		return bSkip; 
	};

	return FormatXInfo->GetChildrenWithFilter(Filter);
	// return FormatXInfo->GetChildren();
}

TArray<UEdGraphNode*> FEdGraphFormatter::GetSameRowNodes(UEdGraphNode* Node)
{
	TArray<UEdGraphNode*> OutNodes;
	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (FBAGraphPinHandle* SameRowPin = SameRowMappingDirect.Find(Pin))
		{
			OutNodes.Add(SameRowPin->GetPin()->GetOwningNode());
		}
	}
	
	return OutNodes;
}

float FEdGraphFormatter::DecideNewParent(UEdGraphNode* Node, UEdGraphNode* NewParent)
{
	TSharedPtr<FFormatXInfo> NodeInfo = GetFormatXInfo(Node);
	TSharedPtr<FFormatXInfo> ParentNodeInfo = GetFormatXInfo(NewParent);

	if (!NodeInfo || !ParentNodeInfo)
	{
		return -1;
	}

	const bool bHasCycle = NodeInfo->GetChildren().Contains(NewParent);
	if (bHasCycle)
	{
		return -1;
	}

	int32 NewX = GetChildX(NewParent, Node, EGPD_Output, true);

	const int32 OldX = Node->NodePosX;
	const bool bPositionIsBetter = NewX > OldX;

	// UE_LOG(LogTemp, Warning, TEXT("Comparing parent %s %s %d %d"),
	// 	*FBAUtils::GetNodeName(Node), *FBAUtils::GetNodeName(NewParent),
	// 	NewX, OldX);

	if (bPositionIsBetter)
	{
		// UE_LOG(LogTemp, Warning, TEXT("\tTaking new parent %s %s"), *FBAUtils::GetNodeName(Node), *FBAUtils::GetNodeName(NewParent));
		NodeInfo->SetParent(ParentNodeInfo);
		return NewX - OldX;
	}

	return -1;
}

void FEdGraphFormatter::MoveChildrenX_Recursive(TSharedPtr<FFormatXInfo> Node, float DeltaX, TSet<TSharedPtr<FFormatXInfo>>& Visited)
{
	// TODO the only use of this function is for positive DeltaX
	if (!Node)
	{
		return;
	}

	if (Visited.Contains(Node))
	{
		return;
	}

	Visited.Add(Node);

	UEdGraphNode* CurrentNode = Node->GetNode();
	CurrentNode->NodePosX += DeltaX;

	// UE_LOG(LogTemp, Warning, TEXT("Moved %s %f"), *FBAUtils::GetNodeName(CurrentNode), DeltaX);

	if (!GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("Decide"))
	{
		TArray<UEdGraphPin*> OutExecPins = FBAUtils::GetExecPins(CurrentNode, EGPD_Output);
		TArray<UEdGraphNode*> LinkedOutNodes = FBAUtils::GetLinkedNodesFromPins(OutExecPins);
		for (UEdGraphNode* LinkedNode : LinkedOutNodes)
		{
			if (!GraphHandler->FilterSelectiveFormatting(LinkedNode, FormatterParameters.NodesToFormat))
			{
				continue;
			}

			if (!Node->GetChildren(EGPD_Output).Contains(LinkedNode))
			{
				const float NewDelta = DecideNewParent(LinkedNode, CurrentNode);
				MoveChildrenX_Recursive(GetFormatXInfo(LinkedNode), NewDelta, Visited);
			}
		}
	}

	for (TSharedPtr<FFormatXInfo> ChildInfo : Node->Children)
	{
		MoveChildrenX_Recursive(ChildInfo, DeltaX, Visited);
	}
}

void FEdGraphFormatter::SetNodeY_KeepingSpacingVisited(UEdGraphNode* Node, float NewPosY, TSet<UEdGraphNode*>& VisitedNodes)
{
	const float Delta = NewPosY - Node->NodePosY;

	TArray<UEdGraphNode*> PendingNodes;
	PendingNodes.Push(Node);

	while (PendingNodes.Num() > 0)
	{
		UEdGraphNode* Current = PendingNodes.Pop();

		if (VisitedNodes.Contains(Current))
		{
			continue;
		}

		VisitedNodes.Add(Current);

		// only move impure nodes and knot nodes (params will be moved with refresh params, comments will auto move) 
		if (FBAUtils::IsNodeImpure(Current) || FBAUtils::IsKnotNode(Current))
		{
			Current->NodePosY += Delta;

			Current->NodePosY = FBAUtils::AlignTo8x8Grid(Current->NodePosY);

			RefreshParameters(Current);

			// add all parameter nodes
			if (TSharedPtr<FEdGraphParameterFormatter> ParamFormatter = GetParameterFormatter(Current))
			{
				PendingNodes.Append(ParamFormatter->GetFormattedNodes().Array());
			}
		}

		if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Current))
		{
			for (auto NodeUnder : CommentHandler.GetNodesUnderComments(Comment))
			{
				PendingNodes.Add(NodeUnder);
			}
		}

		PendingNodes.Append(GetSameRowNodes(Current));

		if (FNodeRelativeLocation* RelativeInfo = NodeRelativeMapping.NodeRelativeYMap.Find(Current))
		{
			for (UEdGraphNode* Child : RelativeInfo->Children)
			{
				PendingNodes.Add(Child);
			}
		}

		TArray<UEdGraphNode*> NodesToMove = KnotTrackCreator.RelativeMapping.FindRef(Current);
		for (UEdGraphNode* NodeToMove : NodesToMove)
		{
			PendingNodes.Add(NodeToMove);
		}
	}
}

void FEdGraphFormatter::ResetRelativeToNodeToKeepStill(const FVector2D& SavedLocation)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ResetRelativeToNodeToKeepStill"), STAT_EdGraphFormatter_ResetRelativeToNodeToKeepStill, STATGROUP_BA_EdGraphFormatter);
	const float DeltaX = SavedLocation.X - NodeToKeepStill->NodePosX;
	const float DeltaY = SavedLocation.Y - NodeToKeepStill->NodePosY;

	if (DeltaX != 0 || DeltaY != 0)
	{
		TSet<UEdGraphNode*> AllNodes = GetFormattedGraphNodes();
		for (UEdGraphNode* Node : AllNodes)
		{
			Node->NodePosX += DeltaX;
			Node->NodePosY += DeltaY;
		}

		for (UEdGraphNode* Node : KnotTrackCreator.GetCreatedKnotNodes())
		{
			Node->NodePosX += DeltaX;
			Node->NodePosY += DeltaY;
		}
	}
}

int32 FEdGraphFormatter::GetChildX(const FPinLink& Link, const bool bUseClusterNodes)
{
	if (Link.From == nullptr)
	{
		return GetNodeBounds(Link.GetNode(), bUseClusterNodes).Left;
	}

	return GetChildX(Link.From->GetOwningNode(), Link.To->GetOwningNode(), Link.GetDirection(), bUseClusterNodes);
}

int32 FEdGraphFormatter::GetChildX(UEdGraphNode* Parent, UEdGraphNode* Child, EEdGraphPinDirection Direction, bool bUseClusterNodes)
{
	if (Parent == nullptr)
	{
		return 0;
	}

	if (Child == nullptr)
	{
		return GetNodeBounds(Parent, bUseClusterNodes).Left;
	}

	float NewNodePos;
	FSlateRect ParentBounds = bUseClusterNodes
		? GetClusterBounds(Parent)
		: FBAUtils::GetCachedNodeBounds(GraphHandler, Parent);

	FSlateRect ChildBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Child);

	FSlateRect LargerBounds = GetNodeBounds(Child, bUseClusterNodes);

	if (Direction == EGPD_Input)
	{
		const float Delta = LargerBounds.Right - ChildBounds.Left;
		NewNodePos = ParentBounds.Left - Delta - NodePadding.X; // -1;
	}
	else
	{
		const float Delta = ChildBounds.Left - LargerBounds.Left;
		NewNodePos = ParentBounds.Right + Delta + NodePadding.X; // +1;
	}

	NewNodePos = FBAUtils::AlignTo8x8Grid(NewNodePos);

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Node %s New Node Pos %d | %s | %s | %s"), *Link.ToString(), FMath::RoundToInt(NewNodePos), *ParentBounds.ToString(), *ChildBounds.ToString(), *LargerBounds.ToString());

	return FMath::RoundToInt(NewNodePos);
}

void FEdGraphFormatter::RemoveKnotNodes()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::RemoveKnotNodes"), STAT_EdGraphFormatter_RemoveKnotNodes, STATGROUP_BA_EdGraphFormatter);
	auto& GraphHandlerCapture = GraphHandler;
	auto& FormatterParamsCapture = FormatterParameters;
	const auto OnlySelected = [&GraphHandlerCapture, &FormatterParamsCapture](UEdGraphPin* Pin)
	{
		return GraphHandlerCapture->FilterSelectiveFormatting(Pin->GetOwningNode(), FormatterParamsCapture.NodesToFormat)
			&& (FBAUtils::IsParameterPin(Pin) || FBAUtils::IsExecOrDelegatePin(Pin));
	};

	KnotTrackCreator.RemoveKnotNodes(FBAUtils::GetNodeTreeWithFilter(RootNode, OnlySelected).Array());
}

void FEdGraphFormatter::GetPinsOfSameHeight()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::GetPinsOfSameHeight"), STAT_EdGraphFormatter_GetPinsOfSameHeight, STATGROUP_BA_EdGraphFormatter);
	TSet<UEdGraphNode*> NodesToCollisionCheck;
	TSet<FPinLink> VisitedLinks;
	TSet<UEdGraphNode*> TempChildren;
	GetPinsOfSameHeight_Recursive(RootNode, nullptr, nullptr, NodesToCollisionCheck, VisitedLinks);
}

void FEdGraphFormatter::FormatParameterNodes()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::FormatParameterNodes"), STAT_EdGraphFormatter_FormatParameterNodes, STATGROUP_BA_EdGraphFormatter);
	TArray<UEdGraphNode*> IgnoredNodes;

	TArray<UEdGraphNode*> NodePoolCopy = NodePool;

	const auto& LeftTopMostSort = [](const UEdGraphNode& NodeA, const UEdGraphNode& NodeB)
	{
		if (NodeA.NodePosX != NodeB.NodePosX)
		{
			return NodeA.NodePosX < NodeB.NodePosX;
		}

		return NodeA.NodePosY < NodeB.NodePosY;
	};
	NodePoolCopy.StableSort(LeftTopMostSort);

	ParameterParentMap.Reset();

	for (UEdGraphNode* MainNode : NodePoolCopy)
	{
		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Format parameters for node %s"), *FBAUtils::GetNodeName(MainNode));

		TSharedPtr<FEdGraphParameterFormatter> ParameterFormatter = GetParameterFormatter(MainNode);
		ParameterFormatter->SetIgnoredNodes(IgnoredNodes);
		ParameterFormatter->FormatNode(MainNode);

		// update node -> parameter formatter map
		for (UEdGraphNode* NodeToCheck : ParameterFormatter->GetFormattedNodes())
		{
			if (ParameterParentMap.Contains(NodeToCheck))
			{
				// if the node already has a parent, update the old parent by removing 
				TSharedPtr<FEdGraphParameterFormatter> ParentFormatter = ParameterParentMap[NodeToCheck];
				ParentFormatter->FormattedOutputNodes.Remove(NodeToCheck);
				ParentFormatter->AllFormattedNodes.Remove(NodeToCheck);
				ParentFormatter->IgnoredNodes.Add(NodeToCheck);

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Removed node %s from %s"), *FBAUtils::GetNodeName(NodeToCheck), *FBAUtils::GetNodeName(ParentFormatter->GetRootNode()));
			}

			// UE_LOG(LogBlueprintAssist, Warning, TEXT("Added node %s to %s"), *FBAUtils::GetNodeName(NodeToCheck), *FBAUtils::GetNodeName(ParameterFormatter->GetRootNode()));
			ParameterParentMap.Add(NodeToCheck, ParameterFormatter);
		}

		// the next main nodes will ignore the input nodes from the parameter formatter
		IgnoredNodes.Append(ParameterFormatter->FormattedInputNodes.Array());
	}

	// Format once again with proper ignored nodes
	for (UEdGraphNode* MainNode : NodePoolCopy)
	{
		TSharedPtr<FEdGraphParameterFormatter> ParameterFormatter = GetParameterFormatter(MainNode);
		ParameterFormatter->FormatNode(MainNode);
	}

	// Expand parameters by height
	if (GetDefault<UBASettings>()->bExpandParametersByHeight)
	{
		for (UEdGraphNode* MainNode : NodePoolCopy)
		{
			TSharedPtr<FEdGraphParameterFormatter> ParameterFormatter = GetParameterFormatter(MainNode);
			ParameterFormatter->ExpandByHeight();
		}
	}

	// Save relative position
	for (auto& Elem : ParameterFormatterMap)
	{
		// UE_LOG(LogBlueprintAssist, Warning, TEXT("MainParamForm: %s"), *FBAUtils::GetNodeName(Elem.Key));

		TSharedPtr<FEdGraphParameterFormatter> ParamFormatter = Elem.Value;
		ParamFormatter->SaveRelativePositions();
		ParamFormatter->bInitialized = true;

		// for (auto Child : ParamFormatter->GetFormattedNodes())
		// {
		// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\tNode %s"), *FBAUtils::GetNodeName(Child));
		// }
	}
}

TSet<UEdGraphNode*> FEdGraphFormatter::GetFormattedGraphNodes()
{
	TSet<UEdGraphNode*> OutNodes;
	for (UEdGraphNode* Node : NodePool)
	{
		OutNodes.Append(GetParameterFormatter(Node)->GetFormattedNodes());
	}

	return OutNodes;
}

void FEdGraphFormatter::RefreshParameters(UEdGraphNode* Node)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::RefreshParameters"), STAT_EdGraphFormatter_RefreshParameters, STATGROUP_BA_EdGraphFormatter);
	if (!Node || FBAUtils::IsNodePure(Node))
	{
		return;
	}

	if (TSharedPtr<FEdGraphParameterFormatter> Formatter = GetParameterFormatter(Node))
	{
		Formatter->FormatNode(Node);
	}
}

bool FEdGraphFormatter::IsFormattingRequired(const TArray<UEdGraphNode*>& NewNodeTree)
{
	if (!NewNodeTree.Contains(NodeToKeepStill))
	{
		return true;
	}

	// Check if a node has been deleted
	if (NodeTree.ContainsByPredicate(FBAUtils::IsNodeDeleted))
	{
		//UE_LOG(LogBlueprintAssist, Warning, TEXT("One of the nodes has been deleted"));
		return true;
	}

	// Check if the number of nodes has changed
	if (NodeTree.Num() != NewNodeTree.Num())
	{
		//UE_LOG(LogBlueprintAssist, Warning, TEXT("Num nodes changed %d to %d"), NewNodeTree.Num(), NodeTree.Num());
		return true;
	}

	// Check if the node tree has changed
	for (UEdGraphNode* Node : NewNodeTree)
	{
		if (!NodeTree.Contains(Node))
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Node tree changed for node %s"), *FBAUtils::GetNodeName(Node));
			return true;
		}
	}

	// Check if any formatted nodes from last time have changed position or links
	for (UEdGraphNode* Node : GetFormattedNodes())
	{
		// Check if node has changed
		if (NodeChangeInfos.Contains(Node))
		{
			FNodeChangeInfo ChangeInfo = NodeChangeInfos[Node];
			if (ChangeInfo.HasChanged(NodeToKeepStill))
			{
				//UE_LOG(LogBlueprintAssist, Warning, TEXT("Node links or position has changed"));
				return true;
			}
		}
	}

	const TSet<UEdGraphNode_Comment*>& CachedComments = CommentHandler.GetComments();

	// TODO Revisit this with new comment handler implementation
	// Check if any comment nodes have been deleted
	const auto& NodeTreeCapture = NodeTree;
	TArray<UEdGraphNode_Comment*> CommentNodes = FBAUtils::GetCommentNodesFromGraph(GraphHandler->GetFocusedEdGraph());
	for (auto Comment : CommentNodes)
	{
		TArray<UEdGraphNode*> Contained = FBAUtils::GetNodesUnderComment(Comment);
		UEdGraphNode** RelativeNodePtr = Contained.FindByPredicate([&NodeTreeCapture](UEdGraphNode* Node)
		{
			return NodeTreeCapture.Contains(Node);
		});

		if (RelativeNodePtr != nullptr)
		{
			if (!CachedComments.Contains(Comment))
			{
				return true;
			}
		}
	}

	// Check contained comment nodes
	for (UEdGraphNode_Comment* Comment : CachedComments)
	{
		if (!CommentNodes.Contains(Comment))
		{
			return true;
		}

		TArray<UEdGraphNode*> CachedContained = CommentHandler.GetNodesUnderComments(Comment);
		TArray<UEdGraphNode*> CurrentContained = FBAUtils::GetNodesUnderComment(Comment);
		if (CachedContained.Num() != CurrentContained.Num())
		{
			return true;
		}

		for (auto Contained : CachedContained)
		{
			if (!CurrentContained.Contains(Contained))
			{
				return true;
			}
		}
	}

	return false;
}

void FEdGraphFormatter::SaveFormattingEndInfo()
{
	// Save the position so we can move relative to this the next time we format
	LastFormattedX = NodeToKeepStill->NodePosX;
	LastFormattedY = NodeToKeepStill->NodePosY;

	// Save node information
	for (UEdGraphNode* Node : GetFormattedNodes())
	{
		if (NodeChangeInfos.Contains(Node))
		{
			NodeChangeInfos[Node].UpdateValues(NodeToKeepStill);
		}
		else
		{
			NodeChangeInfos.Add(Node, FNodeChangeInfo(Node, NodeToKeepStill));
		}
	}
}

TArray<UEdGraphNode*> FEdGraphFormatter::GetNodeTree(UEdGraphNode* InitialNode) const
{
	const auto& GraphHandlerCapture = GraphHandler;
	const auto& FormatterParametersCapture = FormatterParameters;
	const auto Filter = [&GraphHandlerCapture, &FormatterParametersCapture](const FPinLink& Link)
	{
		return GraphHandlerCapture->FilterDelegatePin(Link, FormatterParametersCapture.NodesToFormat);
	};
	return FBAUtils::GetNodeTreeWithFilter(InitialNode, Filter).Array();
}

bool FEdGraphFormatter::IsInitialNodeValid(UEdGraphNode* Node) const
{
	if (!Node)
	{
		return false;
	}
	if (Cast<UEdGraphNode_Comment>(Node))
	{
		return false;
	}
	if (Cast<UK2Node_Knot>(Node))
	{
		return false;
	}

	return true;
}

FSlateRect FEdGraphFormatter::GetClusterBounds(UEdGraphNode* Node)
{
	TSharedPtr<FEdGraphParameterFormatter> ParamFormatter = GetParameterFormatter(Node);
	if (!ParamFormatter)
	{
		return FBAUtils::GetCachedNodeArrayBoundsWithComments(GraphHandler, &CommentHandler, { Node });
	}

	const TArray<UEdGraphNode*> Nodes = ParamFormatter->GetFormattedNodes().Array();
	return FBAUtils::GetCachedNodeArrayBoundsWithComments(GraphHandler, ParamFormatter->GetCommentHandler(), Nodes);
}

UEdGraphNode* FEdGraphFormatter::GetClusterRootNode(UEdGraphNode* ChildNode)
{
	if (TSharedPtr<FEdGraphParameterFormatter> Parent = GetParameterParent(ChildNode))
	{
		return Parent->GetRootNode();
	}

	return nullptr;
}

FSlateRect FEdGraphFormatter::GetClusterBoundsForNodes(const TArray<UEdGraphNode*>& Nodes)
{
	TArray<UEdGraphNode*> NodesInColumn;

	TOptional<FSlateRect> OutBounds;
	for (UEdGraphNode* Node : Nodes)
	{
		if (Node)
		{
			const FSlateRect ClusterBounds = GetClusterBounds(Node);
			OutBounds = OutBounds.IsSet() ? OutBounds->Expand(ClusterBounds) : ClusterBounds;
		}
	}

	return OutBounds.Get(FSlateRect());
}

FSlateRect FEdGraphFormatter::GetNodeBounds(UEdGraphNode* Node, bool bUseClusterBounds)
{
	if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Node))
	{
		return CommentHandler.GetCommentBounds(Comment);
	}

	return bUseClusterBounds ? GetClusterBounds(Node) : FBAUtils::GetCachedNodeBounds(GraphHandler, Node);
}

FSlateRect FEdGraphFormatter::GetNodeArrayBounds(const TArray<UEdGraphNode*>& Nodes, bool bUseClusterBounds)
{
	return bUseClusterBounds ? GetClusterBoundsForNodes(Nodes) : FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes);
}

TSharedPtr<FEdGraphParameterFormatter> FEdGraphFormatter::GetParameterFormatter(UEdGraphNode* Node)
{
	if (FBAUtils::IsCommentNode(Node) || FBAUtils::IsKnotNode(Node))
	{
		return nullptr;
	}

	if (auto Parent = GetParameterParent(Node))
	{
		return Parent;
	}
	
	if (!ParameterFormatterMap.Contains(Node))
	{
		ParameterFormatterMap.Add(Node, MakeShared<FEdGraphParameterFormatter>(GraphHandler, Node, SharedThis(this)));
	}

	return ParameterFormatterMap[Node];
}

TSharedPtr<FFormatterInterface> FEdGraphFormatter::GetChildFormatter(UEdGraphNode* Node)
{
	return GetParameterParent(Node);
}

TArray<TSharedPtr<FFormatterInterface>> FEdGraphFormatter::GetChildFormatters()
{
	TArray<TSharedPtr<FFormatterInterface>> ChildFormatters;
	for (auto& Kvp : ParameterFormatterMap)
	{
		ChildFormatters.Add(Kvp.Value);
	}

	return ChildFormatters;
}

FBAFormatterSettings FEdGraphFormatter::GetFormatterSettings()
{
	if (FBAFormatterSettings* FormatterSettings = UBASettings::FindFormatterSettings(GraphHandler->GetFocusedEdGraph()))
	{
		return *FormatterSettings;
	}

	return GetMutableDefault<UBASettings>()->BlueprintFormatterSettings;
}

void FEdGraphFormatter::SetNodePos(UEdGraphNode* Node, const int X, const int Y)
{
	FFormatterInterface::SetNodePos(Node, X, Y);
	RefreshParameters(Node);
}

TSet<UEdGraphNode*> FEdGraphFormatter::GetRowAndChildren(UEdGraphNode* Node)
{
	TQueue<FPinLink> PendingLinks;
	PendingLinks.Enqueue(FPinLink(nullptr, nullptr, Node));

	TSet<UEdGraphNode*> NodesToMove;
	NodesToMove.Add(Node);

	// UE_LOG(LogTemp, Warning, TEXT("GETROWANDCHILDREN"));

	TSet<FPinLink> VisitedLinks;
	while (!PendingLinks.IsEmpty())
	{
		FPinLink Link;
		PendingLinks.Dequeue(Link);

		// UE_LOG(LogTemp, Error, TEXT("Checking link %s"), *Link.ToString());


		if (VisitedLinks.Contains(Link))
		{
			// UE_LOG(LogTemp, Warning, TEXT("\tSkip visited"));
			continue;
		}

		VisitedLinks.Add(Link);
		VisitedLinks.Add(Link.MakeOppositeLink());

		UEdGraphNode* LinkNode = Link.GetNode();
		NodesToMove.Add(LinkNode);

		// move all children
		if (auto LinkInfo = GetFormatXInfo(LinkNode))
		{
			for (auto Child : GetChildTree(LinkInfo))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tAdd child %s"), *FBAUtils::GetNodeName(Child));
				NodesToMove.Add(Child);
				// if (auto Param = GetParameterFormatter(Child))
				// {
				// 	NodesToMove.Append(Param->GetFormattedNodes());
				// }
			}
		}

		for (const FPinLink& NewLink : FBAUtils::GetPinLinks(LinkNode))
		{
			if (IsSameRow(NewLink))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tAdd same row %s"), *NewLink.ToString());
				PendingLinks.Enqueue(NewLink);
			}
		}
	}

	return NodesToMove;
}

bool FEdGraphFormatter::ShouldIgnoreComment(TSharedPtr<FBACommentContainsNode> ContainsNode)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::ShouldIgnoreComment"), STAT_EdGraphFormatter_ShouldIgnoreComment, STATGROUP_BA_EdGraphFormatter);
	// UE_LOG(LogTemp, Warning, TEXT("Checking should ignore Comment %s"), *FBAUtils::GetNodeName(Comment));

	TSet<UEdGraphNode*> FormattedNodes = GetFormattedNodes();
	const TArray<UEdGraphNode*>& AllNodesUnderComment = ContainsNode->AllContainedNodes;

	// ignore containing comments
	TSet<UEdGraphNode*> NodesUnderComment(AllNodesUnderComment);

	if (NodesUnderComment.Num() == 0)
	{
		// UE_LOG(LogTemp, Warning, TEXT("\tIGNORE: Skipping comment EMPTY"));
		return true;
	}

	// ignore if the comment contains a node which isn't going to be formatted 
	for (UEdGraphNode* Node : NodesUnderComment)
	{
		if (!FormattedNodes.Contains(Node))
		{
			// UE_LOG(LogTemp, Error, TEXT("\t IGNORE Contains missing"));
			return true;
		}

		// ignore if nodes underneath contains parameter but not its parent
		if (TSharedPtr<FEdGraphParameterFormatter> ParamFormatter = GetParameterParent(Node))
		{
			if (!NodesUnderComment.Contains(ParamFormatter->GetRootNode()))
			{
				// UE_LOG(LogTemp, Error, TEXT("\tIGNORE Param form"));
				return true;
			}
		}
	}

	// ignore if all nodes are not in the same node tree 
	const auto IsUnderComment = [&NodesUnderComment](const FPinLink& PinLink)
	{
		return NodesUnderComment.Contains(PinLink.GetNode());
	};

	const TSet<UEdGraphNode*> CommentNodeTree = FBAUtils::GetNodeTreeWithFilter(AllNodesUnderComment[0], IsUnderComment);
	if (CommentNodeTree.Num() != NodesUnderComment.Num())
	{
		// UE_LOG(LogTemp, Error, TEXT("IGNORE Same node tree"));
		return true;
	}

	for (UEdGraphNode* Node : NodesUnderComment)
	{
		if (!CommentNodeTree.Contains(Node))
		{
			// UE_LOG(LogTemp, Error, TEXT("IGNORE Comment node tree"));
			return true;
		}
	}

	// UE_LOG(LogTemp, Error, TEXT("\tDONT IGNOREComment node tree"));
	return false;
}

TSet<UEdGraphNode*> FEdGraphFormatter::GetFormattedNodes()
{
	if (MainParameterFormatter.IsValid())
	{
		return MainParameterFormatter->GetFormattedNodes();
	}

	TSet<UEdGraphNode*> OutNodes;
	for (UEdGraphNode* Node : NodePool)
	{
		OutNodes.Append(GetParameterFormatter(Node)->GetFormattedNodes());
	}

	OutNodes.Append(KnotTrackCreator.GetCreatedKnotNodes());

	return OutNodes;
}

void FEdGraphFormatter::FormatY()
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphFormatter::FormatY"), STAT_EdGraphFormatter_FormatY, STATGROUP_BA_EdGraphFormatter);

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("-------Format Y-------- NO COMMENTS"));

	TSet<UEdGraphNode*> NodesToCollisionCheck;
	TSet<FPinLink> VisitedLinks;
	TSet<UEdGraphNode*> TempChildren;
	FormatY_Recursive(FPinLink(nullptr, nullptr, RootNode), NodesToCollisionCheck, VisitedLinks, true, TempChildren);

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("-------Format Y-------- COMMENTS"));
}

void FEdGraphFormatter::CenterBranches(UEdGraphNode* CurrentNode, TArray<ChildBranch>& ChildBranches, TSet<UEdGraphNode*>& NodesToCollisionCheck)
{
	// Center branches
	TArray<UEdGraphPin*> ChildPins;
	TArray<UEdGraphPin*> ParentPins;
	for (ChildBranch& Branch : ChildBranches)
	{
		ChildPins.Add(Branch.Pin);
		ParentPins.Add(Branch.ParentPin);
	}

	const float ChildrenCenter = FBAUtils::GetCenterYOfPins(GraphHandler, ChildPins);
	const float ParentCenter = FBAUtils::GetCenterYOfPins(GraphHandler, ParentPins);
	const float Offset = ParentCenter - ChildrenCenter;

	TArray<UEdGraphNode*> AllNodes;

	for (ChildBranch& Branch : ChildBranches)
	{
		for (UEdGraphNode* Child : Branch.BranchNodes)
		{
			AllNodes.Add(Child);
			Child->NodePosY += Offset;
			RefreshParameters(Child);
		}
	}

	// Resolve collisions
	AllNodes.Add(CurrentNode);
	FSlateRect AllNodesBounds = GetClusterBoundsForNodes(AllNodes);
	const float InitialTop = AllNodesBounds.Top;
	for (auto Node : NodesToCollisionCheck)
	{
		if (AllNodes.Contains(Node))
		{
			continue;
		}

		FSlateRect Bounds = GetClusterBounds(Node);
		Bounds = Bounds.ExtendBy(FMargin(0, 0, 0, NodePadding.Y));
		if (FSlateRect::DoRectanglesIntersect(Bounds, AllNodesBounds))
		{
			const float OffsetY = Bounds.Bottom - AllNodesBounds.Top;
			AllNodesBounds = AllNodesBounds.OffsetBy(FVector2D(0, OffsetY));
		}
	}

	const float DeltaY = AllNodesBounds.Top - InitialTop;
	if (DeltaY != 0)
	{
		for (auto Node : AllNodes)
		{
			Node->NodePosY += DeltaY;
			RefreshParameters(Node);
		}
	}
}


bool FEdGraphFormatter::AnyCollisionBetweenPins(UEdGraphPin* Pin, UEdGraphPin* OtherPin)
{
	TSet<UEdGraphNode*> FormattedNodes = GetFormattedGraphNodes();

	const FVector2D PinPos = FBAUtils::GetPinPos(GraphHandler, Pin);
	const FVector2D OtherPinPos = FBAUtils::GetPinPos(GraphHandler, OtherPin);

	return NodeCollisionBetweenLocation(PinPos, OtherPinPos, { Pin->GetOwningNode(), OtherPin->GetOwningNode() });
}

bool FEdGraphFormatter::NodeCollisionBetweenLocation(FVector2D Start, FVector2D End, TSet<UEdGraphNode*> IgnoredNodes)
{
	TSet<UEdGraphNode*> FormattedNodes = GetFormattedGraphNodes();

	for (UEdGraphNode* NodeToCollisionCheck : FormattedNodes)
	{
		if (IgnoredNodes.Contains(NodeToCollisionCheck))
		{
			continue;
		}

		FSlateRect NodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToCollisionCheck).ExtendBy(FMargin(0, TrackSpacing - 1));
		if (FBAUtils::LineRectIntersection(NodeBounds, Start, End))
		{
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tNode collision!"));
			return true;
		}
	}

	return false;
}

