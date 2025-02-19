// Copyright 2021 fpwong. All Rights Reserved.

#include "SimpleFormatter.h"

#include "BAFormatterUtils.h"
#include "BlueprintAssistUtils.h"
#include "EdGraphNode_Comment.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistGraphOverlay.h"
#include "Containers/Queue.h"

FSimpleFormatter::FSimpleFormatter(TSharedPtr<FBAGraphHandler> InGraphHandler, const FEdGraphFormatterParameters& InFormatterParameters)
	: GraphHandler(InGraphHandler)
	, FormatterParameters(InFormatterParameters)
	, RootNode(nullptr)
{
	TrackSpacing = GetMutableDefault<UBASettings>()->BlueprintKnotTrackSpacing;
}

void FSimpleFormatter::ProcessSameRowMapping(const FPinLink& Link, TSet<UEdGraphNode*>& VisitedNodes)
{
	UEdGraphNode* CurrentNode = Link.GetNode();
	if (VisitedNodes.Contains(CurrentNode))
	{
		return;
	}

	VisitedNodes.Add(CurrentNode);

	// UE_LOG(LogTemp, Warning, TEXT("Process same row %s"), *Link.ToString());

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


	bool bFirstPin = true;

	FormatterSettings.FormatterDirection;

	for (EEdGraphPinDirection Direction : { EGPD_Input, EGPD_Output })
	{
		for (UEdGraphPin* MyPin : FBAUtils::GetPinsByDirection(CurrentNode, Direction))
		{
			// if (!FBAUtils::IsExecPin(MyPin))
			// {
			// 	continue;
			// }

			TArray<UEdGraphPin*> LinkedTo = MyPin->LinkedTo;
			LinkedTo.StableSort(LinkedToSorter);

			for (UEdGraphPin* OtherPin : LinkedTo)
			{
				// UE_LOG(LogTemp, Warning, TEXT("Checking linked %s %s"), *FBAUtils::GetPinName(MyPin), *FBAUtils::GetPinName(OtherPin));
				UEdGraphNode* OtherNode = OtherPin->GetOwningNode();

				if (!GraphHandler->FilterSelectiveFormatting(OtherNode, GetFormatterParameters().NodesToFormat))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping selective formatting"));
					continue;
				}

				//if (FBAUtils::IsNodeImpure(OtherNode) && OtherNode != GetRootNode())
				// if (OtherNode != GetRootNode())
				// {
				// 	UE_LOG(LogTemp, Warning, TEXT("\tSkipping root"));
				// 	continue;
				// }

				if (VisitedNodes.Contains(OtherNode))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping visited"));
					continue;
				}

				if (!Path.Contains(FPinLink(MyPin, OtherPin)))
				{
					// UE_LOG(LogTemp, Warning, TEXT("\tSkipping path"));
					continue;
				}

				// const bool bApplyHelixing = bFormatWithHelixing && FormattedInputNodes.Contains(OtherNode);

				const bool bSameDirectionAsParent = Link.From == nullptr || MyPin->Direction == Link.From->Direction;

				// UE_LOG(LogTemp, Warning, TEXT("Checking %s"), *Link.ToString());

				if (bFirstPin && bSameDirectionAsParent)
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
				ProcessSameRowMapping(FPinLink(MyPin, OtherPin), VisitedNodes);
			}
		}
	}
}

