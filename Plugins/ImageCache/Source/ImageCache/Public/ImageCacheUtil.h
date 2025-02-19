// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "ImageCacheCommon.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"

class UImageCacheUtil
{
public:
	static UTexture2DDynamic* Texture;

	static UTexture2DDynamic* CreateTextureFromImage(const uint8* Data, size_t Size, EImageFormat& OutFormat);

	static bool EnsureWritableFile(const FString& Filename);

	static int64 GetNowInSeconds();
};

UTexture2DDynamic* UImageCacheUtil::Texture = nullptr;