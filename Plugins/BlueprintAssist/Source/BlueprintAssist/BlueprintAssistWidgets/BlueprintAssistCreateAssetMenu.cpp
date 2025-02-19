// Copyright 2021 fpwong. All Rights Reserved.
#include "BlueprintAssistCreateAssetMenu.h"

#include "AssetToolsModule.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistUtils.h"
#include "ClassIconFinder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Editor/Kismet/Public/SSCSEditor.h"
#include "Factories/Factory.h"
#include "Runtime/SlateCore/Public/Types/SlateEnums.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"

/**********************/
/* FBACreateAssetItem */
/**********************/

class FContentBrowserModule;

FString FBACreateAssetItem::ToString() const
{
	return Factory->GetDisplayName().ToString();
}

void SBACreateAssetMenu::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBAFilteredList<TSharedPtr<FBACreateAssetItem>>)
		.InitListItems(this, &SBACreateAssetMenu::InitListItems)
		.OnGenerateRow(this, &SBACreateAssetMenu::CreateItemWidget)
		.OnSelectItem(this, &SBACreateAssetMenu::SelectItem)
		.WidgetSize(GetWidgetSize())
		.MenuTitle(FString("Create Asset"))
	];
}

void SBACreateAssetMenu::InitListItems(TArray<TSharedPtr<FBACreateAssetItem>>& Items)
{
	AssetTools = &(FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get());
	TArray<UFactory*> Factories = AssetTools->GetNewAssetFactories();
	for (UFactory* Factory : Factories)
	{
		Items.Add(MakeShareable(new FBACreateAssetItem(Factory)));
	}
}

TSharedRef<ITableRow> SBACreateAssetMenu::CreateItemWidget(TSharedPtr<FBACreateAssetItem> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	UFactory* Factory = Item->Factory;
	const FName ClassThumbnailBrushOverride = Factory->GetNewAssetThumbnailOverride();
	const FSlateBrush* ClassThumbnail;

	TWeakPtr<IAssetTypeActions> AssetTypeActions = AssetTools->GetAssetTypeActionsForClass(Factory->GetSupportedClass());

	FLinearColor AssetColor = FLinearColor::White;
	if (AssetTypeActions.IsValid())
	{
		AssetColor = AssetTypeActions.Pin()->GetTypeColor();
	}

	if (ClassThumbnailBrushOverride.IsNone())
	{
		ClassThumbnail = FClassIconFinder::FindThumbnailForClass(Factory->GetSupportedClass());
	}
	else
	{
		// Instead of getting the override thumbnail directly from the editor style here get it from the
		// ClassIconFinder since it may have additional styles registered which can be searched by passing
		// it as a default with no class to search for.
		ClassThumbnail = FClassIconFinder::FindThumbnailForClass(nullptr, ClassThumbnailBrushOverride);
	}

	return
		SNew(STableRow<TSharedPtr<FString>>, OwnerTable).Padding(FMargin(4.0, 4.0))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.WidthOverride(24)
					.HeightOverride(24)
					[
						SNew(SBorder)
						.BorderImage(BA_STYLE_CLASS::Get().GetBrush("AssetThumbnail.AssetBackground"))
						.BorderBackgroundColor(AssetColor.CopyWithNewOpacity(0.3f))
						.Padding(2.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
							.Image(ClassThumbnail)
						]
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				[
					SNew(SBorder)
					.BorderImage(BA_STYLE_CLASS::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(AssetColor)
					.Padding(FMargin(0, FMath::Max(FMath::CeilToFloat(24 * 0.025f), 3.0f), 0, 0))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.FillWidth(1)
			.Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->ToString()))
			]
		];
}

void SBACreateAssetMenu::SelectItem(TSharedPtr<FBACreateAssetItem> Item)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();

	

#if BA_UE_VERSION_OR_LATER(5, 0)
	const FContentBrowserItemPath BrowserPath = ContentBrowser.GetCurrentPath();
	const FString Path = BrowserPath.HasInternalPath() ? ContentBrowser.GetCurrentPath().GetInternalPathString() : FString();
#else
	const FString Path = ContentBrowser.GetCurrentPath();
#endif

	if (!Path.IsEmpty())
	{
		UFactory* Factory = Item->Factory;

		FString DefaultAssetName;
		FString PackageNameToUse;

		AssetTools->CreateUniqueAssetName(Path / Factory->GetDefaultNewAssetName(), FString(), PackageNameToUse, DefaultAssetName);

		UObject* CreatedAsset = AssetTools->CreateAssetWithDialog(DefaultAssetName, Path, Factory->GetSupportedClass(), Factory);
		if (CreatedAsset)
		{
			TArray<FAssetData> Assets;
			Assets.Add(FAssetData(CreatedAsset));
			ContentBrowser.SyncBrowserToAssets(Assets);
		}

		// TODO: it is probably nicer to make a new asset like this but I can't select it to rename after it is created
		// if (Factory)
		// {
		// 	if (Factory->ConfigureProperties())
		// 	{
		// 		FString DefaultAssetName;
		// 		FString PackageNameToUse;
		//
		// 		AssetTools->CreateUniqueAssetName(Path / Factory->GetDefaultNewAssetName(), FString(), PackageNameToUse, DefaultAssetName);
		// 		UObject* NewAsset = AssetTools->CreateAsset(DefaultAssetName, Path, Factory->SupportedClass, Factory);
		// 	}
		// }
	}
}
