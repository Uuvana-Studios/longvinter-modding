// Copyright 2021 fpwong. All Rights Reserved.

#include "FocusSearchBoxMenu.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

void SFocusSearchBoxMenu::Construct(
	const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FSearchBoxStruct>>)
		.InitListItems(this, &SFocusSearchBoxMenu::InitListItems)
		.OnGenerateRow(this, &SFocusSearchBoxMenu::CreateItemWidget)
		.OnSelectItem(this, &SFocusSearchBoxMenu::SelectItem)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Focus Search Box"))
	];
}

void SFocusSearchBoxMenu::InitListItems(TArray<TSharedPtr<FSearchBoxStruct>>& Items)
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();

	TArray<TSharedPtr<SWidget>> DockTabs;
	FBAUtils::GetChildWidgets(Window, "SDockTab", DockTabs);

	for (TSharedPtr<SWidget> Widget : DockTabs)
	{
		TSharedPtr<SDockTab> DockTab = StaticCastSharedPtr<SDockTab>(Widget);

		if (DockTab.IsValid())
		{
			if (DockTab->GetTabRole() == ETabRole::MajorTab)
			{
				continue;
			}

			if (!DockTab->IsForeground())
			{
				continue;
			}

			TArray<TSharedPtr<SWidget>> SearchBoxes;
			FBAUtils::GetChildWidgets(DockTab->GetContent(), "SSearchBox", SearchBoxes);

			for (TSharedPtr<SWidget> SearchBoxWidget : SearchBoxes)
			{
				if (SearchBoxWidget->GetVisibility().IsVisible() &&
					SearchBoxWidget->IsEnabled() &&
					SearchBoxWidget->GetDesiredSize().SizeSquared() > 0 &&
					SearchBoxWidget->GetTickSpaceGeometry().GetAbsoluteSize().SizeSquared() > 0)
				{
					Items.Add(MakeShareable(new FSearchBoxStruct(SearchBoxWidget, DockTab)));
				}
			}
		}
	}
}

TSharedRef<ITableRow> SFocusSearchBoxMenu::CreateItemWidget(TSharedPtr<FSearchBoxStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(2.0f, 4.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(Item->ToString()))
			]
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(Item->GetTabLabel()))
			]
		];
}

void SFocusSearchBoxMenu::SelectItem(TSharedPtr<FSearchBoxStruct> Item)
{
	if (Item->Widget.IsValid())
	{
		FSlateApplication::Get().SetKeyboardFocus(Item->Widget, EFocusCause::Navigation);
		Item->DockTab->FlashTab();
	}
}

/********************/
/* FSearchBoxStruct */
/********************/

FString FSearchBoxStruct::ToString() const
{
	TSharedPtr<SWidget> FoundWidget = FBAUtils::GetChildWidget(Widget, "SEditableText");
	TSharedPtr<SEditableText> EditableTextBox = StaticCastSharedPtr<SEditableText>(FoundWidget);
	if (EditableTextBox.IsValid())
	{
		return EditableTextBox->GetHintText().ToString();
	}

	return Widget->ToString();
}

FString FSearchBoxStruct::GetTabLabel() const
{
	return DockTab->GetTabLabel().ToString();
}
