// Fill out your copyright notice in the Description page of Project Settings.

#include "BAAssetEditorHandlerObject.h"

#include "BABlueprintHandlerObject.h"
#include "BARootObject.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistModule.h"
#include "BlueprintAssistTabHandler.h"
#include "BlueprintAssistToolbar.h"
#include "BlueprintEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"

#if ENGINE_MINOR_VERSION < 24 && ENGINE_MAJOR_VERSION == 4
#include "Toolkits/AssetEditorManager.h"
#endif

UBAAssetEditorHandlerObject::~UBAAssetEditorHandlerObject()
{
	UnbindDelegates();
}

UBAAssetEditorHandlerObject* UBAAssetEditorHandlerObject::Get()
{
	if (UBARootObject* RootObject = FBlueprintAssistModule::Get().GetRootObject())
	{
		return RootObject->GetAssetHandler();
	}

	return nullptr;
}

void UBAAssetEditorHandlerObject::Init()
{
	BindAssetOpenedDelegate();
}

void UBAAssetEditorHandlerObject::Cleanup()
{
	UnbindDelegates();
}

void UBAAssetEditorHandlerObject::Tick()
{
	CheckInvalidAssetEditors();
}

IAssetEditorInstance* UBAAssetEditorHandlerObject::GetEditorFromTab(const TSharedPtr<SDockTab> Tab) const
{
	if (const TWeakObjectPtr<UObject>* FoundAsset = AssetsByTab.Find(Tab))
	{
		if (FoundAsset->IsValid() && FoundAsset->Get()->IsValidLowLevelFast(false))
		{
			if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				return AssetEditorSubsystem->FindEditorForAsset(FoundAsset->Get(), false);
			}
		}
	}

	return nullptr;
}

void UBAAssetEditorHandlerObject::BindAssetOpenedDelegate()
{
	// TODO: OnAssetEditorRequestClose is not being properly called in 4.26, maybe this will work in the future?
	check(GEditor);

#if ENGINE_MINOR_VERSION < 24 && ENGINE_MAJOR_VERSION == 4
	OnAssetOpenedDelegateHandle = FAssetEditorManager::Get().OnAssetOpenedInEditor().AddUObject(this, &UBAAssetEditorHandlerObject::OnAssetOpenedInEditor);
#else
	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OnAssetOpenedInEditor().AddUObject(this, &UBAAssetEditorHandlerObject::OnAssetOpened);
	}
#endif
}

void UBAAssetEditorHandlerObject::UnbindDelegates()
{
	if (!GEditor)
	{
		return;
	}

#if ENGINE_MINOR_VERSION < 24 && ENGINE_MAJOR_VERSION == 4
	FAssetEditorManager::Get().OnAssetOpenedInEditor().RemoveAll(this);
#else
	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OnAssetOpenedInEditor().RemoveAll(this);
	}
#endif
}

void UBAAssetEditorHandlerObject::OnAssetOpened(UObject* Asset, IAssetEditorInstance* AssetEditor)
{
	const FString AssetEditorName = AssetEditor ? AssetEditor->GetEditorName().ToString() : "nullptr";
	check(Asset);
	UE_LOG(LogBlueprintAssist, Log, TEXT("Asset opened %s (%s)"), *Asset->GetName(), *AssetEditorName);

	// Update assets by tab
	if (TSharedPtr<SDockTab> Tab = GetTabForAssetEditor(AssetEditor))
	{
		AssetsByTab.Add(TWeakPtr<SDockTab>(Tab), Asset);

		FBATabHandler::Get().ProcessTab(Tab);
	}

	// apply the toolbar to the newly opened asset
	FBAToolbar::Get().OnAssetOpenedInEditor(Asset, AssetEditor);

	if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
	{
		if (!BlueprintHandlers.Contains(Blueprint->GetBlueprintGuid()))
		{
			BlueprintHandlers.Add(Blueprint->GetBlueprintGuid(), NewObject<UBABlueprintHandlerObject>());
		}

		if (UBABlueprintHandlerObject* Handler = BlueprintHandlers.FindRef(Blueprint->GetBlueprintGuid()))
		{
			Handler->BindBlueprintChanged(Blueprint);
		}
	}
}

void UBAAssetEditorHandlerObject::OnAssetClosed(UObject* Asset)
{
	UE_LOG(LogBlueprintAssist, Log, TEXT("Asset closed %s"), *Asset->GetName());
	if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
	{
		const FGuid& BPGuid = Blueprint->GetBlueprintGuid();
		if (auto FoundHandler = BlueprintHandlers.FindRef(BPGuid))
		{
			FoundHandler->UnbindBlueprintChanged(Blueprint);
		}
	}
}

void UBAAssetEditorHandlerObject::CheckInvalidAssetEditors()
{
	if (!GEditor)
	{
		return;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		return;
	}

	// Remove invalid tabs
	TArray<TWeakPtr<SDockTab>> Tabs;
	AssetsByTab.GetKeys(Tabs);
	const auto IsTabInvalid = [](const TWeakPtr<SDockTab> Tab) { return !Tab.IsValid(); };
	TArray<TWeakPtr<SDockTab>> InvalidTabs = Tabs.FilterByPredicate(IsTabInvalid);
	for (TWeakPtr<SDockTab> Tab : InvalidTabs)
	{
		AssetsByTab.Remove(Tab);
	}

	// Update the open assets
	TArray<UObject*> CurrentOpenAssets = AssetEditorSubsystem->GetAllEditedAssets();
	for (TWeakObjectPtr<UObject> Asset : OpenAssets)
	{
		if (Asset.IsValid() && Asset->IsValidLowLevelFast(false))
		{
			if (!CurrentOpenAssets.Contains(Asset.Get()))
			{
				OnAssetClosed(Asset.Get());
			}
		}
	}

	OpenAssets.Reset(CurrentOpenAssets.Num());
	for (UObject* Asset : CurrentOpenAssets)
	{
		if (Asset != nullptr && Asset->IsValidLowLevelFast(false))
		{
			OpenAssets.Add(Asset);
		}
	}
}

TSharedPtr<SDockTab> UBAAssetEditorHandlerObject::GetTabForAsset(UObject* Asset) const
{
	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		if (IAssetEditorInstance* AssetEditor = AssetEditorSubsystem->FindEditorForAsset(Asset, false))
		{
			return GetTabForAssetEditor(AssetEditor);
		}
	}

	return TSharedPtr<SDockTab>();
}

TSharedPtr<SDockTab> UBAAssetEditorHandlerObject::GetTabForAssetEditor(IAssetEditorInstance* AssetEditor) const
{
	if (AssetEditor)
	{
		TSharedPtr<FTabManager> TabManager = AssetEditor->GetAssociatedTabManager();
		if (TabManager.IsValid())
		{
			return TabManager->GetOwnerTab();
		}
	}

	return TSharedPtr<SDockTab>();
}
