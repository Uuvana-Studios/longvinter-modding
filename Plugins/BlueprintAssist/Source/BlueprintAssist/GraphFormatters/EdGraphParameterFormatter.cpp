// Copyright 2021 fpwong. All Rights Reserved.

#include "EdGraphParameterFormatter.h"

#include "BAFormatterUtils.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistSettings.h"
#include "BlueprintAssistStats.h"
#include "BlueprintAssistUtils.h"
#include "EdGraphFormatter.h"
#include "EdGraphNode_Comment.h"
#include "GraphFormatterTypes.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistGraphOverlay.h"
#include "Containers/Queue.h"
#include "EdGraph/EdGraphNode.h"

FEdGraphParameterFormatter::FEdGraphParameterFormatter(
	TSharedPtr<FBAGraphHandler> InGraphHandler,
	UEdGraphNode* InRootNode,
	TSharedPtr<FEdGraphFormatter> InGraphFormatter,
	UEdGraphNode* InNodeToKeepStill,
	TArray<UEdGraphNode*> InIgnoredNodes)
	: GraphHandler(InGraphHandler)
	, RootNode(InRootNode)
	, GraphFormatter(InGraphFormatter)
	, IgnoredNodes(InIgnoredNodes)
	, NodeToKeepStill(InNodeToKeepStill)
{
	Padding = GetDefault<UBASettings>()->BlueprintParameterPadding;
	bFormatWithHelixing = false;

	if (!NodeToKeepStill)
	{
		NodeToKeepStill = InRootNode;
	}

	bCenterBranches = GetDefault<UBASettings>()->bCenterBranchesForParameters;
	NumRequiredBranches = GetDefault<UBASettings>()->NumRequiredBranchesForParameters;

	if (RootNode != nullptr)
	{
		AllFormattedNodes = { RootNode };
	}
}

void FEdGraphParameterFormatter::FormatNode(UEdGraphNode* InNode)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphParameterFormatter::FormatNode"), STAT_EdGraphParameterFormatter_FormatNode, STATGROUP_BA_EdGraphFormatter);
	if (!FBAUtils::IsGraphNode(RootNode))
	{
		return;
	}

	if (FBAUtils::GetLinkedPins(RootNode).Num() == 0)
	{
		AllFormattedNodes = { RootNode };
		return;
	}

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Formatting parameter node for root %s (%s)"), *FBAUtils::GetNodeName(RootNode), *FBAUtils::GetNodeName(InNode));

	const FSlateRect SavedBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToKeepStill);

	// we have formatted previously, just check how far the root node has moved
	// and move the nodes on the grid accordingly
	if (bInitialized)
	{
		SimpleRelativeFormatting();
		return;
	}

	GraphHandler->GetGraphOverlay()->ClearNodesInQueue();

	CommentHandler.Init(GraphHandler, AsShared());

	AllFormattedNodes = { RootNode };

	NodeOffsets.Reset();

	bFormatWithHelixing = DoesHelixingApply();

	TSet<UEdGraphNode*> SameRowVisited;
	SameRowMapping.Reset();
	SameRowMappingDirect.Reset();
	NodeRelativeMapping.Reset();

	FormatX();

	ProcessSameRowMapping(RootNode, nullptr, nullptr, SameRowVisited);

	if (AllFormattedNodes.Num() <= 1)
	{
		return;
	}

	if (GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("Param_FormatX"))
	{
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("Init param formatter comment handler!"));
	CommentHandler.BuildTree();

	// move the output nodes so they don't overlap with the helixed input nodes
	if (bFormatWithHelixing && FormattedInputNodes.Num() > 0)
	{
		const float InputNodesRight = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, FormattedInputNodes.Array()).Right;
		const FSlateRect RootNodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, RootNode);
		const float Delta = InputNodesRight - RootNodeBounds.Right;
		if (Delta > 0)
		{
			for (UEdGraphNode* Node : FormattedOutputNodes)
			{
				Node->NodePosX += Delta;
			}
		}
	}

	if (GetDefault<UBASettings>()->bApplyCommentPadding && !GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("ParamPadding_X"))
	{
		ApplyCommentPaddingX();
	}

	TSet<UEdGraphNode*> TempVisited;
	TSet<UEdGraphNode*> TempChildren;
	FormatY(FPinLink(nullptr, nullptr, RootNode), TempVisited, false, TempChildren);

	if (GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("Param_FormatY"))
	{
		return;
	}

	// for (auto Node : GetFormattedNodes())
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Same row node %s"), *FBAUtils::GetNodeName(Node));
	// 	for (UEdGraphNode* SameRowNode : GetSameRowNodes(Node))
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(SameRowNode));
	// 	}
	// }

	if (GetDefault<UBASettings>()->bApplyCommentPadding && !GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("ParamPadding_Y"))
	{
		ApplyCommentPaddingY();
	}

	if (GetDefault<UBASettings>()->bExpandParametersByHeight && FBAUtils::IsNodePure(RootNode))
	{
		ExpandByHeight();
	}

	if (FBAUtils::IsNodeImpure(RootNode))
	{
		if (UEdGraphPin* Pin = FBAUtils::GetFirstExecPin(RootNode, EGPD_Input))
		{
			float Baseline = FBAUtils::GetPinPos(GraphHandler, Pin).Y + GetDefault<UBASettings>()->ParameterVerticalPinSpacing;
			TSet<UEdGraphNode*> NodesToMove = GetFormattedNodes();
			NodesToMove.Remove(RootNode);
			MoveBelowBaseline(NodesToMove, Baseline);
		}
	}

	// move the nodes relative to the chosen node to keep still
	const FSlateRect NodeToKeepStillBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToKeepStill);

	const float DeltaX = SavedBounds.Left - NodeToKeepStillBounds.Left;
	const float DeltaY = SavedBounds.Top - NodeToKeepStillBounds.Top;

	if (DeltaX != 0 || DeltaY != 0)
	{
		for (UEdGraphNode* Node : GetFormattedNodes())
		{
			Node->NodePosX += DeltaX;
			Node->NodePosY += DeltaY;
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT("SAME ROW MAPPING"));
	// for (TTuple<FPinLink, bool> Kvp : SameRowMapping)
	// {
	// 	if (Kvp.Value)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Kvp.Key.ToString());
	// 	}
	// }

	// NodeRelativeMapping.PrintInfo();

	// for (TTuple<FPinLink, bool> RowMapping : SameRowMapping)
	// {
	// 	GraphHandler->GetGraphOverlay()->DrawLine(
	// 		FBAUtils::GetPinPos(GraphHandler, RowMapping.Key.From),
	// 		FBAUtils::GetPinPos(GraphHandler, RowMapping.Key.To),
	// 		FLinearColor::MakeRandomColor());
	// }
}

