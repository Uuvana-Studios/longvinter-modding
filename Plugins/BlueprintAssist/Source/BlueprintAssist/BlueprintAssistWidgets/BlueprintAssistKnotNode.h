// Copyright 2021 fpwong. All Rights Reserved.

#pragma once
#include "SGraphNodeKnot.h"

class SBlueprintAssistGraphPinKnot : public SGraphPin
{
public:
	// @formatter:off
	SLATE_BEGIN_ARGS(SBlueprintAssistGraphPinKnot) { }
	SLATE_END_ARGS()
	// @formatter:on

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

protected:
	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
	virtual FSlateColor GetPinColor() const override;
	virtual const FSlateBrush* GetPinIcon() const override;
};

class SBlueprintAssistKnotNode : public SGraphNodeKnot
{
public:
	virtual TSharedPtr<SGraphPin> CreatePinWidget(UEdGraphPin* Pin) const override;
	virtual const FSlateBrush* GetShadowBrush(bool bSelected) const override;
};