void FSimpleFormatter::FormatNode(UEdGraphNode* Node)
{
	RootNode = Node;

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Root node %s"), *FBAUtils::GetNodeName(RootNode));

	FormatterSettings = GetFormatterSettings();

	CommentHandler.Init(GraphHandler, AsShared());

	int32 SavedNodePosX = RootNode->NodePosX;
	int32 SavedNodePosY = RootNode->NodePosY;

	FormatX();

	TSet<UEdGraphNode*> SameRowVisited;
	ProcessSameRowMapping(FPinLink(nullptr, nullptr, RootNode), SameRowVisited);

	CommentHandler.BuildTree();

	if (GetDefault<UBASettings>()->bApplyCommentPadding && CommentHandler.IsValid() &&
		!GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("SimplePaddingX"))
	{
		ApplyCommentPaddingX();
	}

	FormatY();

	// UE_LOG(LogTemp, Warning, TEXT("Same row mapping"));
	// for (auto Kvp : SameRowMapping)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *Kvp.Key.ToString());
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT("Format x children"));
	// for (auto Kvp : FormatXInfoMap)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Kvp.Key));
	// 	for (auto FormatXInfo : Kvp.Value->Children)
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("\t\t%s"), *FBAUtils::GetNodeName(FormatXInfo->GetNode()));
	// 	}
	// }

	if (GetDefault<UBASettings>()->bApplyCommentPadding && CommentHandler.IsValid() &&
		!GetDefault<UBASettings>()->BlueprintAssistDebug.Contains("SimplePaddingY"))
	{
		ApplyCommentPaddingY();
	}

	// reset root node position
	const float DeltaX = SavedNodePosX - RootNode->NodePosX;
	const float DeltaY = SavedNodePosY - RootNode->NodePosY;

	for (UEdGraphNode* FormattedNode : FormattedNodes)
	{
		FormattedNode->NodePosX += DeltaX;
		FormattedNode->NodePosY += DeltaY;

		FVector2D NewPos(FormattedNode->NodePosX, FormattedNode->NodePosY);

		TSharedPtr<SGraphNode> GraphNode = FBAUtils::GetGraphNode(GraphHandler->GetGraphPanel(), FormattedNode);
		TSet<TWeakPtr<SNodePanel::SNode>> NodeSet;
		GraphNode->MoveTo(NewPos, NodeSet);
	}
}