bool FEdGraphParameterFormatter::DoesHelixingApply()
{
	EBAParameterFormattingStyle FormattingStyleToUse = GetDefault<UBASettings>()->ParameterStyle;
	if (GraphFormatter->GetFormatterParameters().OverrideFormattingStyle.IsValid())
	{
		FormattingStyleToUse = *GraphFormatter->GetFormatterParameters().OverrideFormattingStyle.Get();
	}

	if (FormattingStyleToUse != EBAParameterFormattingStyle::Helixing)
	{
		return false;
	}

	TSet<UEdGraphNode*> VisitedNodes;
	VisitedNodes.Add(RootNode);
	TArray<UEdGraphNode*> NodeQueue;
	NodeQueue.Add(RootNode);

	const auto& IgnoredNodesCopy = IgnoredNodes;

	TSet<UEdGraphNode*> GatheredInputNodes;

	while (NodeQueue.Num() > 0)
	{
		UEdGraphNode* NextNode = NodeQueue.Pop();

		const auto Pred = [&IgnoredNodesCopy](UEdGraphPin* Pin)
		{
			return FBAUtils::IsParameterPin(Pin) && !IgnoredNodesCopy.Contains(Pin->GetOwningNode());
		};

		TArray<UEdGraphPin*> LinkedToPinsInput = FBAUtils::GetLinkedToPins(NextNode, EGPD_Input).FilterByPredicate(Pred);

		TArray<UEdGraphPin*> LinkedToPinsOutput = FBAUtils::GetLinkedToPins(NextNode, EGPD_Output).FilterByPredicate(Pred);

		TSet<UEdGraphNode*> LinkedToNodesInput;
		for (UEdGraphPin* Pin : LinkedToPinsInput)
		{
			if (FBAUtils::IsNodePure(Pin->GetOwningNode()))
			{
				LinkedToNodesInput.Add(Pin->GetOwningNode());
			}
		}

		TSet<UEdGraphNode*> LinkedToNodesOutput;
		for (UEdGraphPin* Pin : LinkedToPinsOutput)
		{
			auto OwningNode = Pin->GetOwningNode();
			if (OwningNode == RootNode || OwningNode->NodePosX <= RootNode->NodePosX)
			{
				LinkedToNodesOutput.Add(Pin->GetOwningNode());
			}
		}

		const bool bOutputLinkedToMultipleNodes = LinkedToNodesOutput.Num() > 1;

		const bool bInputLinkedToMultipleNodes = LinkedToNodesInput.Num() > 1;
		if (bInputLinkedToMultipleNodes || bOutputLinkedToMultipleNodes)
		{
			return false;
		}

		GatheredInputNodes.Append(LinkedToNodesInput);

		// add linked nodes input to the queue
		for (UEdGraphNode* Node : LinkedToNodesInput)
		{
			if (VisitedNodes.Contains(Node))
			{
				continue;
			}

			if (IgnoredNodes.Contains(Node))
			{
				continue;
			}

			VisitedNodes.Add(Node);
			NodeQueue.Add(Node);
		}
	}

	// check if any single node is extremely tall
	if (GetDefault<UBASettings>()->bLimitHelixingHeight)
	{
		float TotalHeight = 0;
		for (UEdGraphNode* Node : GatheredInputNodes)
		{
			if (Node != RootNode)
			{
				const float NodeHeight = FBAUtils::GetCachedNodeBounds(GraphHandler, Node).GetSize().Y;
				if (NodeHeight > GetDefault<UBASettings>()->SingleNodeMaxHeight)
				{
					return false;
				}

				TotalHeight += NodeHeight;
			}
		}

		// helixing should not apply if the height of the stack is too large
		if (TotalHeight > GetDefault<UBASettings>()->HelixingHeightMax)
		{
			return false;
		}
	}

	return true;
}

void FEdGraphParameterFormatter::ProcessSameRowMapping(UEdGraphNode* CurrentNode,
														UEdGraphPin* CurrentPin,
														UEdGraphPin* ParentPin,
														TSet<UEdGraphNode*>& VisitedNodes)
{
	if (VisitedNodes.Contains(CurrentNode))
	{
		return;
	}

	auto LinkedToSorter = [&](UEdGraphPin& PinA, UEdGraphPin& PinB)
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
			return PinPosA.X > PinPosB.X;
		}

		return PinPosA.Y < PinPosB.Y;
	};

	VisitedNodes.Add(CurrentNode);

	bool bFirstPin = true;

	for (EEdGraphPinDirection Direction : { EGPD_Input, EGPD_Output })
	{
		for (UEdGraphPin* MyPin : FBAUtils::GetPinsByDirection(CurrentNode, Direction))
		{
			if (FBAUtils::IsExecPin(MyPin))
			{
				continue;
			}

			TArray<UEdGraphPin*> LinkedTo = MyPin->LinkedTo;
			LinkedTo.StableSort(LinkedToSorter);

			for (UEdGraphPin* OtherPin : LinkedTo)
			{
				UEdGraphNode* OtherNode = OtherPin->GetOwningNode();

				if (!GraphHandler->FilterSelectiveFormatting(OtherNode, GraphFormatter->GetFormatterParameters().NodesToFormat))
				{
					continue;
				}

				if (FBAUtils::IsNodeImpure(OtherNode) && OtherNode != GetRootNode())
				{
					continue;
				}

				if (VisitedNodes.Contains(OtherNode))
				{
					continue;
				}

				if (!Path.Contains(FPinLink(MyPin, OtherPin)))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping path"));
					continue;
				}

				const bool bApplyHelixing = bFormatWithHelixing && FormattedInputNodes.Contains(OtherNode);

				const bool bSameDirectionAsParent = ParentPin == nullptr || MyPin->Direction == ParentPin->Direction;

				// UE_LOG(LogTemp, Warning, TEXT("Checking %s"), *Link.ToString());

				const bool bIsNotRootExecNode = CurrentNode != RootNode || FBAUtils::IsNodePure(RootNode);

				if (bFirstPin && bSameDirectionAsParent && !bApplyHelixing && bIsNotRootExecNode)
				{
					// UE_LOG(LogTemp, Warning, TEXT("Is same row?"));
					SameRowMapping.Add(FPinLink(MyPin, OtherPin), true);
					SameRowMapping.Add(FPinLink(OtherPin, MyPin), true);
					SameRowMappingDirect.Add(OtherPin, MyPin);
					SameRowMappingDirect.Add(MyPin, OtherPin);

					// FBAGraphOverlayLineParams Params;
					// Params.StartWidget = FBAUtils::GetGraphPin(GraphHandler->GetGraphPanel(), MyPin);
					// Params.EndWidget = FBAUtils::GetGraphPin(GraphHandler->GetGraphPanel(), OtherPin);
					// GraphHandler->GetGraphOverlay()->DrawLine(Params);

					// GraphHandler->GetGraphOverlay()->DrawLine(FBAUtils::GetPinPos(GraphHandler, MyPin), FBAUtils::GetPinPos(GraphHandler, OtherPin), 5.0f);
					bFirstPin = false;
				}

				TSet<UEdGraphNode*> LocalChildren;
				ProcessSameRowMapping(OtherNode, OtherPin, MyPin, VisitedNodes);
			}
		}
	}
}

