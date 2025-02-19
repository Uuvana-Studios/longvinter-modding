// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BAFilteredList.h"

class SBlueprintContextTargetMenu;
class SWidget;
class FUICommandInfo;

struct FBAHotkeyItem : IBAFilteredListItem
{
	FName CommandName;
	FText CommandChord;
	FText CommandDesc;
	FText CommandLabel;
	TSharedPtr<const FUICommandInfo> CommandInfo;

	FBAHotkeyItem(TSharedPtr<FUICommandInfo> Command);

	virtual FString ToString() const override { return CommandLabel.ToString(); }
};

class BLUEPRINTASSIST_API SBAHotkeyMenu : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SBAHotkeyMenu) {}
	SLATE_ARGUMENT(FName, BindingContextName)
	SLATE_END_ARGS()

	static FVector2D GetWidgetSize() { return FVector2D(600, 500); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FBAHotkeyItem>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FBAHotkeyItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FBAHotkeyItem> Item);

	FName BindingContextName;
};
