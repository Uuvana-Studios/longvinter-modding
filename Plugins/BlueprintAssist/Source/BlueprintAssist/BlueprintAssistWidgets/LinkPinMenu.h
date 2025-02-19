// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BAFilteredList.h"

class UEdGraphPin;
class FBAGraphHandler;

struct FPinLinkerStruct : IBAFilteredListItem
{
	UEdGraphPin* Pin;

	FPinLinkerStruct(UEdGraphPin* InPin)
		: Pin(InPin) { }

	virtual FString ToString() const override;
};

class BLUEPRINTASSIST_API SLinkPinMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SLinkPinMenu)
		: _GraphHandler(TSharedPtr<FBAGraphHandler>())
		, _SourcePin(nullptr) { }
		SLATE_ARGUMENT(TSharedPtr<FBAGraphHandler>, GraphHandler)
		SLATE_ARGUMENT(UEdGraphPin*, SourcePin)
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(400, 200); }

	void Construct(const FArguments& InArgs);
	virtual ~SLinkPinMenu() override;

	void InitListItems(TArray<TSharedPtr<FPinLinkerStruct>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FPinLinkerStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FPinLinkerStruct> Item);

	void MarkActiveSuggestion(TSharedPtr<FPinLinkerStruct> Item);

protected:
	TSharedPtr<SBAFilteredList<TSharedPtr<FPinLinkerStruct>>> FilteredList;
	TSharedPtr<FBAGraphHandler> GraphHandler;
	UEdGraphPin* SourcePin = nullptr;
	FVector2D SavedLocation;
	TSharedPtr<FPinLinkerStruct> LastSelectedItem;

	bool CanConnectSourceToPin(UEdGraphPin* Pin);

	virtual void Tick(
		const FGeometry& AllottedGeometry,
		const double InCurrentTime,
		const float InDeltaTime) override;
};