void FEdGraphParameterFormatter::FormatX()
{
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("FORMATTING X for %s"), *FBAUtils::GetNodeName(RootNode));

	TArray<TEnumAsByte<EEdGraphPinDirection>> InOut = { EGPD_Output, EGPD_Input };

	TSet<FPinLink> VisitedLinks;

	TArray<UEdGraphNode*> TempOutput;

	NodeInfoMap.Reset();

	for (EEdGraphPinDirection InitialDirection : InOut)
	{
		VisitedLinks.Empty();

		TQueue<FPinLink> OutputQueue;
		TQueue<FPinLink> InputQueue;

		FPinLink RootNodeLink = FPinLink(nullptr, nullptr, RootNode);
		if (InitialDirection == EGPD_Input)
		{
			InputQueue.Enqueue(RootNodeLink);
		}
		else
		{
			OutputQueue.Enqueue(RootNodeLink);
		}

		while (!InputQueue.IsEmpty() || !OutputQueue.IsEmpty())
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Input Queue:"));
			//for (PinLink* Link : InputQueue)
			//{
			//	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *Link->ToString());
			//}

			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Output Queue:"));
			//for (PinLink* Link : OutputStack)
			//{
			//	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *Link->ToString());
			//}

			// TODO: This is stupid, fix this!
			FPinLink CurrentPinLink;
			if (InitialDirection == EGPD_Input)
			{
				(!InputQueue.IsEmpty() ? InputQueue : OutputQueue).Dequeue(CurrentPinLink);
			}
			else
			{
				(!OutputQueue.IsEmpty() ? OutputQueue : InputQueue).Dequeue(CurrentPinLink);
			}

			UEdGraphNode* CurrentNode = CurrentPinLink.GetNode();
			UEdGraphPin* ParentPin = CurrentPinLink.From;
			UEdGraphPin* MyPin = CurrentPinLink.To;

			UEdGraphNode* ParentNode = ParentPin != nullptr ? ParentPin->GetOwningNode() : nullptr;

			TSharedPtr<FNodeInfo> ParentInfo;
			if (ParentPin != nullptr)
			{
				ParentInfo = NodeInfoMap[ParentPin->GetOwningNode()];
			}

			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tPopped Node: %s | MyPin %s | ParentNode %s | ParentPin %s")
			// 	, *FBAUtils::GetNodeName(CurrentNode)
			// 	, *FBAUtils::GetPinName(MyPin)
			// 	, *FBAUtils::GetNodeName(ParentNode)
			// 	, *FBAUtils::GetPinName(ParentPin));

			if (NodeInfoMap.Contains(CurrentNode) && CurrentNode != RootNode &&
				TempOutput.Contains(CurrentNode) &&
				InitialDirection == EGPD_Input
					//&& !FormattedOutputNodes.Contains(CurrentNode)
			)
			{
				NodeInfoMap.Remove(CurrentNode);
				TempOutput.Remove(CurrentNode);
			}

			if (NodeInfoMap.Contains(CurrentNode))
			{
				if (ParentPin != nullptr && MyPin != nullptr)
				{
					TSharedPtr<FNodeInfo> CurrentInfo = NodeInfoMap[CurrentNode];

					if (ParentNode != CurrentInfo->GetParentNode())
					{
						int32 NewNodePos = GetChildX(CurrentPinLink, ParentPin->Direction);

						const bool bApplyHelixing = bFormatWithHelixing && FormattedInputNodes.Contains(CurrentNode);
						if (bApplyHelixing)
						{
							NewNodePos = CurrentNode->NodePosX;
						}

						const bool bNewLocationIsBetter = (ParentPin->Direction == EGPD_Input
							? NewNodePos < CurrentNode->NodePosX
							: NewNodePos > CurrentNode->NodePosX) || bApplyHelixing;

						const bool bSameDirection = CurrentInfo->Direction == ParentPin->Direction;

						const bool bTakeNewParent = bNewLocationIsBetter && bSameDirection;

						if (bTakeNewParent)
						{
							// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tParameter Node %s was already formatted, taking new parent %s %d -> %d (Old Parent %s)"),
							// 	*FBAUtils::GetNodeName(CurrentNode),
							// 	*FBAUtils::GetNodeName(ParentNode),
							// 	CurrentNode->NodePosX,
							// 	NewNodePos,
							// 	*FBAUtils::GetNodeName(CurrentInfo->GetParentNode()));

							CurrentNode->NodePosX = NewNodePos;
							CurrentInfo->SetParent(ParentInfo, MyPin);
							TSet<UEdGraphNode*> TempChildren;
							CurrentInfo->MoveChildren(CurrentInfo, GraphHandler, Padding, TempChildren);
						}
						else
						{
							// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tNOT TAKING NEW PARENT ( WORSE ) %s"), *FBAUtils::GetNodeName(CurrentNode));
						}
					}
				}
			}
			else
			{
				CurrentNode->Modify();

				if (CurrentNode != RootNode)
				{
					FBAUtils::StraightenPin(GraphHandler, CurrentPinLink);

					if (InitialDirection == EGPD_Input && CurrentPinLink.GetDirection() == EGPD_Input && (FormattedInputNodes.Contains(ParentNode) || ParentNode == RootNode))
					{
						FormattedInputNodes.Add(CurrentNode);
					}
					else if (InitialDirection == EGPD_Output && CurrentPinLink.GetDirection() == EGPD_Output && (FormattedOutputNodes.Contains(ParentNode) || ParentNode == RootNode))
					{
						FormattedOutputNodes.Add(CurrentNode);
					}

					int32 NewNodePos = GetChildX(CurrentPinLink, ParentPin->Direction);

					const bool bApplyHelixing = bFormatWithHelixing && FormattedInputNodes.Contains(CurrentNode);
					if (bApplyHelixing)
					{
						NewNodePos = ParentNode->NodePosX;
					}

					CurrentNode->NodePosX = NewNodePos;

					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tNode %s | Pos %d | InitialDir %d CurrentDir %d"),
					// 	*FBAUtils::GetNodeName(CurrentNode),
					// 	NewNodePos,
					// 	static_cast<int32>(InitialDirection),
					// 	static_cast<int32>(CurrentPinLink.GetDirection()));

					AllFormattedNodes.Add(CurrentNode);
				}
				else // add the root node
				{
					AllFormattedNodes.Add(RootNode);
				}

				TSharedPtr<FNodeInfo> NewNodeInfo = MakeShareable(new FNodeInfo(CurrentNode, MyPin, ParentInfo, ParentPin, InitialDirection));
				NewNodeInfo->SetParent(ParentInfo, MyPin);
				NodeInfoMap.Add(CurrentNode, NewNodeInfo);

				Path.Add(NewNodeInfo->Link);

				if (InitialDirection == EGPD_Output)
				{
					TempOutput.Add(CurrentNode);
				}

				//if (ParentNode != nullptr)
				//{
				//	UE_LOG(LogBlueprintAssist, Warning, TEXT("SET New Parent for %s to %s | %d | %d"), *FBAUtils::GetNodeName(CurrentNode), *FBAUtils::GetNodeName(ParentNode), CurrentNode->NodePosX, ParentNode->NodePosX);
				//}
			}

			TSharedPtr<FNodeInfo> CurrentInfo = NodeInfoMap[CurrentNode];

			// if the current node is the root node, use the initial direction when getting linked nodes
			const bool bCurrentNodeIsRootAndImpure = CurrentNode == RootNode && FBAUtils::IsNodeImpure(CurrentNode);
			const EEdGraphPinDirection LinkedDirection = bCurrentNodeIsRootAndImpure ? InitialDirection : EGPD_MAX;

			const auto AllowedPins = [](UEdGraphPin* Pin)
			{
				return FBAUtils::IsParameterPin(Pin) || FBAUtils::IsDelegatePin(Pin);
			};

			TArray<UEdGraphPin*> LinkedParameterPins = FBAUtils::GetLinkedPins(CurrentNode, LinkedDirection).FilterByPredicate(AllowedPins);

			for (int i = 0; i < LinkedParameterPins.Num(); i++)
			{
				UEdGraphPin* Pin = LinkedParameterPins[i];

				TArray<UEdGraphPin*> LinkedTo = Pin->LinkedTo;
				for (UEdGraphPin* LinkedPin : LinkedTo)
				{
					UEdGraphNode* LinkedNode = LinkedPin->GetOwningNodeUnchecked();

					if (!GraphHandler->FilterSelectiveFormatting(LinkedNode, GraphFormatter->GetFormatterParameters().NodesToFormat))
					{
						continue;
					}

					if (FBAUtils::IsNodeImpure(LinkedNode) || FBAUtils::IsKnotNode(LinkedNode))
					{
						continue;
					}

					FPinLink CurrentLink = FPinLink(Pin, LinkedPin);
					if (VisitedLinks.Contains(CurrentLink))
					{
						continue;
					}
					VisitedLinks.Add(CurrentLink);

					if (NodeInfoMap.Contains(LinkedNode))
					{
						TSharedPtr<FNodeInfo> LinkedInfo = NodeInfoMap[LinkedNode];
						if (CurrentInfo->DetectCycle(LinkedInfo))
						{
							continue;
						}
					}

					if (LinkedNode == RootNode && FBAUtils::IsNodeImpure(RootNode))
					{
						//UE_LOG(LogBlueprintAssist, Warning, TEXT("SKIPPED Node %s (RootNode)"), *BlueprintAssistUtils::GetNodeName(LinkedNode));
						continue;
					}

					if (IgnoredNodes.Contains(LinkedNode))
					{
						continue;
					}

					if (Pin->Direction == EGPD_Input)
					{
						InputQueue.Enqueue(CurrentLink);
					}
					else
					{
						OutputQueue.Enqueue(CurrentLink);
					}
				}
			}
		}
	}
}

