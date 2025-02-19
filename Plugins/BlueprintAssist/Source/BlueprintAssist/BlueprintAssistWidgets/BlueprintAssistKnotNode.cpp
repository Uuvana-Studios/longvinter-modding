// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistKnotNode.h"

#include "BlueprintAssistUtils.h"
#include "Brushes/SlateImageBrush.h"

////////////////
// Knot Logic
////////////////

void SBlueprintAssistGraphPinKnot::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	SGraphPin::Construct(SGraphPin::FArguments().SideToSideMargin(0.0f), InPin);
}

FSlateColor SBlueprintAssistGraphPinKnot::GetPinColor() const
{
	// Make ourselves transparent if we're the input, since we are underneath the output pin and would double-blend looking ugly
	return FLinearColor::Transparent;
}

TSharedRef<SWidget> SBlueprintAssistGraphPinKnot::GetDefaultValueWidget()
{
	return SNullWidget::NullWidget;
}

const FSlateBrush* SBlueprintAssistGraphPinKnot::GetPinIcon() const
{
	const bool bIsNodeLinked = FBAUtils::IsNodeLinked(GraphPinObj->GetOwningNode());
	return bIsNodeLinked ? FStyleDefaults::GetNoBrush() : SGraphPin::GetPinIcon();
}

//////////////////////////////
// SBlueprintAssistKnotNode
//////////////////////////////
TSharedPtr<SGraphPin> SBlueprintAssistKnotNode::CreatePinWidget(UEdGraphPin* Pin) const
{
	return SNew(SBlueprintAssistGraphPinKnot, Pin);
}

const FSlateBrush* SBlueprintAssistKnotNode::GetShadowBrush(bool bSelected) const
{
	return SGraphNodeKnot::GetShadowBrush(bSelected);
}
