// Copyright 2021 fpwong. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BAFilteredList.h"
#include "AssetRegistry/AssetData.h"

class IAssetFamily;
class UFactory;

enum class EBAWorkflowItemType : uint8
{
	WorkflowMode,
	AssetShortcut,
};

struct FBAWorkflowModeItem final : IBAFilteredListItem
{
	EBAWorkflowItemType Type;

	// WorkflowMode
	TSharedPtr<SWidget> InteractWidget;
	FString Name;

	// AssetShortcut
	FAssetData AssetData;
	TSharedPtr<IAssetFamily> AssetFamily;

	FBAWorkflowModeItem(EBAWorkflowItemType InType, FAssetData InAssetData, TSharedPtr<IAssetFamily> InAssetFamily)
		: Type(InType)
		, AssetData(InAssetData)
		, AssetFamily(InAssetFamily) { };

	FBAWorkflowModeItem(EBAWorkflowItemType InType, TSharedPtr<SWidget> InModeWidget, const FString& InName)
		: Type(InType)
		, InteractWidget(InModeWidget)
		, Name(InName) { };

	virtual FString ToString() const override;
};

class BLUEPRINTASSIST_API SBAWorkflowModeMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SBAWorkflowModeMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(300, 200); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FBAWorkflowModeItem>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FBAWorkflowModeItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FBAWorkflowModeItem> Item);
};