void FEdGraphParameterFormatter::FormatY(
	const FPinLink& CurrentLink,
	TSet<UEdGraphNode*>& VisitedNodes,
	const bool bSameRow,
	TSet<UEdGraphNode*>& OutChildren)
{
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("ParameterFormatter: FormatY %s %d"), *CurrentLink.ToString(), bSameRow);

	UEdGraphNode* CurrentNode = CurrentLink.GetNode();

	if (VisitedNodes.Contains(CurrentNode))
	{
		return;
	}

	if (UEdGraphNode* ParentNode = CurrentLink.GetFromNodeUnsafe())
	{
		NodeRelativeMapping.UpdateRelativeY(CurrentNode, ParentNode);
	}

	const TArray<UEdGraphNode*> Children = NodeInfoMap[CurrentNode]->GetChildNodes();

	// solve collisions against visited nodes
	for (int i = 0; i < 100; ++i)
	{
		bool bNoCollision = true;

		for (UEdGraphNode* NodeToCollisionCheck : VisitedNodes)
		{
			if (NodeToCollisionCheck == CurrentNode)
			{
				continue;
			}

			FSlateRect MyBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, CurrentNode);

			FMargin ExtendPadding(0, 0, 0, Padding.Y);

			FSlateRect OtherBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, NodeToCollisionCheck);

			OtherBounds = OtherBounds.ExtendBy(ExtendPadding);

			if (FSlateRect::DoRectanglesIntersect(MyBounds.ExtendBy(ExtendPadding), OtherBounds))
			{
				// UE_LOG(LogBlueprintAssist, Warning, TEXT("%s: Collision with %s | %s | %s"),
				//        *FBAUtils::GetNodeName(CurrentNode), *FBAUtils::GetNodeName(NodeToCollisionCheck),
				//        *MyBounds.ToString(), *OtherBounds.ToString()
				// );

				const float Delta = OtherBounds.Bottom - MyBounds.Top;
				CurrentNode->NodePosY += Delta + 1;

				bNoCollision = false;
				NodeRelativeMapping.UpdateRelativeY(CurrentNode, NodeToCollisionCheck);
				break;
			}
		}

		if (bNoCollision)
		{
			break;
		}
	}

	VisitedNodes.Add(CurrentNode);

	for (EEdGraphPinDirection Direction : { EGPD_Input, EGPD_Output })
	{
		UEdGraphPin* LastLinked = nullptr;

		TArray<ChildBranch> ChildBranches;
		for (const FPinLink& Link : FBAUtils::GetPinLinks(CurrentNode, Direction))
		{
			if (FBAUtils::IsExecPin(Link.From))
			{
				LastLinked = Link.From;
				continue;
			}

			UEdGraphNode* ToNode = Link.To->GetOwningNode();

			if (IgnoredNodes.Contains(ToNode) ||
				!GraphHandler->FilterSelectiveFormatting(ToNode, GraphFormatter->GetFormatterParameters().NodesToFormat) ||
				!Children.Contains(ToNode) ||
				// !Path.Contains(Link) ||
				VisitedNodes.Contains(ToNode) ||
				FBAUtils::IsNodeImpure(ToNode))
			{
				continue;
			}

			// if we applied helixing, then do not format any input nodes
			const bool bApplyHelixing = bFormatWithHelixing && FormattedInputNodes.Contains(ToNode);
			if (bApplyHelixing)
			{
				ToNode->NodePosY = FBAUtils::GetCachedNodeBounds(GraphHandler, Link.GetFromNodeUnsafe()).Bottom + Padding.Y;
			}
			else
			{
				FBAUtils::StraightenPin(GraphHandler, Link.From, Link.To);
			}

			bool bChildIsSameRow = !bApplyHelixing && IsSameRow(Link);

			TSet<UEdGraphNode*> LocalChildren;
			FormatY(Link, VisitedNodes, bChildIsSameRow, LocalChildren);
			OutChildren.Append(LocalChildren);

			ChildBranches.Add(ChildBranch(Link.To, Link.From, LocalChildren));

			// avoid last linked pin
			if (!(bFormatWithHelixing && Direction == EGPD_Input) && LocalChildren.Num() > 0 && LastLinked != nullptr)
			{
				const FSlateRect Bounds = GraphFormatter->GetNodeArrayBounds(LocalChildren.Array(), false);

				//UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tPin to avoid %s (%s)"), *FBAUtils::GetPinName(PinToAvoid), *FBAUtils::GetPinName(OtherPin));
				const float PinPos = GraphHandler->GetPinY(LastLinked) + GetDefault<UBASettings>()->ParameterVerticalPinSpacing;
				const float Delta = PinPos - Bounds.Top;
				if (Delta > 0)
				{
					for (UEdGraphNode* Child : LocalChildren)
					{
						// UE_LOG(LogBlueprintAssist, Warning, TEXT("Moved child %s by %f (Pin %f (%s) | %f)"), *FBAUtils::GetNodeName(Child), Delta, PinPos, *FBAUtils::GetPinName(PinToAvoid), Bounds.Top);
						Child->NodePosY += Delta;
					}
				}
			}

			LastLinked = Link.From;
		}

		// check if there are any child nodes to the right of us
		TArray<UEdGraphNode*> AllChildren;
		for (auto Branch : ChildBranches)
		{
			AllChildren.Append(Branch.BranchNodes.Array());
		}
		FSlateRect ChildBounds = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, AllChildren);
		const bool bChildrenTooBig = CurrentNode->NodePosX < ChildBounds.Right;

		if (bCenterBranches && Direction == EGPD_Input && ChildBranches.Num() >= NumRequiredBranches && !bChildrenTooBig)
		{
			if (FBAUtils::IsNodePure(CurrentNode))
			{
				CenterBranches(CurrentNode, ChildBranches, VisitedNodes);
			}
		}
	}

	OutChildren.Add(CurrentNode);

	if (bSameRow && CurrentLink.From && IsSameRow(CurrentLink))
	{
		FBAUtils::StraightenPin(GraphHandler, CurrentLink.To, CurrentLink.From);
	}
}

