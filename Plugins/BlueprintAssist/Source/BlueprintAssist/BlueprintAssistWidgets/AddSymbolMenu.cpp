// Copyright 2021 fpwong. All Rights Reserved.

#include "AddSymbolMenu.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintEditor.h"
#include "BlueprintAssist/BlueprintAssistUtils/BAMiscUtils.h"
#include "Runtime/SlateCore/Public/Types/SlateEnums.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

#if BA_UE_VERSION_OR_LATER(5, 0)
#include "SSubobjectEditor.h"
#else
#include "Editor/Kismet/Public/SSCSEditor.h"
#endif


FString FAddSymbolStruct::ToString() const
{
	if (DocumentType.IsSet())
	{
		switch (DocumentType.GetValue())
		{
			case FBlueprintEditor::CGT_NewFunctionGraph:
				return FString("Add Function");
			case FBlueprintEditor::CGT_NewMacroGraph:
				return FString("Add Macro");
			case FBlueprintEditor::CGT_NewEventGraph:
				return FString("Add Event Graph");
			default:
				break;
		}
	}
	else if (OtherDocumentType.IsSet())
	{
		switch (OtherDocumentType.GetValue())
		{
			case EBAOtherDocumentType::Variable:
				return FString("Add Variable");
			case EBAOtherDocumentType::LocalVariable:
				return FString("Add Local Variable");
			case EBAOtherDocumentType::Delegate:
				return FString("Add Event Dispatcher");
			default:
				break;
		}
	}
	else if (Widget.IsValid())
	{
		return "Add Component";
	}

	return "Error";
}

void SAddSymbolMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FAddSymbolStruct>>)
		.InitListItems(this, &SAddSymbolMenu::InitListItems)
		.OnGenerateRow(this, &SAddSymbolMenu::CreateItemWidget)
		.OnSelectItem(this, &SAddSymbolMenu::SelectItem)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Add Symbol"))
	];
}

void SAddSymbolMenu::InitListItems(TArray<TSharedPtr<FAddSymbolStruct>>& Items)
{
	FBlueprintEditor* BPEditor = FBAMiscUtils::GetEditorFromActiveTabCasted<UBlueprint, FBlueprintEditor>();
	if (!BPEditor)
	{
		return;
	}

	if (BPEditor->InEditingMode())
	{
		Items.Add(MakeShareable(new FAddSymbolStruct(EBAOtherDocumentType::Variable)));
		Items.Add(MakeShareable(new FAddSymbolStruct(EBAOtherDocumentType::Delegate)));

		if (BPEditor->CanAddNewLocalVariable())
		{
			Items.Add(MakeShareable(new FAddSymbolStruct(EBAOtherDocumentType::LocalVariable)));
		}
	}

	const auto DocumentTypes = {
		FBlueprintEditor::CGT_NewFunctionGraph,
		FBlueprintEditor::CGT_NewMacroGraph,
		FBlueprintEditor::CGT_NewEventGraph,
	};

	for (auto& DocumentType : DocumentTypes)
	{
		if (BPEditor->NewDocument_IsVisibleForType(DocumentType))
		{
			Items.Add(MakeShareable(new FAddSymbolStruct(DocumentType)));
		}
	}

#if BA_UE_VERSION_OR_LATER(5, 0)
	if (TSharedPtr<SSubobjectEditor> SCSEditor = BPEditor->GetSubobjectEditor())
#else
	if (TSharedPtr<SSCSEditor> SCSEditor = BPEditor->GetSCSEditor())
#endif
	{
#if ENGINE_MAJOR_VERSION >= 5
		TSharedPtr<SWidget> AddComponentsButton = FBAUtils::GetChildWidget(SCSEditor->GetToolButtonsBox(), "SComponentClassCombo");
#else
		TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();
		TSharedPtr<SWidget> AddComponentsButton = FBAUtils::GetChildWidget(Window, "SComponentClassCombo");
#endif
		if (AddComponentsButton.IsValid())
		{
			TSharedPtr<FAddSymbolStruct> AddSymbolStruct = MakeShareable(new FAddSymbolStruct(AddComponentsButton));
			Items.Add(AddSymbolStruct);
		}
	}
}

TSharedRef<ITableRow> SAddSymbolMenu::CreateItemWidget(TSharedPtr<FAddSymbolStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(4.0, 2.0))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).FillWidth(1)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->ToString()))
			]
		];
}

void SAddSymbolMenu::SelectItem(TSharedPtr<FAddSymbolStruct> Item)
{
	if (Item->DocumentType.IsSet())
	{
		FBlueprintEditor* BPEditor = FBAMiscUtils::GetEditorFromActiveTabCasted<UBlueprint, FBlueprintEditor>();
		if (BPEditor)
		{
			BPEditor->NewDocument_OnClicked(Item->DocumentType.GetValue());
		}
	}
	else if (Item->OtherDocumentType.IsSet())
	{
		FBlueprintEditor* BPEditor = FBAMiscUtils::GetEditorFromActiveTabCasted<UBlueprint, FBlueprintEditor>();
		if (BPEditor)
		{
			switch (Item->OtherDocumentType.GetValue())
			{
				case EBAOtherDocumentType::Variable:
					BPEditor->OnAddNewVariable();
					break;
				case EBAOtherDocumentType::LocalVariable:
					BPEditor->OnAddNewLocalVariable();
					break;
				case EBAOtherDocumentType::Delegate:
					BPEditor->OnAddNewDelegate();
					break;
				default: ;
			}
		}
	}
	else if (Item->Widget.IsValid())
	{
		FBAUtils::InteractWithWidget(Item->Widget);
	}
}
