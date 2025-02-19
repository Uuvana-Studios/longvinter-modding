// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintAssistOpenFileMenu.h"

#include "Editor.h"
#include "SlateOptMacros.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Modules/ModuleManager.h"
#include "Stats/StatsMisc.h"
#include "Subsystems/AssetEditorSubsystem.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SBAOpenFileMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FBAFileItem>>)
		.InitListItems(this, &SBAOpenFileMenu::InitListItems)
		.OnGenerateRow(this, &SBAOpenFileMenu::CreateItemWidget)
		.OnSelectItem(this, &SBAOpenFileMenu::SelectItem)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Blueprint Assist Hotkeys"))
	];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SBAOpenFileMenu::InitListItems(TArray<TSharedPtr<FBAFileItem>>& Items)
{
	IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();

	TArray<FAssetData> OutAssetData;

	double ThisTime = 0;
	{
		SCOPE_SECONDS_COUNTER(ThisTime);
		AssetRegistry.GetAllAssets(OutAssetData, false);
	}
	UE_LOG(LogTemp, Warning, TEXT("Get all assets took %.2f"), ThisTime);

	for (FAssetData& AssetData : OutAssetData)
	{
		if (!AssetData.IsRedirector() && AssetData.IsUAsset())
		{
			Items.Add(MakeShared<FBAFileItem>(AssetData.AssetName.ToString()));
		}
	}
}

TSharedRef<ITableRow> SBAOpenFileMenu::CreateItemWidget(TSharedPtr<FBAFileItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	const FText ItemText = FText::FromString(Item->FilePath);
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(8.0, 4.0))
		.ToolTipText(ItemText)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(ItemText)
				.Font(BA_STYLE_CLASS::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
			]
		];
}

void SBAOpenFileMenu::SelectItem(TSharedPtr<FBAFileItem> Item)
{
	// IAssetRegistry& AssetRegistry = IAssetRegistry::GetChecked();
	// FAssetData Asset = AssetRegistry.GetAssetByObjectPath(FName(*Item->FilePath), true);
	if (!Item->FilePath.IsEmpty())
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Item->FilePath);
	}
}