int32 FEdGraphParameterFormatter::GetChildX(const FPinLink& Link, const EEdGraphPinDirection Direction) const
{
	float NewNodePos;

	UEdGraphNode* Parent = Link.From->GetOwningNode();
	UEdGraphNode* Child = Link.To->GetOwningNode();

	FSlateRect ParentBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Parent);

	const FSlateRect ChildBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Child);

	if (Direction == EGPD_Input)
	{
		NewNodePos = ParentBounds.Left - ChildBounds.GetSize().X - FMath::Max(1.0f, Padding.X);
	}
	else
	{
		NewNodePos = ParentBounds.Right + FMath::Max(1.0f, Padding.X);
	}

	return FMath::RoundToInt(NewNodePos);
}

bool FEdGraphParameterFormatter::AnyLinkedImpureNodes() const
{
	TSet<UEdGraphNode*> VisitedNodes;
	TArray<UEdGraphNode*> NodeQueue;

	VisitedNodes.Add(RootNode);
	NodeQueue.Push(RootNode);

	while (NodeQueue.Num() > 0)
	{
		UEdGraphNode* Node = NodeQueue.Pop();
		if (FBAUtils::IsNodeImpure(Node))
		{
			return true;
		}

		for (UEdGraphNode* LinkedNode : FBAUtils::GetLinkedNodes(Node))
		{
			if (VisitedNodes.Contains(LinkedNode))
			{
				continue;
			}

			VisitedNodes.Add(LinkedNode);
			NodeQueue.Push(LinkedNode);
		}
	}

	return false;
}

FSlateRect FEdGraphParameterFormatter::GetBounds()
{
	return FBAUtils::GetCachedNodeArrayBounds(GraphHandler, GetFormattedNodes().Array());
}

FSlateRect FEdGraphParameterFormatter::GetParameterBounds()
{
	TArray<UEdGraphNode*> WithoutRootNode = GetFormattedNodes().Array();
	WithoutRootNode = WithoutRootNode.FilterByPredicate([this](UEdGraphNode* Node) { return Node != RootNode; });

	if (WithoutRootNode.Num() == 0)
	{
		return FSlateRect(0, 0, 0, 0);
	}

	FSlateRect ParameterBounds = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, WithoutRootNode);

	const FSlateRect RootNodeBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, RootNode);

	const float TopPadding = FMath::Max(0.f, ParameterBounds.Top - RootNodeBounds.Bottom);
	ParameterBounds = ParameterBounds.ExtendBy(FMargin(0, TopPadding, 0, 0));

	return ParameterBounds;
}

void FEdGraphParameterFormatter::MoveBelowBaseline(TSet<UEdGraphNode*> Nodes, const float Baseline)
{
	if (GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("Baseline"))
	{
		return;
	}

	const FSlateRect Bounds
		= GetDefault<UBASettings>()->bApplyCommentPadding
		? CommentHandler.GetNodeArrayBoundsWithComments(Nodes.Array(), RootNode, { RootNode })
		: FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes.Array());

	if (Baseline > Bounds.Top)
	{
		const float Delta = Baseline - Bounds.Top;
		for (UEdGraphNode* Node : Nodes)
		{
			Node->NodePosY += Delta;
		}
	}
}

TSet<UEdGraphNode*> FEdGraphParameterFormatter::GetFormattedNodes()
{
	return AllFormattedNodes;
}

void FEdGraphParameterFormatter::DebugPrintFormatted()
{
	UE_LOG(LogBlueprintAssist, Warning, TEXT("Node Info Map: "));
	for (auto& Elem : NodeInfoMap)
	{
		UEdGraphNode* Node = Elem.Key;
		TSharedPtr<FNodeInfo> Info = Elem.Value;

		UE_LOG(LogBlueprintAssist, Warning, TEXT("\tNode %s | Parent %s"), *FBAUtils::GetNodeName(Node), *FBAUtils::GetNodeName(Info->GetParentNode()));

		for (UEdGraphNode* Child : Info->GetChildNodes())
		{
			UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tChild %s"), *FBAUtils::GetNodeName(Child));
		}
	}

	UE_LOG(LogBlueprintAssist, Warning, TEXT("Formatted Input %s"), *FBAUtils::GetNodeName(RootNode));
	for (UEdGraphNode* Node : FormattedInputNodes)
	{
		UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	}

	UE_LOG(LogBlueprintAssist, Warning, TEXT("Formatted Output %s"), *FBAUtils::GetNodeName(RootNode));
	for (UEdGraphNode* Node : FormattedOutputNodes)
	{
		UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	}

	UE_LOG(LogBlueprintAssist, Warning, TEXT("Formatted ALL for %s"), *FBAUtils::GetNodeName(RootNode));
	for (UEdGraphNode* Node : GetFormattedNodes())
	{
		UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	}
}

void FEdGraphParameterFormatter::SimpleRelativeFormatting()
{
	for (auto Elem : NodeOffsets)
	{
		UEdGraphNode* Node = Elem.Key;
		FVector2D Offset = Elem.Value;

		Node->NodePosX = NodeToKeepStill->NodePosX + Offset.X;
		Node->NodePosY = NodeToKeepStill->NodePosY + Offset.Y;
	}
}

FSlateRect FEdGraphParameterFormatter::GetNodeBounds(UEdGraphNode* Node)
{
	if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Node))
	{
		return CommentHandler.GetCommentBounds(Comment);
	}

	return FBAUtils::GetCachedNodeBounds(GraphHandler, Node);
}