void FSimpleFormatter::FormatX()
{
	TSet<UEdGraphNode*> VisitedNodes;
	TSet<UEdGraphNode*> PendingNodes;
	PendingNodes.Add(RootNode);
	TSet<FPinLink> VisitedLinks;
	const FPinLink RootNodeLink(nullptr, nullptr, RootNode);
	TSharedPtr<FFormatXInfo> RootInfo = MakeShareable(new FFormatXInfo(RootNodeLink, nullptr));

	TArray<TSharedPtr<FFormatXInfo>> OutputStack;
	TArray<TSharedPtr<FFormatXInfo>> InputStack;
	OutputStack.Push(RootInfo);
	FormatXInfoMap.Add(RootNode, RootInfo);

	EEdGraphPinDirection LastDirection = FormatterSettings.FormatterDirection;

	NodesToExpand.Reset();

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

		FormattedNodes.Add(CurrentNode);

		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Processing %s | %s"), *FBAUtils::GetNodeName(CurrentNode), *CurrentInfo->Link.ToString());
		const int32 NewX = GetChildX(CurrentInfo->Link);

		if (!FormatXInfoMap.Contains(CurrentNode))
		{
			if (CurrentNode != RootNode)
			{
				CurrentInfo->SetParent(CurrentInfo->Parent);
				CurrentNode->NodePosX = NewX;

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

		TArray<UEdGraphPin*> LinkedPins = FBAUtils::GetLinkedPins(CurrentInfo->GetNode());
		// for (auto Pin : LinkedPins)
		// {
		// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("Pin %s"), *FBAUtils::GetPinName(Pin));
		// }

		for (int i = LinkedPins.Num() - 1; i >= 0; --i)
		{
			UEdGraphPin* ParentPin = LinkedPins[i];

			for (UEdGraphPin* LinkedPin : ParentPin->LinkedTo)
			{
				UEdGraphNode* LinkedNode = LinkedPin->GetOwningNode();

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Iterating node %s"), *FBAUtils::GetNodeName(LinkedNode));

				const FPinLink PinLink(ParentPin, LinkedPin, LinkedNode);
				if (VisitedLinks.Contains(PinLink))
				{
					continue;
				}

				VisitedLinks.Add(PinLink);

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tIterating pin link %s"), *PinLink.ToString());

				TSharedPtr<FFormatXInfo> LinkedInfo = MakeShareable(new FFormatXInfo(PinLink, CurrentInfo));

				if (ParentPin->Direction == FormatterSettings.FormatterDirection)
				{
					OutputStack.Push(LinkedInfo);
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\t\t\tAdded to output stack"));
				}
				else
				{
					if (GetMutableDefault<UBASettings>()->FormattingStyle == EBANodeFormattingStyle::Expanded)
					{
						EEdGraphPinDirection OppositeDirection = UEdGraphPin::GetComplementaryDirection(FormatterSettings.FormatterDirection);

						if (CurrentInfo->Link.GetDirection() == FormatterSettings.FormatterDirection)
						{
							const bool bHasCycle = PendingNodes.Contains(LinkedNode) || FBAUtils::GetExecTree(LinkedNode, OppositeDirection).Contains(CurrentInfo->GetNode());

							if (!bHasCycle)
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
		ExpandPendingNodes();
	}
}

int32 FSimpleFormatter::GetChildX(const FPinLink& Link)
{
	if (Link.From == nullptr)
	{
		return GetNodeBounds(Link.GetNode()).Left;
	}

	EEdGraphPinDirection Direction = Link.GetDirection();
	UEdGraphNode* Parent = Link.From->GetOwningNode();
	UEdGraphNode* Child = Link.To->GetOwningNode();
	FSlateRect ParentBounds = GetNodeBounds(Parent);

	FSlateRect ChildBounds = GetNodeBounds(Child);

	FSlateRect LargerBounds = GetNodeBounds(Child);

	float NewNodePos;
	if (Link.From->Direction == EGPD_Input)
	{
		const float Delta = LargerBounds.Right - ChildBounds.Left;
		NewNodePos = ParentBounds.Left - Delta - FormatterSettings.Padding.X; // -1;
	}
	else
	{
		const float Delta = ChildBounds.Left - LargerBounds.Left;
		NewNodePos = ParentBounds.Right + Delta + FormatterSettings.Padding.X; // +1;
	}

	return FMath::RoundToInt(NewNodePos);
	// return ParentBounds.Left - Padding.X - ChildBounds.GetSize().X;
}

void FSimpleFormatter::SetNodeY_KeepingSpacingVisited(UEdGraphNode* Node, float NewPosY, TSet<UEdGraphNode*>& VisitedNodes)
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
		// if (FBAUtils::IsNodeImpure(Current) || FBAUtils::IsKnotNode(Current))
		{
			Current->NodePosY += Delta;
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
	}
}

TArray<UEdGraphNode*> FSimpleFormatter::GetSameRowNodes(UEdGraphNode* Node)
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

bool FSimpleFormatter::IsSameRow(const FPinLink& Link)
{
	return SameRowMapping.Contains(Link);
}

FSlateRect FSimpleFormatter::GetNodeBounds(UEdGraphNode* Node)
{
	if (UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(Node))
	{
		return CommentHandler.GetCommentBounds(Comment);
	}

	return FBAUtils::GetCachedNodeBounds(GraphHandler, Node);
}

FSlateRect FSimpleFormatter::GetNodeArrayBounds(const TArray<UEdGraphNode*>& Nodes)
{
	return FBAUtils::GetCachedNodeArrayBounds(GraphHandler, Nodes);
}

FBAFormatterSettings FSimpleFormatter::GetFormatterSettings()
{
	return UBASettings::GetFormatterSettings(GraphHandler->GetFocusedEdGraph());
}

void FSimpleFormatter::ExpandPendingNodes()
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

		auto OppositeDirection = UEdGraphPin::GetComplementaryDirection(FormatterSettings.FormatterDirection);
		TArray<UEdGraphNode*> Children = Info->GetChildren(OppositeDirection);

		// UE_LOG(LogBlueprintAssist, Warning, TEXT("Expand X %s | %s"), *FBAUtils::GetNodeName(Info->GetNode()), *FBAUtils::GetNodeName(Parent));

		if (Children.Num() > 0)
		{
			FSlateRect ChildrenBounds = GetNodeArrayBounds(Children);

			FSlateRect ParentBounds = GetNodeBounds(Parent);

			bool bShouldExpand = FormatterSettings.FormatterDirection == EGPD_Output
				? ParentBounds.Right > ChildrenBounds.Left
				: ChildrenBounds.Right > ParentBounds.Left;

			if (bShouldExpand)
			{
				const float Delta = FormatterSettings.FormatterDirection == EGPD_Output
					? ParentBounds.Right - ChildrenBounds.Left + FormatterSettings.Padding.X
					: ParentBounds.Left - ChildrenBounds.Right - FormatterSettings.Padding.X;

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Expanding node %s by %f"), *FBAUtils::GetNodeName(Node), Delta);

				Node->NodePosX += Delta;

				TArray<UEdGraphNode*> AllChildren = Info->GetChildren();
				for (UEdGraphNode* Child : AllChildren)
				{
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tChild %s"), *FBAUtils::GetNodeName(Child));
					Child->NodePosX += Delta;
				}
			}
		}
	}
}

void FSimpleFormatter::FormatY()
{
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Format y?!?!?"));

	TSet<UEdGraphNode*> NodesToCollisionCheck;
	TSet<FPinLink> VisitedLinks;
	TSet<UEdGraphNode*> TempChildren;
	FormatY_Recursive(RootNode, nullptr, nullptr, NodesToCollisionCheck, VisitedLinks, true, TempChildren);
}

void FSimpleFormatter::FormatY_Recursive(
	UEdGraphNode* CurrentNode,
	UEdGraphPin* CurrentPin,
	UEdGraphPin* ParentPin,
	TSet<UEdGraphNode*>& NodesToCollisionCheck,
	TSet<FPinLink>& VisitedLinks,
	bool bSameRow,
	TSet<UEdGraphNode*>& Children)
{
	// 	const FString NodeNameA = CurrentNode == nullptr
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

	if (ParentPin)
	{
		if (UEdGraphNode* ParentNode = ParentPin->GetOwningNode())
		{
			NodeRelativeMapping.UpdateRelativeY(CurrentNode, ParentNode);
		}
	}

	uint16 CollisionLimit = 30;
	while (true)
	{
		if (CollisionLimit <= 0)
		{
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("BlueprintAssist: FormatY failed to resolve collision!"));
			break;
		}

		CollisionLimit -= 1;

		bool bNoCollision = true;
		for (UEdGraphNode* NodeToCollisionCheck : NodesToCollisionCheck)
		{
			if (NodeToCollisionCheck == CurrentNode)
			{
				continue;
			}

			FSlateRect MyBounds = GetNodeBounds(CurrentNode);
			const FMargin CollisionPadding(0, 0, FormatterSettings.Padding.X * 0.75f, FormatterSettings.Padding.Y);

			FSlateRect OtherBounds = GetNodeBounds(NodeToCollisionCheck);

			OtherBounds = OtherBounds.ExtendBy(CollisionPadding);

			if (FSlateRect::DoRectanglesIntersect(MyBounds.ExtendBy(CollisionPadding), OtherBounds))
			{
				bNoCollision = false;
				const int32 Delta = OtherBounds.Bottom - MyBounds.Top;

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Collision between %d | %s (%s) and %s (%s)"),
				// 	Delta + 1,
				// 	*FBAUtils::GetNodeName(CurrentNode), *MyBounds.ToString(),
				// 	*FBAUtils::GetNodeName(NodeToCollisionCheck), *OtherBounds.ToString());

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tMoved node single %s"), *FBAUtils::GetNodeName(CurrentNode));
				CurrentNode->NodePosY += Delta + 1;
				NodeRelativeMapping.UpdateRelativeY(CurrentNode, NodeToCollisionCheck);
			}
		}

		if (bNoCollision)
		{
			break;
		}
	}

	NodesToCollisionCheck.Emplace(CurrentNode);

	TArray<TArray<UEdGraphPin*>> OutputInput;

	const EEdGraphPinDirection Direction = ParentPin == nullptr ? EGPD_Input : ParentPin->Direction.GetValue();

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Pin Direction: %d"), Direction);

	OutputInput.Add(FBAUtils::GetLinkedPins(CurrentNode, Direction));
	OutputInput.Add(FBAUtils::GetLinkedPins(CurrentNode, UEdGraphPin::GetComplementaryDirection(Direction)));

	bool bFirstPin = true;

	UEdGraphPin* MainPin = CurrentPin;

	auto& GraphHandlerCapture = GraphHandler;

	for (TArray<UEdGraphPin*>& Pins : OutputInput)
	{
		UEdGraphPin* LastLinked = CurrentPin;
		UEdGraphPin* LastProcessed = nullptr;

		int DeltaY = 0;
		for (UEdGraphPin* MyPin : Pins)
		{
			TArray<UEdGraphPin*> LinkedPins = MyPin->LinkedTo;

			for (int i = 0; i < LinkedPins.Num(); ++i)
			{
				UEdGraphPin* OtherPin = LinkedPins[i];
				UEdGraphNode* OtherNode = OtherPin->GetOwningNode();
				FPinLink Link(MyPin, OtherPin);

				bool bIsSameLink = Path.Contains(Link);

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tIter Child %s"), *FBAUtils::GetNodeName(OtherNode));
				//
				// if (!bIsSameLink)
				// {
				// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tNot same link!"));
				// }

				if (VisitedLinks.Contains(Link)
					// || !NodePool.Contains(OtherNode)
					|| NodesToCollisionCheck.Contains(OtherNode)
					|| !bIsSameLink)
				{
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tSkipping child"));
					continue;
				}
				VisitedLinks.Add(Link);

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\tTaking Child %s"), *FBAUtils::GetNodeName(OtherNode));

				FBAUtils::StraightenPin(GraphHandler, MyPin, OtherPin);

				bool bChildIsSameRow = IsSameRow(Link);

				if (bFirstPin && (ParentPin == nullptr || MyPin->Direction == ParentPin->Direction))
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
						const int32 NewNodePosY = FMath::Max(OtherNode->NodePosY, LastProcessed->GetOwningNode()->NodePosY);
						FBAUtils::SetNodePosY(GraphHandler, OtherNode, NewNodePosY);
					}
				}

				TSet<UEdGraphNode*> LocalChildren;
				FormatY_Recursive(OtherNode, OtherPin, MyPin, NodesToCollisionCheck, VisitedLinks, bChildIsSameRow, LocalChildren);
				Children.Append(LocalChildren);

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
						FSlateRect Bounds = GetNodeArrayBounds(LocalChildren.Array());

						//UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tPin to avoid %s (%s)"), *FBAUtils::GetPinName(PinToAvoid), *FBAUtils::GetPinName(OtherPin));
						const float PinPos = GraphHandler->GetPinY(PinToAvoid) + TrackSpacing;
						const float Delta = PinPos - Bounds.Top;

						if (Delta > 0)
						{
							for (UEdGraphNode* Child : LocalChildren)
							{
								Child->NodePosY += Delta;
							}
						}
					}
				}

				LastProcessed = OtherPin;
			}

			LastLinked = MyPin;

			DeltaY += 1;
		}
	}

	Children.Add(CurrentNode);

	if (bSameRow && ParentPin != nullptr)
	{
		//UE_LOG(LogBlueprintAssist, Warning, TEXT("\t\t\tStraightening pin from %s to %s"),
		//       *FBAUtils::GetPinName(CurrentPin),
		//       *FBAUtils::GetPinName(ParentPin));
		// SameRowMapping.Add(FPinLink(CurrentPin, ParentPin));
		// SameRowMapping.Add(FPinLink(ParentPin, CurrentPin));
		// SameRowMappingDirect.Add(CurrentPin, ParentPin);
		// SameRowMappingDirect.Add(ParentPin, CurrentPin);

		FBAUtils::StraightenPin(GraphHandler, CurrentPin, ParentPin);
	}
}

