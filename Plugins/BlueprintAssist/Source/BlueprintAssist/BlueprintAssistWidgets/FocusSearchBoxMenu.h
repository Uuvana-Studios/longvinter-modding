// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BAFilteredList.h"
#include "Widgets/Views/SListView.h"

class FBAInputProcessor;
class FBAGraphHandler;
class SBlueprintContextTargetMenu;
class SEditableTextBox;
class UEdGraph;
class UEdGraphNode;
class SWidget;

struct FBlueprintActionContext;
struct FCustomExpanderData;

class ITableRow;
class SGraphPanel;
class SSearchBox;
class UK2Node_Event;
class UBlueprint;
class SDockTab;

struct FSearchBoxStruct : IBAFilteredListItem
{
	TSharedPtr<SWidget> Widget;
	TSharedPtr<SDockTab> DockTab;

	FSearchBoxStruct(TSharedPtr<SWidget> InWidget, TSharedPtr<SDockTab> InDockTab)
		: Widget(InWidget)
		, DockTab(InDockTab) { }

	virtual FString ToString() const override;

	FString GetTabLabel() const;
};

class BLUEPRINTASSIST_API SFocusSearchBoxMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SFocusSearchBoxMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(400, 300); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FSearchBoxStruct>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FSearchBoxStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FSearchBoxStruct> Item);
};