void FEdGraphParameterFormatter::ExpandByHeight()
{
	if (bFormatWithHelixing)
	{
		return;
	}

	TArray<EEdGraphPinDirection> InputOutput = { EGPD_Input }; // , EGPD_Output

	auto FormattedNodes = GetFormattedNodes();
	auto FormattedOutputNodesCapture = FormattedOutputNodes;

	const auto IsFormatted = [FormattedNodes, FormattedOutputNodesCapture](UEdGraphPin* Pin)
	{
		return FormattedNodes.Contains(Pin->GetOwningNode()) || FormattedOutputNodesCapture.Contains(Pin->GetOwningNode());
	};

	TSet<UEdGraphNode*> NodesToExpand = FormattedInputNodes;
	NodesToExpand.Add(RootNode);

	for (UEdGraphNode* FormattedNode : NodesToExpand)
	{
		for (EEdGraphPinDirection Direction : InputOutput)
		{
			const TArray<FPinLink> PinLinks = FBAUtils::GetPinLinks(FormattedNode, Direction);

			float LargestPinDelta = 0;
			for (const FPinLink& Link : PinLinks)
			{
				const float PinDelta = FBAUtils::GetPinPos(GraphHandler, Link.To).Y - FBAUtils::GetPinPos(GraphHandler, Link.From).Y;
				LargestPinDelta = FMath::Max(PinDelta, LargestPinDelta);
			}

			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Checking node %s %f %d"), *FBAUtils::GetNodeName(FormattedNode), LargestPinDelta, Direction);

			if (LargestPinDelta < 150)
			{
				continue;
			}

			const int32 ExpandDirection = Direction == EGPD_Input ? -1 : 1;
			const float ExpandX = ExpandDirection * LargestPinDelta * 0.2f;

			const auto NodeTree = FBAUtils::GetNodeTreeWithFilter(FormattedNode, IsFormatted, Direction, true);
			for (UEdGraphNode* Node : NodeTree)
			{
				if (Node != FormattedNode && Node != RootNode)
				{
					Node->NodePosX += ExpandX;

					//UE_LOG(LogBlueprintAssist, Warning, TEXT("\tExpanded node %s %d"), *FBAUtils::GetNodeName(Node), Direction);
				}
			}
		}
	}
}

void FEdGraphParameterFormatter::SaveRelativePositions()
{
	for (UEdGraphNode* Node : GetFormattedNodes())
	{
		if (Node != NodeToKeepStill)
		{
			const FVector2D RelativePos = FVector2D(Node->NodePosX - NodeToKeepStill->NodePosX, Node->NodePosY - NodeToKeepStill->NodePosY);

			// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tRelative pos %s to %s : %s"), *FBAUtils::GetNodeName(Node), *FBAUtils::GetNodeName(NodeToKeepStill), *RelativePos.ToString());

			NodeOffsets.Add(Node, RelativePos);
		}
	}
}

void FEdGraphParameterFormatter::SetNodeY_KeepingSpacingVisited(UEdGraphNode* Node, float NewPosY, TSet<UEdGraphNode*>& VisitedNodes)
{
	const float Delta = NewPosY - Node->NodePosY;

	TArray<UEdGraphNode*> PendingNodes;
	PendingNodes.Push(Node);

	while (PendingNodes.Num() > 0)
	{
		UEdGraphNode* Current = PendingNodes.Pop();

		if (VisitedNodes.Contains(Current))
		{
			// UE_LOG(LogTemp, Warning, TEXT("\t\tSkipping visited"));
			continue;
		}

		VisitedNodes.Add(Current);

		// UE_LOG(LogTemp, Warning, TEXT("SETNODEY %s"), *FBAUtils::GetNodeName(Current));

		Current->NodePosY += Delta;

		auto SameRowNodes = GetSameRowNodes(Current);
		PendingNodes.Append(SameRowNodes);

		// FBAUtils::PrintNodeArray(SameRowNodes, "Same row nodes");

		if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Current))
		{
			for (auto NodeUnder : CommentHandler.GetNodesUnderComments(Comment))
			{
				PendingNodes.Add(NodeUnder);
				// UE_LOG(LogTemp, Warning, TEXT("Node under comment %s"), *FBAUtils::GetNodeName(NodeUnder));
			}
		}

		if (FNodeRelativeLocation* RelativeInfo = NodeRelativeMapping.NodeRelativeYMap.Find(Current))
		{
			for (UEdGraphNode* Child : RelativeInfo->Children)
			{
				PendingNodes.Add(Child);
				// UE_LOG(LogTemp, Warning, TEXT("Relative %s"), *FBAUtils::GetNodeName(Child));
			}
		}
	}
}

TSet<UEdGraphNode*> FEdGraphParameterFormatter::GetRowAndChildren(UEdGraphNode* Node)
{
	TQueue<FPinLink> PendingLinks;
	PendingLinks.Enqueue(FPinLink(nullptr, nullptr, Node));

	TSet<UEdGraphNode*> RowAndChildren;
	RowAndChildren.Add(Node);

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
		RowAndChildren.Add(LinkNode);

		// move all children
		if (TSharedPtr<FNodeInfo> LinkInfo = NodeInfoMap.FindRef(LinkNode))
		{
			for (auto Child : LinkInfo->GetAllChildNodes())
			{
				RowAndChildren.Add(Child);
			}
		}

		for (const FPinLink& NewLink : FBAUtils::GetPinLinks(LinkNode))
		{
			// UE_LOG(LogTemp, Warning, TEXT("Link %s"), *NewLink.ToString());
			if (IsSameRow(NewLink))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tSAME ROW"));
				PendingLinks.Enqueue(NewLink);
			}
			else
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tNOT SAME ROW"));
			}
		}
	}

	return RowAndChildren;
}

TArray<UEdGraphNode*> FEdGraphParameterFormatter::GetSameRowNodes(UEdGraphNode* Node)
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

