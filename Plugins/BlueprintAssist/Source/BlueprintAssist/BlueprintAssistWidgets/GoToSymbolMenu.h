// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BAFilteredList.h"

class UEdGraph;
class UEdGraphNode;

struct FGoToSymbolStruct final : IBAFilteredListItem
{
	UEdGraphNode* EventNode;
	UEdGraph* Graph;

	FGoToSymbolStruct(UEdGraphNode* InEventNode, UEdGraph* InGraph)
		: EventNode(InEventNode)
		, Graph(InGraph) { }

	FGoToSymbolStruct()
		: EventNode(nullptr)
		, Graph(nullptr) { }

	virtual FString ToString() const override;

	virtual FString GetSearchText() const override;

	FString GetTypeDescription() const;
};

class BLUEPRINTASSIST_API SGoToSymbolMenu final : public SBorder
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SGoToSymbolMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	static FVector2D GetWidgetSize() { return FVector2D(400, 300); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FGoToSymbolStruct>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FGoToSymbolStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FGoToSymbolStruct> Item);
};
