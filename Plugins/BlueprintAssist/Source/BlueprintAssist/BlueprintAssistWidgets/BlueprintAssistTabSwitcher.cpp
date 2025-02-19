// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistTabSwitcher.h"

#include "BlueprintAssistUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

void SBATabSwitcher::Construct(
	const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FBATabSwitcherItem>>)
		.InitListItems(this, &SBATabSwitcher::InitListItems)
		.OnGenerateRow(this, &SBATabSwitcher::CreateItemWidget)
		.OnMarkActiveSuggestion(this, &SBATabSwitcher::MarkActiveSuggestion)
		.OnSelectItem(this, &SBATabSwitcher::SelectItem)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Tab Switcher"))
	];
}

void SBATabSwitcher::InitListItems(TArray<TSharedPtr<FBATabSwitcherItem>>& Items)
{
	struct FLocal
	{
		static void AddChildWindows(TArray<TSharedRef<SWindow>>& OutWindows, TSharedRef<SWindow> CurrentWindow)
		{
			OutWindows.Add(CurrentWindow);

			const TArray<TSharedRef<SWindow>>& WindowChildren = CurrentWindow->GetChildWindows();
			for (int32 ChildIndex = 0; ChildIndex < WindowChildren.Num(); ++ChildIndex)
			{
				AddChildWindows(OutWindows, WindowChildren[ChildIndex]);
			}
		}
	};

	TArray<TSharedRef<SWindow>> AllWindows;
	for (TSharedRef<SWindow> Window : FSlateApplication::Get().GetInteractiveTopLevelWindows())
	{
		FLocal::AddChildWindows(AllWindows, Window);
	}

	for (TSharedRef<SWindow> Window : AllWindows)
	{
		TArray<TSharedPtr<SWidget>> FoundTabs;
		FBAUtils::GetChildWidgets(Window, "SDockTab", FoundTabs);
		for (auto Tab : FoundTabs)
		{			
			auto DockTab = StaticCastSharedPtr<SDockTab>(Tab);
			if (DockTab.IsValid())
			{
				if (DockTab->GetTabRole() == PanelTab)
				{
					continue;
				}

				TSharedPtr<FBATabSwitcherItem> TabItem = MakeShareable(new FBATabSwitcherItem(DockTab));
				if (!Items.Contains(TabItem))
				{
					Items.Add(TabItem);
				}
			}
		}
	}
}

TSharedRef<ITableRow> SBATabSwitcher::CreateItemWidget(TSharedPtr<FBATabSwitcherItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(2.0f, 4.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(2.0f, 0.0f, 4.0f, 0.0f))
			[
				SNew(SImage).Image(Item->Tab->GetTabIcon())
			]
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(Item->ToString()))
			]
		];
}

void SBATabSwitcher::MarkActiveSuggestion(TSharedPtr<FBATabSwitcherItem> Item)
{
	Item->Tab->FlashTab();
}

void SBATabSwitcher::SelectItem(TSharedPtr<FBATabSwitcherItem> Item)
{
	Item->Tab->ActivateInParent(ETabActivationCause::SetDirectly);

	auto ParentWindow = Item->Tab->GetParentWindow(); 

	if (ParentWindow.IsValid())
	{
		ParentWindow->BringToFront();
	}
	else
	{
		ParentWindow = FBAUtils::GetParentWindow(Item->Tab);
	}

	if (ParentWindow.IsValid())
	{
		ParentWindow->BringToFront();
	}
}

/****************************/
/* FBATabSwitcherItem */
/****************************/

FString FBATabSwitcherItem::ToString() const
{
	return Tab->GetTabLabel().ToString();
}