bool FEdGraphParameterFormatter::ShouldIgnoreComment(TSharedPtr<FBACommentContainsNode> ContainsNode)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("FEdGraphParameterFormatter::ShouldIgnoreComment"), STAT_EdGraphParameterFormatter_ShouldIgnoreComment, STATGROUP_BA_EdGraphFormatter);
	// UE_LOG(LogTemp, Warning, TEXT("Checking Comment %s"), *FBAUtils::GetNodeName(Comment));

	TSet<UEdGraphNode*> FormattedNodes = GetFormattedNodes();
	const TArray<UEdGraphNode*>& AllNodesUnderComment = ContainsNode->AllContainedNodes;

	// ignore containing comments
	TSet<UEdGraphNode*> NodesUnderComment(AllNodesUnderComment);

	if (NodesUnderComment.Num() == 0)
	{
		// UE_LOG(LogTemp, Warning, TEXT("\tPARAM Skipping comment EMPTY %s"), *FBAUtils::GetNodeName(Comment));
		return true;
	}

	// ignore if the comment contains a node which isn't going to be formatted 
	for (UEdGraphNode* Node : NodesUnderComment)
	{
		if (!FormattedNodes.Contains(Node))
		{
			// UE_LOG(LogTemp, Warning, TEXT("\tPARAM Skipping has non-formatted %s"), *FBAUtils::GetNodeName(Comment));
			return true;
		}
	}

	// if the comment contains all of our nodes, then it will be handled by the EdGraphFormatter
	if (NodesUnderComment.Includes(FormattedNodes))
	{
		return true;
	}

	// ignore if all nodes are not in the same node tree 
	const auto IsUnderComment = [&NodesUnderComment](const FPinLink& PinLink)
	{
		return NodesUnderComment.Contains(PinLink.GetNode());
	};

	const TSet<UEdGraphNode*> CommentNodeTree = FBAUtils::GetNodeTreeWithFilter(AllNodesUnderComment[0], IsUnderComment);
	if (CommentNodeTree.Num() != NodesUnderComment.Num())
	{
		// UE_LOG(LogTemp, Warning, TEXT("\tPARAM Skipping not in node tree %s"), *FBAUtils::GetNodeName(Comment));
		return true;
	}

	for (UEdGraphNode* Node : NodesUnderComment)
	{
		if (!CommentNodeTree.Contains(Node))
		{
			// UE_LOG(LogTemp, Warning, TEXT("\tPARAM Skipping not in node tree 2 %s"), *FBAUtils::GetNodeName(Comment));
			return true;
		}
	}

	return false;
}

void FEdGraphParameterFormatter::CenterBranches(UEdGraphNode* CurrentNode, const TArray<ChildBranch>& ChildBranches, const TSet<UEdGraphNode*>& NodesToCollisionCheck)
{
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Centering Node %s"), *FBAUtils::GetNodeName(CurrentNode));
	// for (auto& Branch : ChildBranches)
	// {
	// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\tBranch %s"), *Branch.ToString());
	// 	for (auto Node : Branch.BranchNodes)
	// 	{
	// 		UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t%s"), *FBAUtils::GetNodeName(Node));
	// 	}
	// }

	// Center branches
	TArray<UEdGraphPin*> ChildPins;
	TArray<UEdGraphPin*> ParentPins;
	for (const ChildBranch& Branch : ChildBranches)
	{
		ChildPins.Add(Branch.Pin);
		ParentPins.Add(Branch.ParentPin);
	}

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Child pins:"));
	// for (auto Pin : ChildPins)
	// {
	// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *FBAUtils::GetPinName(Pin));
	// }
	//
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Parent pins:"));
	// for (auto Pin : ParentPins)
	// {
	// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t%s"), *FBAUtils::GetPinName(Pin));
	// }

	const float ChildrenCenter = FBAUtils::GetCenterYOfPins(GraphHandler, ChildPins);
	const float ParentCenter = FBAUtils::GetCenterYOfPins(GraphHandler, ParentPins);
	const float Offset = ParentCenter - ChildrenCenter;

	TArray<UEdGraphNode*> AllNodes;

	for (const ChildBranch& Branch : ChildBranches)
	{
		for (UEdGraphNode* Child : Branch.BranchNodes)
		{
			AllNodes.Add(Child);
			Child->NodePosY += Offset;
		}
	}

	// Resolve collisions
	AllNodes.Add(CurrentNode);

	FSlateRect AllNodesBounds = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, AllNodes);
	const float InitialTop = AllNodesBounds.Top;
	for (auto Node : NodesToCollisionCheck)
	{
		if (AllNodes.Contains(Node))
		{
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("Skipping %s"), *FBAUtils::GetNodeName(Node));
			continue;
		}

		FSlateRect Bounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Node).ExtendBy(FMargin(0, Padding.Y));
		if (FSlateRect::DoRectanglesIntersect(Bounds, AllNodesBounds))
		{
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("Colliding with %s"), *FBAUtils::GetNodeName(Node));
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
		}
	}
}

void FEdGraphParameterFormatter::ApplyCommentPaddingX()
{
	TArray<UEdGraphNode*> Contains = GetFormattedNodes().Array();

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : CommentHandler.GetRootNodes())
	{
		for (UEdGraphNode* Node : ContainsNode->AllContainedNodes)
		{
			Contains.RemoveSwap(Node);
		}
	}

	TArray<FPinLink> OutLeafLinks;
	ApplyCommentPaddingX_Recursive(Contains, CommentHandler.GetRootNodes().Array(), OutLeafLinks);
}

