// Copyright 2021 fpwong. All Rights Reserved.
#include "BlueprintAssistWorkflowModeMenu.h"

#include "AssetToolsModule.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistUtils.h"
#include "IAssetFamily.h"
#include "IContentBrowserSingleton.h"
#include "PersonaModule.h"
#include "BlueprintAssist/BlueprintAssistUtils/BAMiscUtils.h"
#include "Editor/Kismet/Public/SSCSEditor.h"
#include "Runtime/SlateCore/Public/Types/SlateEnums.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

/**********************/
/* FBAWorkflowModeItem */
/**********************/

class FContentBrowserModule;

FString FBAWorkflowModeItem::ToString() const
{
	return Name;
}

void SBAWorkflowModeMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FBAWorkflowModeItem>>)
		.InitListItems(this, &SBAWorkflowModeMenu::InitListItems)
		.OnGenerateRow(this, &SBAWorkflowModeMenu::CreateItemWidget)
		.OnSelectItem(this, &SBAWorkflowModeMenu::SelectItem)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Switch workflow"))
	];
}

void SBAWorkflowModeMenu::InitListItems(TArray<TSharedPtr<FBAWorkflowModeItem>>& Items)
{
	const TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (!ActiveWindow)
	{
		return;
	}

	// Use asset family
	FPersonaModule& PersonaModule = FModuleManager::LoadModuleChecked<FPersonaModule>("Persona");
	if (UObject* ActiveObject = FBAMiscUtils::GetAssetForActiveTab<UObject>())
	{
		TSharedRef<IAssetFamily> AssetFamily = PersonaModule.CreatePersonaAssetFamily(ActiveObject);

		TArray<UClass*> AssetTypes;
		AssetFamily->GetAssetTypes(AssetTypes);
		for (UClass* Class : AssetTypes)
		{
			FAssetData AssetData = AssetFamily->FindAssetOfType(Class);
			if (AssetData.GetAsset() == ActiveObject || !AssetData.GetClass())
			{
				continue;
			}

			Items.Add(MakeShareable(new FBAWorkflowModeItem(EBAWorkflowItemType::AssetShortcut, AssetData, AssetFamily)));
		}
	}

	// Skip checking for mode widgets if we found a valid asset family
	if (Items.Num() > 0)
	{
		return;
	}

	// Read window for SModeWidgets (should not really be used since WBP and BehaviorTrees are handled differently in the InputProcessor)
	TArray<TSharedPtr<SWidget>> ModeWidgets;
	FBAUtils::GetChildWidgets(ActiveWindow, "SModeWidget", ModeWidgets);
	for (TSharedPtr<SWidget> ModeWidget : ModeWidgets)
	{
		TSharedPtr<SWidget> ModeBorder = FBAUtils::GetChildWidget(ModeWidget, "SCheckBox");
		TSharedPtr<STextBlock> ModeTextWidget = FBAUtils::GetChildWidgetCasted<STextBlock>(ModeWidget, "STextBlock");
		if (!ModeBorder || !ModeTextWidget)
		{
			UE_LOG(LogBlueprintAssist, Warning, TEXT("Mode widget is missing a border or text block"));
			continue;
		}
	
		if (ModeTextWidget.IsValid())
		{
			// The mode widget reacts when we click on its border
			Items.Add(MakeShareable(new FBAWorkflowModeItem(EBAWorkflowItemType::WorkflowMode, ModeBorder, ModeTextWidget->GetText().ToString())));
		}
	}
}

TSharedRef<ITableRow> SBAWorkflowModeMenu::CreateItemWidget(TSharedPtr<FBAWorkflowModeItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	if (Item->Type == EBAWorkflowItemType::AssetShortcut)
	{
		TSharedRef<SHorizontalBox> Content = SNew(SHorizontalBox);

#if BA_UE_VERSION_OR_LATER(5, 0)
		UClass* Class = Item->AssetData.GetClass();
		const FSlateBrush* Icon = Item->AssetFamily->GetAssetTypeDisplayIcon(Class);
		const FSlateColor Tint = Item->AssetFamily->GetAssetTypeDisplayTint(Class);

		Content->AddSlot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(FMargin(4.0, 2.0))
		[
			SNew(SImage).Image(Icon).ColorAndOpacity(Tint)
		];
#endif

		Content->AddSlot().AutoWidth().HAlign(HAlign_Left).VAlign(VAlign_Center).FillWidth(1)
		[
			SNew(STextBlock).Text(FText::FromName(Item->AssetData.AssetClass))
		];

		return
			SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(4.0, 2.0))
			[
				Content
			];
	}

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

void SBAWorkflowModeMenu::SelectItem(TSharedPtr<FBAWorkflowModeItem> Item)
{
	switch (Item->Type)
	{
		case EBAWorkflowItemType::WorkflowMode:
			FBAUtils::InteractWithWidget(Item->InteractWidget);
			break;
		case EBAWorkflowItemType::AssetShortcut:
			if (UObject* AssetObject = Item->AssetData.GetAsset())
			{
				TArray<UObject*> Assets;
				Assets.Add(AssetObject);
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAssets(Assets);
			}
			break;
		default: ;
	}
}
