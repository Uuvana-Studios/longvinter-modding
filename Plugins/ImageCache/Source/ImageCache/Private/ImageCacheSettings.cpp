// Copyright Qibo Pang 2022. All Rights Reserved.


#include "ImageCacheSettings.h"
#include "Dom/JsonObject.h"	// Json
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <string>

UImageCacheSettings::UImageCacheSettings()
	: ImageCacheSizeGB(1.0f)
	, ImageCacheExpireTime(0)
	, CacheFileDirectory(TEXT("ImageCaches/"))
{
	;
}

UImageCacheSettings* UImageCacheSettings::GetInstance()
{
	return GetMutableDefault<UImageCacheSettings>();
}

FString UImageCacheSettings::GetCacheFileDirectory()
{
	return FPaths::ProjectSavedDir() + CacheFileDirectory;
}

SIZE_T UImageCacheSettings::GetCacheMaxSize()
{
	return ImageCacheSizeGB * 1024 * 1024 * 1024;
}