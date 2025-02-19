// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"

#include "ImageCacheLibrary.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FImageCacheDelegate, UTexture2DDynamic*, Texture);

UCLASS()
class IMAGECACHE_API UImageCacheLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = "ImageCache")
		static void CacheImage(FString URL);

	UFUNCTION(BlueprintCallable, Category = "ImageCache")
		static void CacheImages(const TArray<FString>& URLs);

	UFUNCTION(BlueprintCallable, Category = "ImageCache")
		static void ClearAllImageCache();

	UFUNCTION(BlueprintCallable, Blueprintpure, Category = "ImageCache")
		static UTexture2DDynamic* GetCachedImage(const FString& URL);

	// set a negative value means no expire
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Image Cache Expire Time(Second)"), Category = "ImageCache")
		static void SetImageCacheExpireTime(int32 ExpireTime);

	UFUNCTION(BlueprintCallable, Blueprintpure, meta = (DisplayName = "Get Image Cache Expire Time(Second)"), Category = "ImageCache")
		static int32 GetImageCacheExpireTime();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Set Image Cache Max Size(GB)"), Category = "ImageCache")
		static void SetImageCacheMaxSizeGB(float SizeGB);

	UFUNCTION(BlueprintCallable, Blueprintpure, meta = (DisplayName = "Get Image Cache Max Size(GB)"), Category = "ImageCache")
		static float GetImageCacheMaxSizeGB();

	UFUNCTION(BlueprintCallable, Blueprintpure, Category = "ImageCache")
		static FString GetImageCacheDirectory();
};
