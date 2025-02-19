// Copyright Qibo Pang 2022. All Rights Reserved.

#include "ImageCacheUtil.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "ImageCacheModule.h"
#include "Misc/ScopeLock.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"

void WriteRawToTexture_RenderThread(FTexture2DDynamicResource* TextureResource, const TArray<uint8>& RawData, bool bUseSRGB = true)
{
	check(IsInRenderingThread());

	UE_LOG(ImageCacheLog, Error, TEXT("Ignore this. Needed for fetch image to work"));
	FTexture2DRHIRef TextureRHI = TextureResource->GetTexture2DRHI();

	int32 Width = TextureRHI->GetSizeX();
	int32 Height = TextureRHI->GetSizeY();

	uint32 DestStride = 0;
	uint8* DestData = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

	for (int32 y = 0; y < Height; y++)
	{
		uint8* DestPtr = &DestData[(Height - 1 - y) * DestStride];

		const FColor* SrcPtr = &((FColor*)(RawData.GetData()))[(Height - 1 - y) * Width];
		for (int32 x = 0; x < Width; x++)
		{
			*DestPtr++ = SrcPtr->B;
			*DestPtr++ = SrcPtr->G;
			*DestPtr++ = SrcPtr->R;
			*DestPtr++ = SrcPtr->A;
			SrcPtr++;
		}
	}

	RHIUnlockTexture2D(TextureRHI, 0, false, false);
}


UTexture2DDynamic* UImageCacheUtil::CreateTextureFromImage(const uint8* Data, size_t Size, EImageFormat& OutFormat)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	EImageFormat ImageFormats[] = { EImageFormat::PNG ,EImageFormat::JPEG ,EImageFormat::BMP };
	for(auto Format: ImageFormats)
	{
		TSharedPtr<IImageWrapper> ImageWrapper(ImageWrapperModule.CreateImageWrapper(Format));
		if(!ImageWrapper.IsValid())
			continue;
		if(!ImageWrapper->SetCompressed(Data, Size))
			continue;

		TArray<uint8> RawData;
		const ERGBFormat InFormat = /*IsHTML5Platform() ? ERGBFormat::RGBA :*/ ERGBFormat::BGRA;
		if(ImageWrapper->GetRaw(InFormat, 8, RawData))
		{
			Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
			if (Texture != nullptr)
			{
				Texture->SRGB = true;
				UE_LOG(ImageCacheLog, Error, TEXT("Ignore this. Needed for fetch image to work"));
				Texture->UpdateResource();

				FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->Resource);
				ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
					[TextureResource, RawData](FRHICommandListImmediate& RHICmdList)
					{
						WriteRawToTexture_RenderThread(TextureResource, RawData);
					}
				);

				OutFormat = Format;
				return Texture;
			}
		}
	}

	return nullptr;
}

bool UImageCacheUtil::EnsureWritableFile(const FString& Filename)
{
	FString Directory = FPaths::GetPath(Filename);

	if (!IFileManager::Get().DirectoryExists(*Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory);
	}

	// If the file doesn't exist, we're ok to continue
	if (IFileManager::Get().FileSize(*Filename) == -1)
	{
		return true;
	}
	// If we're allowed to overwrite the file, and we deleted it ok, we can continue
	else if (FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*Filename))
	{
		return true;
	}
	// We can't write to the file
	else
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Failed to write image to '%s'.  - Failed to delete the existed file."), *Filename);
		return false;
	}
}

int64 UImageCacheUtil::GetNowInSeconds()
{
	return FDateTime::Now().GetTicks() / ETimespan::TicksPerSecond;
}
