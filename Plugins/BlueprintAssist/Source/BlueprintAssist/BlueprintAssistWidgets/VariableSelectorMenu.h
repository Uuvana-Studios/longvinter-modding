// Copyright 2021 fpwong. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "BAFilteredList.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistTypes.h"
#include "Widgets/SCompoundWidget.h"

struct FVariableSelectorStruct : IBAFilteredListItem
{
	BA_PROPERTY* Property = nullptr;
	TSharedPtr<class BA_SUBOBJECT_EDITOR_TREE_NODE> SCSNode;

	bool bSCSNode = false;

	FString DisplayName;

	explicit FVariableSelectorStruct(BA_PROPERTY* InProperty);

	explicit FVariableSelectorStruct(TSharedPtr<class BA_SUBOBJECT_EDITOR_TREE_NODE> InNode);

	FVariableSelectorStruct() {}

	FString GetType() const { return bSCSNode ? "Component" : "Variable"; }

	virtual FString ToString() const override;
};

class BLUEPRINTASSIST_API SVariableSelectorMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SVariableSelectorMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(400, 300); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FVariableSelectorStruct>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FVariableSelectorStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FVariableSelectorStruct> Item);

	void MarkActiveSuggestion(TSharedPtr<FVariableSelectorStruct> Item);

	bool ShouldSkipProperty(BA_PROPERTY* Property);

protected:
	TSharedPtr<class SGraphActionMenu> ActionMenu;
	TSharedPtr<class BA_SUBOBJECT_EDITOR> SCSEditor;
};