void FEdGraphParameterFormatter::ApplyCommentPaddingX_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes, TArray<FPinLink>& OutLeafLinks)
{
	NodeSet.RemoveAll([&AllFormattedNodes = AllFormattedNodes](UEdGraphNode* Node)
	{
		return !AllFormattedNodes.Contains(Node);
	});

	for (TSharedPtr<FBACommentContainsNode> Contains : ContainsNodes)
	{
		NodeSet.Add(Contains->Comment);
	}

	const auto RightMost = [&](UEdGraphNode& NodeA, UEdGraphNode& NodeB)
	{
		const float RightA = GetNodeBounds(&NodeA).Right;
		const float RightB = GetNodeBounds(&NodeB).Right;
		return RightA > RightB;
	};

	NodeSet.StableSort(RightMost);

	TArray<FPinLink> LeafLinks;
	for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
	{
		ApplyCommentPaddingX_Recursive(ContainsNode->OwnedNodes, ContainsNode->Children, LeafLinks);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Format SubGraph"));
	// for (UEdGraphNode* Node : NodeSet)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT("Leaf links:"));
	// for (FPinLink& LeafLink : LeafLinks)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *LeafLink.ToString());
	// }

	for (UEdGraphNode* NodeA : NodeSet)
	{
		// collide only with our children
		TSet<TSharedPtr<FNodeInfo>> Children;
		if (UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA))
		{
			for (UEdGraphNode* Node : CommentHandler.GetContainsNode(CommentA)->AllContainedNodes)
			{
				if (auto FormatXInfo = NodeInfoMap.FindRef(Node))
				{
					Children.Append(FormatXInfo->Children);
				}
			}
		}
		else
		{
			if (auto FormatXInfo = NodeInfoMap.FindRef(NodeA))
			{
				Children.Append(FormatXInfo->Children);
			}
		}

		// UE_LOG(LogTemp, Warning, TEXT("Children for node %s"), *FBAUtils::GetNodeName(NodeA));
		// for (TSharedPtr<FNodeInfo> Info : Children)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Info->GetNode()));
		// }

		// gather leaf links
		TArray<FPinLink> LinksInNodeSet;
		TArray<FPinLink> PotentialLeafLinks;
		for (TSharedPtr<FNodeInfo> Info : Children)
		{
			const bool bFromRoot = Info->Link.GetFromNodeUnsafe() == GetRootNode() && FBAUtils::IsNodeImpure(GetRootNode()) && !IsUsingHelixing();
			if (!IsSameRow(Info->Link) && !bFromRoot)
			{
				continue;
			}

			if (NodeSet.Contains(Info->Link.GetNode()))
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
			}
		}

		for (TSharedPtr<FNodeInfo> Info : Children)
		{
			UEdGraphNode* NodeB = Info->Node;

			// UE_LOG(LogTemp, Warning, TEXT("\tCHECKING {%s} > {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

			const bool bIsImpureRootNode = NodeA == RootNode && FBAUtils::IsNodeImpure(RootNode);
			if (!SameRowMapping.Contains(Info->Link) && !bIsImpureRootNode && !LeafLinks.Contains(Info->Link))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tNOt same row skipping"));
				continue;
			}

			if (!NodeSet.Contains(NodeB))
			{
				bool bHasContainingComment = false;
				for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
				{
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
				if (FCommentHandler::AreCommentsIntersecting(CommentA, CommentB))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkip comments intersecting"));
					continue;
				}
			}

			FSlateRect BoundsA = GetNodeBounds(NodeA).ExtendBy(FMargin(Padding.X, 0.f));
			FSlateRect BoundsB = GetNodeBounds(NodeB);

			if (CommentA)
			{
				BoundsA = CommentHandler.GetCommentBounds(CommentA).ExtendBy(FMargin(Padding.X, 0.f));
			}

			if (CommentB)
			{
				BoundsB = CommentHandler.GetCommentBounds(CommentB);
			}

			// UE_LOG(LogTemp, Error, TEXT("Checking Link {%s}"), *Info->Link.ToString());

			if (FSlateRect::DoRectanglesIntersect(BoundsA, BoundsB))
			{
				const float Delta = Info->Link.GetDirection() == EGPD_Output ?
					BoundsA.Right + 1.0f - BoundsB.Left :
					BoundsA.Left - BoundsB.Right;

				if (CommentB)
				{
					TSet<UEdGraphNode*> AllChildren;
					for (UEdGraphNode* Node : CommentHandler.GetContainsNode(CommentB)->AllContainedNodes)
					{
						if (!NodeInfoMap.Contains(Node))
						{
							continue;
						}

						AllChildren.Add(Node);

						for (TSharedPtr<FNodeInfo> Child : NodeInfoMap[Node]->Children)
						{
							AllChildren.Add(Child->Node);
						}
						
						AllChildren.Append(NodeInfoMap[Node]->GetAllChildNodes());
					}

					// UE_LOG(LogTemp, Warning, TEXT("\tNode {%s} Colliding with COMMENT {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

					// GraphHandler->GetGraphOverlay()->DrawBounds(BoundsA);
					// GraphHandler->GetGraphOverlay()->DrawBounds(BoundsB, FLinearColor::Blue);

					for (auto Child : AllChildren)
					{
						Child->NodePosX += Delta;
						// UE_LOG(LogTemp, Warning, TEXT("\tMove child %s"), *FBAUtils::GetNodeName(Child));
					}

					// UE_LOG(LogTemp, Warning, TEXT("CommentBounds %s"), *BoundsB.ToString());
					// UE_LOG(LogTemp, Warning, TEXT("RegularBounds %s"), *FBAUtils::GetCachedNodeArrayBounds(GraphHandler, CommentHandler.CommentNodesContains[CommentB]).ToString());
				}
				else
				{
					if (!NodeInfoMap.Contains(NodeB))
					{
						continue;
					}

					// UE_LOG(LogTemp, Warning, TEXT("COMMENT {%s} Colliding with Node {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

					NodeB->NodePosX += Delta;
					for (auto Child : NodeInfoMap[NodeB]->GetAllChildNodes())
					{
						Child->NodePosX += Delta;
						// UE_LOG(LogTemp, Warning, TEXT("\tMove child %s"), *FBAUtils::GetNodeName(Child));
					}
				}
			}
		}
	}
}

void FEdGraphParameterFormatter::ApplyCommentPaddingY()
{
	TArray<UEdGraphNode*> Contains = GetFormattedNodes().Array();

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : CommentHandler.GetRootNodes())
	{
		for (UEdGraphNode* Node : ContainsNode->AllContainedNodes)
		{
			Contains.RemoveSwap(Node);
		}
	}

	ApplyCommentPaddingY_Recursive(Contains, CommentHandler.GetRootNodes().Array());
}

void FEdGraphParameterFormatter::ApplyCommentPaddingY_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes)
{
	NodeSet.RemoveAll([&AllFormattedNodes = AllFormattedNodes](UEdGraphNode* Node)
	{
		return !AllFormattedNodes.Contains(Node);
	});

	for (TSharedPtr<FBACommentContainsNode> Contains : ContainsNodes)
	{
		NodeSet.Add(Contains->Comment);
	}

	NodeSet.StableSort([&](UEdGraphNode& NodeA, UEdGraphNode& NodeB)
	{
		float TopA = GetNodeBounds(&NodeA).Top;
		if (auto Comment = Cast<UEdGraphNode_Comment>(&NodeA))
		{
			auto Nodes = FBAUtils::GetNodesUnderComment(Comment);
			Nodes.RemoveAll(FBAUtils::IsCommentNode);
			TopA = FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes).Top;
		}

		float TopB = GetNodeBounds(&NodeB).Top;
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

	for (UEdGraphNode* NodeA : NodeSet)
	{
		for (UEdGraphNode* NodeB : NodeSet)
		{
			if (NodeA == NodeB)
			{
				continue;
			}

			UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA);
			UEdGraphNode_Comment* CommentB = Cast<UEdGraphNode_Comment>(NodeB);

			if (CommentA && CommentB)
			{
				if (FCommentHandler::AreCommentsIntersecting(CommentA, CommentB))
				{
					continue;
				}
			}

			FSlateRect BoundsA = GetNodeBounds(NodeA).ExtendBy(FMargin(0, Padding.Y));
			FSlateRect BoundsB = GetNodeBounds(NodeB);

			// UE_LOG(LogTemp, Warning, TEXT("{%s} Checking {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

			if (FSlateRect::DoRectanglesIntersect(BoundsA, BoundsB))
			{
				const float Delta = BoundsA.Bottom + 1.0f - BoundsB.Top;

				TSet<UEdGraphNode*> Visited;

				// Don't move NodeA
				if (CommentA)
				{
					Visited.Append(CommentHandler.GetNodesUnderComments(CommentA));
				}
				else
				{
					Visited.Add(NodeA);
				}

				if (CommentB)
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tColliding Comment"));

					for (UEdGraphNode* Node : CommentHandler.GetNodesUnderComments(CommentB))
					{
						SetNodeY_KeepingSpacingVisited(Node, Node->NodePosY + Delta, Visited);
					}
				}
				else
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tColliding Node"));
					SetNodeY_KeepingSpacingVisited(NodeB, NodeB->NodePosY + Delta, Visited);
				}

				// for (UEdGraphNode* Visit : Visited)
				// {
				// 	UE_LOG(LogTemp, Warning, TEXT("\tMoved %s"), *FBAUtils::GetNodeName(Visit));
				// }
			}
		}
	}
}

const FEdGraphFormatterParameters& FEdGraphParameterFormatter::GetFormatterParameters()
{
	return GraphFormatter->GetFormatterParameters();
}