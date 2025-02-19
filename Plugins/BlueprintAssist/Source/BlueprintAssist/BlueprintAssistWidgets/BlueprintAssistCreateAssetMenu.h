// Copyright 2021 fpwong. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BAFilteredList.h"

class IAssetTools;
class UFactory;

struct FBACreateAssetItem final : IBAFilteredListItem
{
	UFactory* Factory;

	FBACreateAssetItem(UFactory* InFactory) : Factory(InFactory) { };

	virtual FString ToString() const override;
};

class BLUEPRINTASSIST_API SBACreateAssetMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SBACreateAssetMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	IAssetTools* AssetTools;

	static FVector2D GetWidgetSize() { return FVector2D(300, 400); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FBACreateAssetItem>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FBACreateAssetItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FBACreateAssetItem> Item);
};