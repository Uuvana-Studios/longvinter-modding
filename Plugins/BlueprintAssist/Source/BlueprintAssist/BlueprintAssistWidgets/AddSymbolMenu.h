// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BAFilteredList.h"
#include "BlueprintEditor.h"

enum class EBAOtherDocumentType : uint8
{
	Variable,
	LocalVariable,
	Delegate,
};

struct FAddSymbolStruct final : IBAFilteredListItem
{
	TSharedPtr<SWidget> Widget;

	TOptional<FBlueprintEditor::ECreatedDocumentType> DocumentType;
	TOptional<EBAOtherDocumentType> OtherDocumentType;

	explicit FAddSymbolStruct(FBlueprintEditor::ECreatedDocumentType InDocumentType)
	{
		DocumentType.Emplace(InDocumentType);
	}

	explicit FAddSymbolStruct(EBAOtherDocumentType InDocumentType)
	{
		OtherDocumentType.Emplace(InDocumentType);
	}

	explicit FAddSymbolStruct(const TSharedPtr<SWidget> InWidget) : Widget(InWidget) { }

	virtual FString ToString() const override;
};

class BLUEPRINTASSIST_API SAddSymbolMenu final : public SCompoundWidget
{
	// @formatter:off
	SLATE_BEGIN_ARGS(SAddSymbolMenu) { }
	SLATE_END_ARGS()
	// @formatter:on

	FVector2D GetWidgetSize() { return FVector2D(300, 200); }

	void Construct(const FArguments& InArgs);

	void InitListItems(TArray<TSharedPtr<FAddSymbolStruct>>& Items);

	TSharedRef<ITableRow> CreateItemWidget(TSharedPtr<FAddSymbolStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const;

	void SelectItem(TSharedPtr<FAddSymbolStruct> Item);
};
