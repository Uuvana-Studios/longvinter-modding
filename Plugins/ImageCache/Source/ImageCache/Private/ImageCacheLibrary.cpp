// Copyright Qibo Pang 2022. All Rights Reserved.

#include "ImageCacheLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "Modules/ModuleManager.h"
#include "ImageCacheModule.h"
#include "AsyncTaskFetchImage.h"

UImageCacheLibrary::UImageCacheLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	;
}

void UImageCacheLibrary::CacheImage(FString URL) 
{
	UAsyncTaskFetchImage::FetchImage(URL);
}

void UImageCacheLibrary::CacheImages(const TArray<FString>& URLs)
{
	for (auto URL : URLs) 
	{
		CacheImage(URL);
	}
}

void UImageCacheLibrary::ClearAllImageCache()
{
	UImageCache::GetInstance()->ClearAll();
}

void UImageCacheLibrary::SetImageCacheExpireTime(int32 ExpireTime)
{
	UImageCache::GetInstance()->SetExpireTime(ExpireTime);
}

int32 UImageCacheLibrary::GetImageCacheExpireTime()
{
	return UImageCache::GetInstance()->GetExpireTime();
}

FString UImageCacheLibrary::GetImageCacheDirectory()
{
	return UImageCache::GetInstance()->GetCacheDirectory();
}

UTexture2DDynamic* UImageCacheLibrary::GetCachedImage(const FString& URL) 
{
	auto CachedItem = UImageCache::GetInstance()->FindFromCache(URL);
	if (CachedItem != nullptr)
	{
		return Cast<UCacheImageItem>(CachedItem)->Texture;
	}
	return nullptr;
}

void UImageCacheLibrary::SetImageCacheMaxSizeGB(float SizeGB)
{
	UImageCache::GetInstance()->SetMaxSizeGB(SizeGB);
}

float UImageCacheLibrary::GetImageCacheMaxSizeGB()
{
	return UImageCache::GetInstance()->GetMaxSizeGB();
}