// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistCache.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistModule.h"
#include "BlueprintAssistSettings.h"
#include "Editor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetRegistryState.h"
#include "Core/Public/HAL/PlatformFilemanager.h"
#include "Core/Public/Misc/CoreDelegates.h"
#include "Core/Public/Misc/FileHelper.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EngineSettings/Classes/GeneralProjectSettings.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"
#include "Misc/LazySingleton.h"
#include "Projects/Public/Interfaces/IPluginManager.h"
#include "Stats/StatsMisc.h"

#if BA_UE_VERSION_OR_LATER(5, 0)
#include "UObject/ObjectSaveContext.h"
#endif

#define CACHE_VERSION 2

FBACache& FBACache::Get()
{
	return TLazySingleton<FBACache>::Get();
}

void FBACache::TearDown()
{
	TLazySingleton<FBACache>::TearDown();
}

void FBACache::Init()
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	AssetRegistry.OnFilesLoaded().AddRaw(this, &FBACache::LoadCache);

	FCoreDelegates::OnPreExit.AddRaw(this, &FBACache::SaveCache);

#if BA_UE_VERSION_OR_LATER(5, 0)
	FCoreUObjectDelegates::OnObjectPreSave.AddRaw(this, &FBACache::OnObjectPreSave);
#else
	FCoreUObjectDelegates::OnObjectSaved.AddRaw(this, &FBACache::OnObjectSaved);
#endif
}

void FBACache::LoadCache()
{
	if (!GetDefault<UBASettings>()->bSaveBlueprintAssistCacheToFile)
	{
		return;
	}

	if (bHasLoaded)
	{
		return;
	}

	bHasLoaded = true;

	const auto CachePath = GetCachePath();

	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*CachePath))
	{
		FString FileData;
		FFileHelper::LoadFileToString(FileData, *CachePath);

		if (FJsonObjectConverter::JsonObjectStringToUStruct(FileData, &CacheData, 0, 0))
		{
			UE_LOG(LogBlueprintAssist, Log, TEXT("Loaded blueprint assist node size cache: %s"), *GetCachePath(true));
		}
		else
		{
			UE_LOG(LogBlueprintAssist, Log, TEXT("Failed to load node size cache: %s"), *GetCachePath(true));
		}
	}

	if (CacheData.CacheVersion != CACHE_VERSION)
	{
		// clear the cache if our version doesn't match
		CacheData.PackageData.Empty();

		CacheData.CacheVersion = CACHE_VERSION;
	}

	CleanupFiles();

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	AssetRegistry.OnFilesLoaded().RemoveAll(this);
}

void FBACache::SaveCache()
{
	if (!GetDefault<UBASettings>()->bSaveBlueprintAssistCacheToFile)
	{
		return;
	}

	const FString CachePath = GetCachePath();

	double SaveTime = 0;

	{
		SCOPE_SECONDS_COUNTER(SaveTime);

		// Write data to file
		FString JsonAsString;
		FJsonObjectConverter::UStructToJsonObjectString(CacheData, JsonAsString);
		FFileHelper::SaveStringToFile(JsonAsString, *CachePath);
	}

	UE_LOG(LogBlueprintAssist, Log, TEXT("Saved node cache to %s took %.2fms"), *GetCachePath(true), SaveTime * 1000);
}

void FBACache::DeleteCache()
{
	FString CachePath = GetCachePath();
	CacheData.PackageData.Empty();

	if (FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*CachePath))
	{
		UE_LOG(LogBlueprintAssist, Log, TEXT("Deleted cache file at %s"), *GetCachePath(true));
	}
	else
	{
		UE_LOG(LogBlueprintAssist, Log, TEXT("Delete cache failed: Cache file does not exist or is read-only %s"), *GetCachePath(true));
	}
}

void FBACache::CleanupFiles()
{
	// Get all assets
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	// Get package guids from assets
	TSet<FName> CurrentPackageNames;

#if BA_UE_VERSION_OR_LATER(5, 0)
	TArray<FAssetData> Assets;
	FARFilter Filter;
	AssetRegistry.GetAllAssets(Assets, true);
	for (const FAssetData& Asset : Assets)
	{
		CurrentPackageNames.Add(Asset.PackageName);
	}
#else
	const auto& AssetDataMap = AssetRegistry.GetAssetRegistryState()->GetObjectPathToAssetDataMap();
	for (const TPair<FName, const FAssetData*>& AssetDataPair : AssetDataMap)
	{
		const FAssetData* AssetData = AssetDataPair.Value;
		CurrentPackageNames.Add(AssetData->PackageName);
	}
#endif
	// Remove missing files
	TArray<FName> OldPackageGuids;
	CacheData.PackageData.GetKeys(OldPackageGuids);
	for (FName PackageGuid : OldPackageGuids)
	{
		if (!CurrentPackageNames.Contains(PackageGuid))
		{
			CacheData.PackageData.Remove(PackageGuid);
		}
	}
}

FBAGraphData& FBACache::GetGraphData(UEdGraph* Graph)
{
	UPackage* Package = Graph->GetOutermost();

	FBAPackageData& PackageData = CacheData.PackageData.FindOrAdd(Package->GetFName());

	return PackageData.GraphData.FindOrAdd(Graph->GraphGuid);
}

FString FBACache::GetCachePath(bool bFullPath)
{
	FString PluginDir = IPluginManager::Get().FindPlugin("BlueprintAssist")->GetBaseDir();

	if (bFullPath)
	{
		PluginDir = FPaths::ConvertRelativePathToFull(PluginDir);
	}

	const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>();
	const FGuid& ProjectID = ProjectSettings->ProjectID;

	return PluginDir + "/NodeSizeCache/" + ProjectID.ToString() + ".json";
}

void FBAGraphData::CleanupGraph(UEdGraph* Graph)
{
	if (Graph == nullptr)
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("Tried to cleanup null graph"));
		return;
	}

	TSet<FGuid> CurrentNodes;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		// Collect all node guids from the graph
		CurrentNodes.Add(Node->NodeGuid);

		if (FBANodeData* FoundNode = NodeData.Find(Node->NodeGuid))
		{
			// Collect current pin guids
			TSet<FGuid> CurrentPins;
			for (UEdGraphPin* Pin : Node->Pins)
			{
				CurrentPins.Add(Pin->PinId);
			}

			// Collect cached pin guids
			TArray<FGuid> CachedPinGuids;
			FoundNode->CachedPins.GetKeys(CachedPinGuids);

			// Cleanup missing guids
			for (FGuid PinGuid : CachedPinGuids)
			{
				if (!CurrentPins.Contains(PinGuid))
				{
					FoundNode->CachedPins.Remove(PinGuid);
				}
			}
		}
	}

	// Remove any missing guids from the cached nodes
	TArray<FGuid> CachedNodeGuids;
	NodeData.GetKeys(CachedNodeGuids);

	for (FGuid NodeGuid : CachedNodeGuids)
	{
		if (!CurrentNodes.Contains(NodeGuid))
		{
			NodeData.Remove(NodeGuid);
		}
	}
}

#if BA_UE_VERSION_OR_LATER(5, 0)
void FBACache::OnObjectPreSave(UObject* Object, FObjectPreSaveContext Context)
{
	OnObjectSaved(Object);
}
#endif

void FBACache::OnObjectSaved(UObject* Object)
{
	if (bHasSavedThisFrame)
	{
		return;
	}

	bHasSavedThisFrame = true;

	SaveCache();

	GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateRaw(this, &FBACache::ResetSavedThisFrame));
}