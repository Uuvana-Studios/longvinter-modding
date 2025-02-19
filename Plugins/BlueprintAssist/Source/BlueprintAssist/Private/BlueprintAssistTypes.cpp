#include "BlueprintAssistTypes.h"

#include "EdGraph/EdGraph.h"

void FBAGraphPinHandle::SetPin(UEdGraphPin* Pin)
{
	if (!FBAUtils::IsValidPin(Pin))
	{
		Invalidate();
	}
	else if (UEdGraphNode* Node = Pin->GetOwningNodeUnchecked())
	{
		Graph = Node->GetGraph();
		NodeGuid = Node->NodeGuid;
		PinId = Pin->PinId;
		PinType = Pin->PinType;
		PinName = Pin->PinName;
	}
}

UEdGraphPin* FBAGraphPinHandle::GetPin()
{
	if (!IsValid())
	{
		return nullptr;
	}

	for (auto Node : Graph->Nodes)
	{
		if (Node->NodeGuid == NodeGuid)
		{
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (Pin->PinId == PinId)
				{
					return Pin;
				}
			}

			// guid failed, find using PinType & PinName
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (Pin->PinType == PinType && Pin->PinName == PinName)
				{
					// side effect: also update the latest PinId
					PinId = Pin->PinId;

					return Pin;
				}
			}

			return nullptr;
		}
	}

	return nullptr;
}

void FBANodePinHandle::SetPin(UEdGraphPin* Pin)
{
	if (Pin)
	{
		PinId = Pin->PinId;
		Node = Pin->GetOwningNode();
		PinType = Pin->PinType;
		PinName = Pin->PinName;
	}
	else
	{
		PinId.Invalidate();
		Node = nullptr;
		PinType.ResetToDefaults();
		PinName = NAME_None;
	}
}

UEdGraphPin* FBANodePinHandle::GetPin()
{
	if (!IsValid())
	{
		return nullptr;
	}

	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->PinId == PinId)
		{
			return Pin;
		}
	}

	// guid failed, find using PinType & PinName
	for (UEdGraphPin* Pin : Node->Pins)
	{
		if (Pin->PinType == PinType && Pin->PinName == PinName)
		{
			// side effect: also update the latest PinId
			PinId = Pin->PinId;

			return Pin;
		}
	}

	return nullptr;
}