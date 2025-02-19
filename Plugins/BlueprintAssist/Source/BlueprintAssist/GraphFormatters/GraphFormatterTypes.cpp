// Copyright 2021 fpwong. All Rights Reserved.

#include "GraphFormatterTypes.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistSettings.h"
#include "BlueprintAssistUtils.h"
#include "EdGraphFormatter.h"
#include "EdGraphNode_Comment.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistGraphOverlay.h"
#include "EdGraph/EdGraphPin.h"
#include "Editor/BlueprintGraph/Classes/K2Node_Knot.h"
#include "Runtime/SlateCore/Public/Layout/SlateRect.h"

FNodeInfo::FNodeInfo(
	UEdGraphNode* InNode,
	UEdGraphPin* InPin,
	TSharedPtr<FNodeInfo> InParent,
	UEdGraphPin* InParentPin,
	const EEdGraphPinDirection InDirection)
	: Node(InNode)
	, Pin(InPin)
	, Direction(InDirection)
	, Children(TArray<TSharedPtr<FNodeInfo>>())
{
	Link = FPinLink(InParentPin, InPin);
}

void FNodeInfo::SetParent(TSharedPtr<FNodeInfo> NewParent, UEdGraphPin* MyPin)
{
	Pin = MyPin;

	if (Parent.IsValid())
	{
		Parent->Children.Remove(SharedThis(this));
	}

	if (NewParent.IsValid() && NewParent != Parent)
	{
		NewParent->Children.Add(SharedThis(this));
	}

	Parent = NewParent;
}

int32 FNodeInfo::GetChildX(
	UEdGraphNode* Child,
	TSharedPtr<FBAGraphHandler> GraphHandler,
	const FVector2D& Padding,
	const EEdGraphPinDirection ChildDirection) const
{
	const FSlateRect MyBounds = FBAUtils::GetCachedNodeBounds(GraphHandler, Node);

	const float ChildWidth = FBAUtils::GetCachedNodeBounds(GraphHandler, Child).GetSize().X;

	const float NewNodePos
		= ChildDirection == EGPD_Input
		? MyBounds.Left - ChildWidth - Padding.X - 1
		: MyBounds.Right + Padding.X + 1;

	return FMath::RoundToInt(NewNodePos);
}

bool FNodeInfo::DetectCycle(TSharedPtr<FNodeInfo> OtherInfo)
{
	TArray<TSharedPtr<FNodeInfo>> PendingInfos;
	PendingInfos.Add(OtherInfo);

	while (PendingInfos.Num() > 0)
	{
		TSharedPtr<FNodeInfo> NextInfo = PendingInfos.Pop();
		for (TSharedPtr<FNodeInfo> Child : NextInfo->Children)
		{
			if (Child == SharedThis(this))
			{
				return true;
			}

			PendingInfos.Add(Child);
		}
	}

	return false;
}

TArray<UEdGraphNode*> FNodeInfo::GetAllChildNodes()
{
	TArray<UEdGraphNode*> OutChildren;

	// const auto& FilterByDirection = [Direction](TSharedPtr<FFormatXInfo> Info)
	// {
	// 	return Info->Link.GetDirection() == Direction || Direction == EGPD_MAX;
	// };
	TArray<TSharedPtr<FNodeInfo>> PendingInfo = Children.Array();

	while (PendingInfo.Num() > 0)
	{
		TSharedPtr<FNodeInfo> CurrentInfo = PendingInfo.Pop();
		if (OutChildren.Contains(CurrentInfo->GetNode()))
		{
			break;
		}

		OutChildren.Push(CurrentInfo->GetNode());

		for (TSharedPtr<FNodeInfo> Info : CurrentInfo->Children)
		{
			PendingInfo.Push(Info);
		}
	}

	return OutChildren;
}

void FNodeInfo::MoveChildren(
	TSharedPtr<FNodeInfo> Info,
	TSharedPtr<FBAGraphHandler> GraphHandler,
	const FVector2D& Padding,
	TSet<UEdGraphNode*>& TempVisited) const
{
	for (TSharedPtr<FNodeInfo> Child : Info->Children)
	{
		if (TempVisited.Contains(Child->Node))
		{
			continue;
		}

		TempVisited.Add(Child->Node);

		Child->Node->NodePosX = Info->GetChildX(Child->Node, GraphHandler, Padding, Child->Direction);
		MoveChildren(Child, GraphHandler, Padding, TempVisited);
	}
}

