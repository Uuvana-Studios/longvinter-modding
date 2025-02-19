// Copyright 2021 fpwong. All Rights Reserved.

#include "EditDetailsMenu.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintEditor.h"
#include "IDetailTreeNode.h"
#include "PropertyPath.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

bool SEditDetailsMenu::CanOpenMenu()
{
	return GetDetailView().IsValid();
}

void SEditDetailsMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FEditDetailsStruct>>)
		.InitListItems(this, &SEditDetailsMenu::InitListItems)
		.OnGenerateRow(this, &SEditDetailsMenu::CreateItemWidget)
		.OnSelectItem(this, &SEditDetailsMenu::SelectItem)
		.OnMarkActiveSuggestion(this, &SEditDetailsMenu::MarkActiveSuggestion)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Edit Details"))
	];
}

SEditDetailsMenu::~SEditDetailsMenu()
{
	// stop highlighting a property
	HighlightProperty(FPropertyPath());
}

void SEditDetailsMenu::InitListItems(TArray<TSharedPtr<FEditDetailsStruct>>& Items)
{
	TMap<FString, TSharedPtr<FEditDetailsStruct>> ItemsByName;

	TSharedPtr<IDetailsView> DetailView = GetDetailView();
	if (!DetailView)
	{
		return;
	}

	// Grab items by widget from detail view
	TArray<TSharedPtr<SWidget>> ItemRows;
	FBAUtils::GetChildWidgets(DetailView, "SDetailSingleItemRow", ItemRows);

	for (TSharedPtr<SWidget> Row : ItemRows)
	{
		TSharedPtr<SWidget> FoundTextBlock = FBAUtils::GetChildWidget(Row, "STextBlock");
		TSharedPtr<STextBlock> TextBlock = StaticCastSharedPtr<STextBlock>(FoundTextBlock);
		FString WidgetText = TextBlock ? TextBlock->GetText().ToString() : Row->ToString();

		TSharedPtr<SWidget> FoundSplitter = FBAUtils::GetChildWidget(Row, "SSplitter");
		TSharedPtr<SSplitter> Splitter = StaticCastSharedPtr<SSplitter>(FoundSplitter);

		TSharedPtr<SWidget> Interactable;
		if (Splitter.IsValid())
		{
			if (FChildren* SplitterChildren = Splitter->GetChildren())
			{
				if (SplitterChildren->Num() >= 2)
				{
					TSharedPtr<SWidget> WidgetToSearch = SplitterChildren->GetChildAt(1);
					Interactable = FBAUtils::GetInteractableChildWidget(WidgetToSearch);
				}
			}
		}
		else
		{
			Interactable = FBAUtils::GetInteractableChildWidget(Row);
		}

		if (Interactable.IsValid() && !FBAUtils::IsWidgetOfType(Interactable->GetParentWidget(), "SResetToDefaultPropertyEditor"))
		{
			ItemsByName.Add(WidgetText, MakeShareable(new FEditDetailsStruct(Interactable, WidgetText)));
		}
	}

	// Grab items by property from detail view
	for (FPropertyPath PropertyPath : DetailView->GetPropertiesInOrderDisplayed())
	{
		BA_WEAK_FIELD_PTR<BA_PROPERTY> Property = PropertyPath.GetLeafMostProperty().Property;
		if (!Property.IsValid())
		{
			continue;
		}

		const FString PropertyDisplayString = Property->GetDisplayNameText().ToString();

		if (TSharedPtr<FEditDetailsStruct> FoundItem = ItemsByName.FindRef(PropertyDisplayString))
		{
			FoundItem->SetPropertyPath(PropertyPath);
		}
		else
		{
			ItemsByName.Add(PropertyDisplayString, MakeShareable(new FEditDetailsStruct(PropertyPath)));
		}
	}

	for (auto Elem : ItemsByName)
	{
		Items.Add(Elem.Value);
	}
}

TSharedRef<ITableRow> SEditDetailsMenu::CreateItemWidget(TSharedPtr<FEditDetailsStruct> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(2.0f, 4.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left).FillWidth(1)
			[
				SNew(STextBlock).Text(FText::FromString(Item->DetailName))
			]
		];
}

void SEditDetailsMenu::MarkActiveSuggestion(TSharedPtr<FEditDetailsStruct> Item)
{
	HighlightProperty(Item->PropertyPath);
}

void SEditDetailsMenu::SelectItem(TSharedPtr<FEditDetailsStruct> Item)
{
	if (Item->PropertyPath.GetNumProperties() > 0)
	{
		BA_WEAK_FIELD_PTR<BA_PROPERTY> Property = Item->PropertyPath.GetLeafMostProperty().Property;
		if (Property.IsValid())
		{
			TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();

			TArray<TSharedPtr<SWidget>> ItemRows;
			FBAUtils::GetChildWidgets(Window, "SDetailSingleItemRow", ItemRows);

			for (TSharedPtr<SWidget> Row : ItemRows)
			{
				TSharedPtr<SWidget> FoundWidget = FBAUtils::GetChildWidget(Row, "STextBlock");
				TSharedPtr<STextBlock> TextBlock = StaticCastSharedPtr<STextBlock>(FoundWidget);
				FString WidgetText = TextBlock->GetText().ToString();

				if (WidgetText.Equals(Property->GetDisplayNameText().ToString()))
				{
					TSharedPtr<SWidget> FoundSplitter = FBAUtils::GetChildWidget(Row, "SSplitter");
					TSharedPtr<SSplitter> Splitter = StaticCastSharedPtr<SSplitter>(FoundSplitter);

					if (Splitter.IsValid())
					{
						if (FChildren* SplitterChildren = Splitter->GetChildren())
						{
							if (SplitterChildren->Num() >= 2)
							{
								TSharedPtr<SWidget> WidgetToSearch = SplitterChildren->GetChildAt(1);
								TSharedPtr<SWidget> Interactable = FBAUtils::GetInteractableChildWidget(WidgetToSearch);

								if (Interactable.IsValid() && !FBAUtils::IsWidgetOfType(Interactable->GetParentWidget(), "SResetToDefaultPropertyEditor"))
								{
									FBAUtils::InteractWithWidget(Interactable);
								}
							}
						}
					}
				}
			}
		}
	}
	else // interact by saved widget
	{
		FBAUtils::InteractWithWidget(Item->Widget);
	}

	HighlightProperty(FPropertyPath());
}

TSharedPtr<IDetailsView> SEditDetailsMenu::GetDetailView()
{
	const TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();
	return FBAUtils::GetChildWidgetCasted<IDetailsView>(Window, "SDetailsView");
}

void SEditDetailsMenu::HighlightProperty(FPropertyPath Property)
{
	if (const TSharedPtr<IDetailsView> DetailView = GetDetailView())
	{
		DetailView->HighlightProperty(Property);
	}
}
