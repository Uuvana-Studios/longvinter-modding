// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "ImageCacheCommon.h"
#include "ImageCacheItem.h"
#include "ImageCache.generated.h"


class UTexture2DDynamic;


UCLASS()
class IMAGECACHE_API UImageCache : public UObject
{
	GENERATED_BODY()

public:
	UImageCache();
	//
	void Initial();
	bool ReadIndexFile();
	bool WriteIndexFile() const;

	//
	bool AddToCache(UCacheItem* Item);

	//
	FString GetCacheDirectory() const;
	FString GetCacheIndexPath() const;

	UCacheItem* FindFromCache(const FString& URL);

	void SetExpireTime(int32 ExpireT);
	int32 GetExpireTime() const;
	
	void SetMaxSizeGB(float SizeGB);
	float GetMaxSizeGB();

	void ClearAll();

public:
	static UImageCache* GetInstance();

private:
	bool CheckInCache(const FString& URL);
	
	void RemoveInvalidCache();

	void RemoveMismatchCache();
	void RemoveExpiredCache();
	void RemoveOutofSizeCache();

	UPROPERTY()
		TMap<FString, UCacheItem* > CacheMap;

	FCriticalSection CS;
};

