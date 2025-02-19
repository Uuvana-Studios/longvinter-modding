// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ImageCacheSettings.generated.h"


/**
 * 
 */
UCLASS(config = ImageCache, defaultconfig)
class IMAGECACHE_API UImageCacheSettings : public UObject
{
	GENERATED_BODY()

	UImageCacheSettings();

public:

	/** Maximum size of the look-ahead cache (in GB; 0 = no limit; default = 1 GB). */
	UPROPERTY(EditAnywhere, config, Category = ImageCache, meta = (ClampMin = 0))
		float ImageCacheSizeGB;
	
	/** Maximum number of image cache expire time in second (0 = never expire, default = never expire). */
	UPROPERTY(config, EditAnywhere, Category = ImageCache, meta = (ClampMin = 0))
		int32 ImageCacheExpireTime = 0;

	/** Image cache file directory, relative to project saved directory (default = ImageCaches). */
	UPROPERTY(config, EditAnywhere, Category = ImageCache)
		FString CacheFileDirectory;

	static UImageCacheSettings* GetInstance();

	FString GetCacheFileDirectory();

	SIZE_T GetCacheMaxSize();
};
