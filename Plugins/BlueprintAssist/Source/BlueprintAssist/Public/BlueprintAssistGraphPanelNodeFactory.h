// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "EdGraphUtilities.h"

class BLUEPRINTASSIST_API FBlueprintAssistGraphPanelNodeFactory : public FGraphPanelNodeFactory
{
public:
	virtual TSharedPtr<SGraphNode> CreateNode(class UEdGraphNode* Node) const override;
};