TSet<UEdGraphNode*> FSimpleFormatter::GetFormattedNodes()
{
	return FormattedNodes;
}

void FSimpleFormatter::ApplyCommentPaddingX()
{
	// UE_LOG(LogTemp, Warning, TEXT("EXPAND COMMENTS X Comments"));
	TArray<UEdGraphNode*> Contains = GetFormattedNodes().Array();
	for (TSharedPtr<FBACommentContainsNode> ContainsNode : CommentHandler.GetRootNodes())
	{
		for (UEdGraphNode* Node : ContainsNode->AllContainedNodes)
		{
			Contains.RemoveSwap(Node);
		}
	}

	ApplyCommentPaddingX_Recursive(Contains, CommentHandler.GetRootNodes().Array());
}

void FSimpleFormatter::ApplyCommentPaddingX_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes)
{
	const auto LeftMost = [&](UEdGraphNode& NodeA, UEdGraphNode& NodeB)
	{
		const FSlateRect BoundsA = GetNodeBounds(&NodeA);
		const FSlateRect BoundsB = GetNodeBounds(&NodeB);
		if (FormatterSettings.FormatterDirection == EGPD_Output)
		{
			return BoundsA.Left < BoundsB.Left;
		}

		return BoundsA.Right > BoundsB.Right;
	};

	for (TSharedPtr<FBACommentContainsNode> Contains : ContainsNodes)
	{
		NodeSet.Add(Contains->Comment);
	}

	NodeSet.Sort(LeftMost);

	for (TSharedPtr<FBACommentContainsNode> ContainsNode : ContainsNodes)
	{
		ApplyCommentPaddingX_Recursive(ContainsNode->OwnedNodes, ContainsNode->Children);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Format SubGraph %d"), static_cast<int>(FormatterSettings.FormatterDirection));
	// for (UEdGraphNode* Node : NodeSet)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Node));
	// }

	for (UEdGraphNode* NodeA : NodeSet)
	{
		// collide only with our children
		TSet<TSharedPtr<FFormatXInfo>> Children;
		if (UEdGraphNode_Comment* CommentA = Cast<UEdGraphNode_Comment>(NodeA))
		{
			for (UEdGraphNode* Node : CommentHandler.GetContainsNode(CommentA)->AllContainedNodes)
			{
				if (TSharedPtr<FFormatXInfo> FormatXInfo = FormatXInfoMap.FindRef(Node))
				{
					Children.Append(FormatXInfo->Children);
				}
			}
		}
		else
		{
			if (TSharedPtr<FFormatXInfo> FormatXInfo = FormatXInfoMap.FindRef(NodeA))
			{
				Children.Append(FormatXInfo->Children);
			}
		}

		// UE_LOG(LogTemp, Warning, TEXT("Children for node %s"), *FBAUtils::GetNodeName(NodeA));
		// for (TSharedPtr<FFormatXInfo> Info : Children)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("\t%s"), *FBAUtils::GetNodeName(Info->GetNode()));
		// }

		for (TSharedPtr<FFormatXInfo> Info : Children)
		{
			UEdGraphNode* NodeB = Info->GetNode();

			// UE_LOG(LogTemp, Warning, TEXT("TRY {%s} Checking {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

			if (!SameRowMapping.Contains(Info->Link))
			{
				// UE_LOG(LogTemp, Warning, TEXT("\tNOt same row skipping"));
				continue;
			}

			if (!NodeSet.Contains(NodeB))
			{
				bool bHasContainingComment = false;
				for (auto Contains : ContainsNodes)
				{
					if (Contains->AllContainedNodes.Contains(NodeB))
					{
						NodeB = Contains->Comment;
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

			FSlateRect BoundsA = GetNodeBounds(NodeA).ExtendBy(FMargin(FormatterSettings.Padding.X, 0.f));
			FSlateRect BoundsB = GetNodeBounds(NodeB);

			if (CommentA)
			{
				BoundsA = CommentHandler.GetCommentBounds(CommentA).ExtendBy(FMargin(FormatterSettings.Padding.X, 0.f));
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
					for (auto Node : CommentHandler.GetContainsNode(CommentB)->AllContainedNodes)
					{
						if (!FormatXInfoMap.Contains(Node))
						{
							continue;
						}

						AllChildren.Add(Node);
						AllChildren.Append(FormatXInfoMap[Node]->GetChildren());
					}

					// UE_LOG(LogTemp, Warning, TEXT("\tNode {%s} Colliding with COMMENT {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

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
					if (!FormatXInfoMap.Contains(NodeB))
					{
						continue;
					}

					// UE_LOG(LogTemp, Warning, TEXT("COMMENT {%s} Colliding with Node {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

					NodeB->NodePosX += Delta;
					for (auto Child : FormatXInfoMap[NodeB]->GetChildren())
					{
						Child->NodePosX += Delta;
						// UE_LOG(LogTemp, Warning, TEXT("\tMove child %s"), *FBAUtils::GetNodeName(Child));
					}
				}
			}
		}
	}
}

void FSimpleFormatter::ApplyCommentPaddingY()
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

void FSimpleFormatter::ApplyCommentPaddingY_Recursive(TArray<UEdGraphNode*> NodeSet, TArray<TSharedPtr<FBACommentContainsNode>> ContainsNodes)
{
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

			FSlateRect BoundsA = GetNodeBounds(NodeA).ExtendBy(FMargin(0, FormatterSettings.Padding.Y));
			FSlateRect BoundsB = GetNodeBounds(NodeB);

			// UE_LOG(LogTemp, Warning, TEXT("{%s} Checking {%s}"), *FBAUtils::GetNodeName(NodeA), *FBAUtils::GetNodeName(NodeB));

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

				// UE_LOG(LogTemp, Warning, TEXT("\tColliding move %f"), Delta);

				SetNodeY_KeepingSpacingVisited(NodeB, NodeB->NodePosY + Delta, Visited);
			}
		}
	}
}
