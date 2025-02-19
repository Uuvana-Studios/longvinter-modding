// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistNodeSizeChangeData.h"

#include "BlueprintAssistUtils.h"
#include "EdGraphSchema_K2.h"

void FBAPinChangeData::UpdatePin(UEdGraphPin* Pin)
{
	bPinHidden = Pin->bHidden;
	bPinLinked = FBAUtils::IsPinLinked(Pin);
	PinValue = Pin->DefaultValue;
	PinObject = GetPinDefaultObjectName(Pin);
}

bool FBAPinChangeData::HasPinChanged(UEdGraphPin* Pin)
{
	if (bPinHidden != Pin->bHidden)
	{
		return true;
	}
	
	if (bPinLinked != FBAUtils::IsPinLinked(Pin))
	{
		// these pins do not change size
		if (Pin->PinType.PinSubCategory != UEdGraphSchema_K2::PC_Exec)
		{
			return true;
		}
	}

	if (PinValue != Pin->DefaultValue)
	{
		return true;
	}

	const FString PinDefaultObjectName = GetPinDefaultObjectName(Pin);
	if (PinObject != PinDefaultObjectName)
	{
		return true;
	}

	return false;
}

FString FBAPinChangeData::GetPinDefaultObjectName(UEdGraphPin* Pin) const
{
	return Pin->DefaultObject ? Pin->DefaultObject->GetName() : FString();
}

FBANodeSizeChangeData::FBANodeSizeChangeData(UEdGraphNode* Node)
{
	UpdateNode(Node);
}

void FBANodeSizeChangeData::UpdateNode(UEdGraphNode* Node)
{
	PinChangeData.Reset();
	for (UEdGraphPin* Pin : Node->GetAllPins())
	{
		PinChangeData.FindOrAdd(Pin->PinId).UpdatePin(Pin);
	}

	AdvancedPinDisplay = Node->AdvancedPinDisplay == ENodeAdvancedPins::Shown;
	NodeTitle = FBAUtils::GetNodeName(Node);
	bCommentBubblePinned = Node->bCommentBubblePinned;
	bCommentBubbleVisible = Node->bCommentBubbleVisible;
	NodeEnabledState = Node->GetDesiredEnabledState();
}

bool FBANodeSizeChangeData::HasNodeChanged(UEdGraphNode* Node)
{
	TArray<FGuid> PinGuids;
    PinChangeData.GetKeys(PinGuids);

	for (UEdGraphPin* Pin : Node->GetAllPins())
	{
		if (FBAPinChangeData* FoundPinData = PinChangeData.Find(Pin->PinId))
		{
			if (FoundPinData->HasPinChanged(Pin))
			{
				return true;
			}

			PinGuids.Remove(Pin->PinId);
		}
		else // added a new pin
		{
			return true;
		}
	}

	// If there are remaining pins, then they must have been removed
	if (PinGuids.Num())
	{
		return true;
	}

	if (AdvancedPinDisplay != (Node->AdvancedPinDisplay == ENodeAdvancedPins::Shown))
	{
		return true;
	}

	if (NodeTitle != FBAUtils::GetNodeName(Node))
	{
		return true;
	}

	if (bCommentBubblePinned != Node->bCommentBubblePinned)
	{
		return true;
	}

	if (bCommentBubbleVisible != Node->bCommentBubbleVisible)
	{
		return true;
	}

	if (NodeEnabledState != Node->GetDesiredEnabledState())
	{
		return true;
	}

	return false;
}
