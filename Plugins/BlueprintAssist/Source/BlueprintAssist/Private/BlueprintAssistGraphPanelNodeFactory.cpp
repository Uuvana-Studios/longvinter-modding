// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistGraphPanelNodeFactory.h"

#include "BlueprintAssistSettings.h"
#include "K2Node_Knot.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistKnotNode.h"

TSharedPtr<SGraphNode> FBlueprintAssistGraphPanelNodeFactory::CreateNode(class UEdGraphNode* InNode) const
{
	if (GetDefault<UBASettings>()->bEnableInvisibleKnotNodes)
	{
		if (Cast<UK2Node_Knot>(InNode))
		{
			TSharedRef<SBlueprintAssistKnotNode> GraphNode = SNew(SBlueprintAssistKnotNode, InNode);
			return GraphNode;
		}
	}

	return nullptr;
}
