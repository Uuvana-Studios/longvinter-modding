// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SGraphPin.h"
#include "BlueprintAssistGlobals.h"

#include "BlueprintAssistCache.generated.h"

USTRUCT()
struct BLUEPRINTASSIST_API FBANodeData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector2D CachedNodeSize;

	UPROPERTY()
	TMap<FGuid, float> CachedPins; // pin guid -> pin offset
};

USTRUCT()
struct BLUEPRINTASSIST_API FBAGraphData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TMap<FGuid, FBANodeData> NodeData; // node guid -> node data

	void CleanupGraph(UEdGraph* Graph);
};

USTRUCT()
struct BLUEPRINTASSIST_API FBAPackageData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TMap<FGuid, FBAGraphData> GraphData; // graph guid -> graph data
};

USTRUCT()
struct BLUEPRINTASSIST_API FBACacheData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TMap<FName, FBAPackageData> PackageData; // package name -> package data

	UPROPERTY()
	TArray<FString> BookmarkedFolders;

	UPROPERTY()
	int CacheVersion = -1;
};

class BLUEPRINTASSIST_API FBACache
{
public:
	static FBACache& Get();
	static void TearDown();

	void Init();

	FBACacheData& GetCacheData() { return CacheData; }

	void LoadCache();

	void SaveCache();

	void DeleteCache();

	void CleanupFiles();

	FBAGraphData& GetGraphData(UEdGraph* Graph);

	FString GetCachePath(bool bFullPath = false);

	void SetBookmarkedFolder(const FString& FolderPath, int Index)
	{
		if (Index >= CacheData.BookmarkedFolders.Num())
		{
			CacheData.BookmarkedFolders.SetNum(Index + 1);
		}

		CacheData.BookmarkedFolders[Index] = FolderPath;
	}

	TOptional<FString> FindBookmarkedFolder(int Index)
	{
		return CacheData.BookmarkedFolders.IsValidIndex(Index) ? CacheData.BookmarkedFolders[Index] : TOptional<FString>();
	}

private:
	bool bHasLoaded = false;

	FBACacheData CacheData;

	bool bHasSavedThisFrame = false;

#if BA_UE_VERSION_OR_LATER(5, 0)
	void OnObjectPreSave(UObject* Object, FObjectPreSaveContext Context);
#endif

	void OnObjectSaved(UObject* Object);

	void ResetSavedThisFrame() { bHasSavedThisFrame = false; }
};
