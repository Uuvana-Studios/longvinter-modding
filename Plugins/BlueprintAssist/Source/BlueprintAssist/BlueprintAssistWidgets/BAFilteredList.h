// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistStyle.h"
#include "BlueprintAssistStyle.h"
#include "EditorStyleSet.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Views/ITypedTableView.h"
#include "Framework/Views/TableViewTypeTraits.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SListView.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

class IBAFilteredListItem
{
public:
	virtual ~IBAFilteredListItem() = default;

	virtual FString ToString() const = 0;

	virtual FString GetSearchText() const { return ToString(); }

	virtual FString GetKeySearchText() const { return ToString(); }
};

template<typename ItemType>
class BLUEPRINTASSIST_API SBAFilteredList final
	: public SCompoundWidget
	, TListTypeTraits<ItemType>::SerializerType
{
	DECLARE_DELEGATE_RetVal_TwoParams(TSharedRef<class ITableRow>, FBAOnGenerateRow, ItemType, const TSharedRef<class STableViewBase>&);
	DECLARE_DELEGATE_OneParam(FBAInitListItems, TArray<ItemType>&);
	DECLARE_DELEGATE_OneParam(FBAOnSelectItem, ItemType);
	DECLARE_DELEGATE_OneParam(FBAOnMarkActiveSuggestion, ItemType);

public:
	SLATE_BEGIN_ARGS(SBAFilteredList)
			: _WidgetSize(600, 500)
			, _MenuTitle("Menu Title")
			, _SelectionMode(ESelectionMode::Single) { }

		SLATE_EVENT(FBAInitListItems, InitListItems)
		SLATE_EVENT(FBAOnSelectItem, OnSelectItem)
		SLATE_EVENT(FBAOnMarkActiveSuggestion, OnMarkActiveSuggestion)
		SLATE_EVENT(FBAOnGenerateRow, OnGenerateRow)
		SLATE_ARGUMENT(FVector2D, WidgetSize)
		SLATE_ARGUMENT(FString, MenuTitle)
		SLATE_ARGUMENT(ESelectionMode::Type, SelectionMode)
	SLATE_END_ARGS()

	FVector2D WidgetSize;
	FString MenuTitle;
	ESelectionMode::Type SelectionMode;

	int32 SuggestionIndex = INDEX_NONE;
	TArray<ItemType> AllItems;
	TArray<ItemType> FilteredItems;
	TSharedPtr<SSearchBox> FilterTextBox;
	TSharedPtr<SListView<ItemType>> FilteredItemsListView;
private:
	FBAOnSelectItem OnSelectItem;
	FBAOnMarkActiveSuggestion OnMarkActiveSuggestion;
	FText FilterText;

public:
	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct(const FArguments& InArgs)
	{
		OnSelectItem = InArgs._OnSelectItem;
		OnMarkActiveSuggestion = InArgs._OnMarkActiveSuggestion;
		WidgetSize = InArgs._WidgetSize;
		MenuTitle = InArgs._MenuTitle;
		SelectionMode = InArgs._SelectionMode;

		AllItems.Empty();

		InArgs._InitListItems.Execute(AllItems);

		FilteredItems = AllItems;

		RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SBAFilteredList::SetFocusPostConstruct));

		TSharedRef<SBorder> Content = SNew(SBorder)
			.BorderImage(FBAStyle::GetBrush("BlueprintAssist.PanelBorder"))
			.Padding(5)
			[
				SNew(SBox)
				.WidthOverride(WidgetSize.X)
				.HeightOverride(WidgetSize.Y)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(2)
					[

						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						.Padding(FMargin(8.f, 0.f))
						[
							SNew(SSeparator)
							.Orientation(Orient_Horizontal)
							.Thickness(1.0f)
							.SeparatorImage(BA_STYLE_CLASS::Get().GetBrush("Menu.Separator"))
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.Padding(FMargin(2.f, 0.f, 0.f, 0.f))
						.AutoWidth()
						[
							SNew(STextBlock)
#if ENGINE_MINOR_VERSION >= 26 || ENGINE_MAJOR_VERSION >= 5
							.TransformPolicy(ETextTransformPolicy::ToUpper)
#endif
							.Text(FText::FromString(MenuTitle))
							.Font(BA_STYLE_CLASS::Get().GetFontStyle(TEXT("PropertyWindow.BoldFont")))
							.TextStyle(BA_STYLE_CLASS::Get(), TEXT("DetailsView.CategoryTextStyle"))
							.WrapTextAt(WidgetSize.X * 0.9f)
						]
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						.Padding(FMargin(8.f, 0.f))
						[
							SNew(SSeparator)
							.Orientation(Orient_Horizontal)
							.Thickness(1.0f)
							.SeparatorImage(BA_STYLE_CLASS::Get().GetBrush("Menu.Separator"))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(12.f, 4.f)
					[
						SAssignNew(FilterTextBox, SSearchBox)
						.OnTextChanged(this, &SBAFilteredList::OnFilterTextChanged)
						.OnTextCommitted(this, &SBAFilteredList::OnFilterTextCommitted)
						.OnKeyDownHandler(this, &SBAFilteredList::OnKeyDown)
					]
					+ SVerticalBox::Slot().FillHeight(1.f).Padding(2.f)
					[
						SNew(SBorder)
						.BorderImage(FBAStyle::GetBrush("BlueprintAssist.PanelBorder"))
						[
							SAssignNew(FilteredItemsListView, SListView<ItemType>)
							.SelectionMode(SelectionMode)
							.ListItemsSource(&FilteredItems)
							.OnGenerateRow(InArgs._OnGenerateRow)
							.OnMouseButtonClick(this, &SBAFilteredList::OnListItemClicked)
							.IsFocusable(false)
						]
					]
				]
			];

		// make the menu outline
		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FBAStyle::GetBrush("BlueprintAssist.PlainBorder"))
				.ColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))

			]
			+ SOverlay::Slot()
			.Padding(1)
			[
				Content
			]
		];
	}
	END_SLATE_FUNCTION_BUILD_OPTIMIZATION

	void OnFilterTextChanged(const FText& InFilterText)
	{
		FilterText = InFilterText;
		FString FilterString = InFilterText.ToString();

		// Trim and sanitized the filter text (so that it more likely matches the action descriptions)
		const FString TrimmedFilterString = FText::TrimPrecedingAndTrailing(InFilterText).ToString();

		// Tokenize the search box text into a set of terms; all of them must be present to pass the filter
		TArray<FString> FilterTerms;
		TrimmedFilterString.ParseIntoArray(FilterTerms, TEXT(" "), true);

		FilteredItems.Empty();

		const bool bRequiresFiltering = FilterTerms.Num() > 0;
		for (int32 ItemIndex = 0; ItemIndex < AllItems.Num(); ++ItemIndex)
		{
			ItemType Item = AllItems[ItemIndex];

			// If we're filtering, search check to see if we need to show this action
			bool bShowAction = true;
			if (bRequiresFiltering)
			{
				FString SearchText = Item->GetSearchText();
				SearchText.ReplaceInline(TEXT(" "), TEXT(""));

				FString EachTermSanitized;
				for (int32 FilterIndex = 0; FilterIndex < FilterTerms.Num() && bShowAction; ++FilterIndex)
				{
					const bool bMatchesTerm = SearchText.Contains(FilterTerms[FilterIndex]);
					bShowAction = bShowAction && bMatchesTerm;
				}
			}

			if (bShowAction)
			{
				FilteredItems.Add(Item);
			}
		}

		if (!TrimmedFilterString.IsEmpty())
		{
			FilteredItems.StableSort([&FilterString](const ItemType ItemA, const ItemType ItemB)
			{
				if (FilterString.Equals(ItemA->GetKeySearchText(), ESearchCase::IgnoreCase))
				{
					return true;
				}

				if (FilterString.Equals(ItemB->GetKeySearchText(), ESearchCase::IgnoreCase))
				{
					return false;
				}

				return ItemA->GetKeySearchText().Len() < ItemB->GetKeySearchText().Len();
			});
		}

		FilteredItemsListView->RequestListRefresh();

		// Make sure the selected suggestion stays within the filtered list
		if (SuggestionIndex >= 0 && FilteredItems.Num() > 0)
		{
			SuggestionIndex = FMath::Clamp<int32>(SuggestionIndex, 0, FilteredItems.Num() - 1);
			MarkActiveSuggestion();
		}
		else
		{
			SuggestionIndex = INDEX_NONE;
		}
	}

	void OnFilterTextCommitted(const FText& InText, ETextCommit::Type CommitInfo)
	{
		if (CommitInfo == ETextCommit::OnEnter)
		{
			SelectFirstItem();
		}
	}

	void OnListItemClicked(ItemType Item)
	{
		SelectItem(Item);
	}

	void SelectItem(ItemType Item)
	{
		FSlateApplication::Get().DismissMenuByWidget(SharedThis(this));
		OnSelectItem.ExecuteIfBound(Item);
	}

	bool SelectFirstItem()
	{
		if (FilteredItems.Num() > 0)
		{
			SelectItem(FilteredItems[0]);
			return true;
		}

		return false;
	}

	void MarkActiveSuggestion()
	{
		if (FilteredItems.IsValidIndex(SuggestionIndex))
		{
			ItemType& ItemToSelect = FilteredItems[SuggestionIndex];
			FilteredItemsListView->SetSelection(ItemToSelect);
			FilteredItemsListView->RequestScrollIntoView(ItemToSelect);
			OnMarkActiveSuggestion.ExecuteIfBound(ItemToSelect);
		}
		else
		{
			FilteredItemsListView->ClearSelection();
		}
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override
	{
		if (KeyEvent.GetKey() == EKeys::Escape)
		{
			FSlateApplication::Get().DismissMenuByWidget(SharedThis(this));
			return FReply::Handled();
		}

		if (KeyEvent.GetKey() == EKeys::Enter)
		{
			TArray<ItemType> SelectedItems;
			FilteredItemsListView->GetSelectedItems(SelectedItems);

			if (SelectedItems.Num() > 0)
			{
				SelectItem(SelectedItems[0]);
				return FReply::Handled();
			}

			if (SelectFirstItem())
			{
				return FReply::Handled();
			}
		}

		const int NumItems = FilteredItems.Num();
		if (NumItems > 0)
		{
			int32 SelectionDelta = 0;

			// move up and down through the filtered node list
			if (KeyEvent.GetKey() == EKeys::Up)
			{
				SelectionDelta = -1;
			}
			else if (KeyEvent.GetKey() == EKeys::Down)
			{
				SelectionDelta = 1;
			}

			if (SelectionDelta != 0)
			{
				// If we have no selected suggestion then we need to use the items in the root to set the selection and set the focus
				if (SuggestionIndex == INDEX_NONE)
				{
					SuggestionIndex = SelectionDelta == 1 ? 0 : NumItems - 1;
					MarkActiveSuggestion();
					return FReply::Handled();
				}

				//Move up or down one, wrapping around
				SuggestionIndex = (SuggestionIndex + SelectionDelta + NumItems) % NumItems;
				MarkActiveSuggestion();

				return FReply::Handled();
			}
		}

		return FReply::Unhandled();
	}

	EActiveTimerReturnType SetFocusPostConstruct(double InCurrentTime, float InDeltaTime)
	{
		if (FilterTextBox.IsValid())
		{
			FWidgetPath WidgetToFocusPath;
			FSlateApplication::Get().GeneratePathToWidgetUnchecked(FilterTextBox.ToSharedRef(), WidgetToFocusPath);
			FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
			WidgetToFocusPath.GetWindow()->SetWidgetToFocusOnActivate(FilterTextBox);

			return EActiveTimerReturnType::Stop;
		}

		return EActiveTimerReturnType::Continue;
	}

	ItemType GetSuggestedItem()
	{
		return FilteredItems.IsValidIndex(SuggestionIndex) ? FilteredItems[SuggestionIndex] : nullptr;
	}

	FText GetFilterText()
	{
		return FilterText;
	}
};