FString FNodeInfo::ToString() const
{
	UEdGraphNode* ParentNode = Parent.IsValid() ? Parent->Node : nullptr;

	return FString::Printf(
		TEXT("NodeInfo <%s> | Par <%s>"),
		*Node->GetNodeTitle(ENodeTitleType::ListView).ToString(),
		ParentNode ? *FBAUtils::GetNodeName(ParentNode) : *FString("null")
	);
}

TArray<UEdGraphNode*> FNodeInfo::GetChildNodes()
{
	TArray<UEdGraphNode*> ChildNodes;
	for (TSharedPtr<FNodeInfo> Info : Children)
	{
		ChildNodes.Emplace(Info->Node);
	}

	return ChildNodes;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphPin* FPinLink::GetFromPin()
{
	return FromHandle.GetPin();
}

UEdGraphPin* FPinLink::GetToPin()
{
	return ToHandle.GetPin();
}

UEdGraphNode* FPinLink::GetFromNode()
{
	if (UEdGraphPin* Pin = GetFromPin())
	{
		return Pin->GetOwningNodeUnchecked();
	}

	return nullptr;
}

UEdGraphNode* FPinLink::GetToNode()
{
	if (UEdGraphPin* Pin = GetToPin())
	{
		return Pin->GetOwningNodeUnchecked();
	}

	return nullptr;
}

UEdGraphNode* FPinLink::GetNode() const
{
	return To == nullptr ? FallbackNode : To->GetOwningNodeUnchecked();
}

EEdGraphPinDirection FPinLink::GetDirection() const
{
	return From != nullptr ? From->Direction.GetValue() : EGPD_Output;
}

FString FPinLink::ToString() const
{
	UEdGraphNode* ParentNode = From == nullptr ? nullptr : From->GetOwningNodeUnchecked();
	return FString::Printf(
		TEXT("<%s> to <%s> | Pin <%s> to <%s> "),
		*FBAUtils::GetNodeName(ParentNode),
		*FBAUtils::GetNodeName(GetNode()),
		*FBAUtils::GetPinName(From),
		*FBAUtils::GetPinName(To)
	);
}

FFormatXInfo::FFormatXInfo(const FPinLink& InLink, TSharedPtr<FFormatXInfo> InParent)
	: Link(InLink)
	, Parent(InParent) {}

UEdGraphNode* FFormatXInfo::GetNode() const
{
	return Link.GetNode();
}

void FFormatXInfo::SetParent(TSharedPtr<FFormatXInfo> NewParent)
{
	if (Parent.IsValid())
	{
		Parent->Children.Remove(SharedThis(this));
	}

	if (NewParent.IsValid())
	{
		//const FString OldParent
		//	= Parent != nullptr
		//	? *FBlueprintAssistUtils::GetNodeName(Parent->GetNode())
		//	: FString("None");
		//UE_LOG(
		//	LogBlueprintAssist,
		//	Warning,
		//	TEXT("\tSet Parent for %s to %s (Old : %s)"),
		//	*FBlueprintAssistUtils::GetNodeName(GetNode()),
		//	*FBlueprintAssistUtils::GetNodeName(NewParent->GetNode()),
		//	*OldParent);

		NewParent->Children.Add(SharedThis(this));
	}

	Parent = NewParent;
}

TArray<UEdGraphNode*> FFormatXInfo::GetChildren(EEdGraphPinDirection Direction, bool bInitialDirectionOnly) const
{
	TArray<UEdGraphNode*> OutChildren;

	const auto& FilterByDirection = [Direction](TSharedPtr<FFormatXInfo> Info)
	{
		return Info->Link.GetDirection() == Direction || Direction == EGPD_MAX;
	};
	TArray<TSharedPtr<FFormatXInfo>> PendingInfo = Children.FilterByPredicate(FilterByDirection);

	while (PendingInfo.Num() > 0)
	{
		TSharedPtr<FFormatXInfo> CurrentInfo = PendingInfo.Pop();
		if (OutChildren.Contains(CurrentInfo->GetNode()))
		{
			break;
		}

		OutChildren.Push(CurrentInfo->GetNode());

		for (TSharedPtr<FFormatXInfo> Info : bInitialDirectionOnly ? CurrentInfo->Children : CurrentInfo->Children.FilterByPredicate(FilterByDirection))
		{
			PendingInfo.Push(Info);
		}
	}

	return OutChildren;
}

TArray<UEdGraphNode*> FFormatXInfo::GetChildrenWithFilter(TFunctionRef<bool(TSharedPtr<FFormatXInfo>)> Filter, EEdGraphPinDirection Direction, bool bInitialDirectionOnly) const
{
	TArray<UEdGraphNode*> OutChildren;

	const auto& FilterByDirection = [Direction](TSharedPtr<FFormatXInfo> Info)
	{
		return Info->Link.GetDirection() == Direction || Direction == EGPD_MAX;
	};
	TArray<TSharedPtr<FFormatXInfo>> PendingInfo = Children.FilterByPredicate(FilterByDirection);

	while (PendingInfo.Num() > 0)
	{
		TSharedPtr<FFormatXInfo> CurrentInfo = PendingInfo.Pop();
		if (OutChildren.Contains(CurrentInfo->GetNode()))
		{
			continue;
		}

		if (Filter(CurrentInfo))
		{
			continue;
		}

		OutChildren.Push(CurrentInfo->GetNode());

		for (TSharedPtr<FFormatXInfo> Info : bInitialDirectionOnly ? CurrentInfo->Children : CurrentInfo->Children.FilterByPredicate(FilterByDirection))
		{
			PendingInfo.Push(Info);
		}
	}

	return OutChildren;
}

TArray<UEdGraphNode*> FFormatXInfo::GetImmediateChildren() const
{
	TArray<UEdGraphNode*> OutChildren;
	for (TSharedPtr<FFormatXInfo> Child : Children)
	{
		OutChildren.Add(Child->GetNode());
	}
	return OutChildren;
}

TArray<FPinLink> FFormatXInfo::GetChildrenAsLinks(EEdGraphPinDirection Direction) const
{
	TArray<FPinLink> OutLinks;
	for (TSharedPtr<FFormatXInfo> Child : Children)
	{
		if (Child->Link.GetDirection() == Direction)
		{
			OutLinks.Add(Child->Link);
		}
	}
	return OutLinks;
}

TSharedPtr<FFormatXInfo> FFormatXInfo::GetRootParent()
{
	TSet<TSharedPtr<FFormatXInfo>> Visited;
	TSharedPtr<FFormatXInfo> Next = SharedThis(this);
	while (Next->Parent.IsValid())
	{
		if (Visited.Contains(Next))
		{
			return nullptr;
		}
		Visited.Add(Next);

		Next = Next->Parent;
	}

	return Next;
}

void FNodeRelativeMapping::UpdateRelativeY(UEdGraphNode* Node, UEdGraphNode* Root)
{
	FNodeRelativeLocation& NodeRelative = NodeRelativeYMap.FindOrAdd(Node);
	FNodeRelativeLocation& RootRelative = NodeRelativeYMap.FindOrAdd(Root);

	// update the old relative info
	if (NodeRelative.RootNode)
	{
		if (auto OldRelative = NodeRelativeYMap.Find(NodeRelative.RootNode))
		{
			OldRelative->Children.Remove(Node);
		}
	}

	NodeRelative.RootNode = Root;
	NodeRelative.RelativeOffset = Root->NodePosY - Node->NodePosY;

	RootRelative.Children.Add(Node);
}

void FNodeRelativeMapping::DebugRelativeMapping() const
{
	for (TTuple<UEdGraphNode*, FNodeRelativeLocation> RelativeYMap : NodeRelativeYMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("PARENT %s"), *FBAUtils::GetNodeName(RelativeYMap.Key));

		for (UEdGraphNode* Child : RelativeYMap.Value.Children)
		{
			UE_LOG(LogTemp, Warning, TEXT("\tCHILD %s"), *FBAUtils::GetNodeName(Child));

			// draw line between parent and child
			if (auto GraphHandler = FBAUtils::GetCurrentGraphHandler())
			{
				if (auto Overlay = GraphHandler->GetGraphOverlay())
				{
					Overlay->DrawLine(
						FBAUtils::GetNodePosition(RelativeYMap.Key),
						FBAUtils::GetNodePosition(Child),
						FLinearColor::MakeRandomColor());
				}
			}
		}
	}
}
