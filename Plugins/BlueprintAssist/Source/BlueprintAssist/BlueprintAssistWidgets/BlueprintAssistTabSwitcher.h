// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BAFilteredList.h"

class SDockTab;

struct FBATabSwitcherItem : IBAFilteredListItem
{
	TSharedPtr<SDockTab> Tab;

	FBATabSwitcherItem(TSharedPtr<SDockTab> InTab)
		: Tab(InTab) { }

	virtual FString ToString() const override;

	bool operator==(const FBATabSwitcherItem& Other) const
	{
		return Tab != Other.Tab;
	}

	bool operator!=(const FBATabSwitcherItem& Other) const
	{
		return !(*this == Other);
	}
};

class BLUEPRINTASSIST_API SBATabSwitcher final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SBATabSwitcher) { }
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(400, 300); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FBATabSwitcherItem>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FBATabSwitcherItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void MarkActiveSuggestion(TSharedPtr<FBATabSwitcherItem> Item);

	void SelectItem(TSharedPtr<FBATabSwitcherItem> Item);
};
