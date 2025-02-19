// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BAFilteredList.h"
#include "Widgets/SCompoundWidget.h"

struct FBAFileItem : IBAFilteredListItem
{
	FString FilePath;
	FBAFileItem(const FString& InFilePath) : FilePath(InFilePath) {};

	virtual FString ToString() const override { return FilePath; }
};

/**
 * 
 */
class BLUEPRINTASSIST_API SBAOpenFileMenu final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBAOpenFileMenu) {}
	SLATE_END_ARGS()

	static FVector2D GetWidgetSize() { return FVector2D(600, 500); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FBAFileItem>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FBAFileItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FBAFileItem> Item);
};
