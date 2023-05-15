// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordImageManager.h"
#include "ImageUtils.h"
#include "DiscordManagerUtils.h"
#include "Engine/Texture2D.h"
#include "DiscordImage.h"
#include "Launch/Resources/Version.h"

FDiscordImageHandle::FDiscordImageHandle(const FRawDiscord::DiscordImageHandle& Raw)
{
	Id   = Raw.id;
	Size = Raw.size;
	Type = static_cast<EDiscordImageType>(Raw.type);
}

FRawDiscord::DiscordImageHandle FDiscordImageHandle::ToRaw() const
{
	FRawDiscord::DiscordImageHandle Raw;
	Raw.id   = Id;
	Raw.size = Size;
	Raw.type = static_cast<FRawDiscord::EDiscordImageType>(Type);
	return Raw;
}

UDiscordImageManager* UDiscordImageManager::GetImageManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->ImageManager)
	{
		return Cast<UDiscordImageManager>(DiscordCore->ImageManager);
	}

	UDiscordImageManager* const Manager = NewObject<UDiscordImageManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->ImageManager = Manager;

	return Manager;
}

void UDiscordImageManager::Fetch(const FDiscordImageHandle& Handle, const bool bRefresh, FOnImageFetchedEvent OnImageFetched)
{
	if (!DiscordCore.IsValid() || !DiscordCore->App.image)
	{
		UE_LOG(LogDiscordImage, Error, TEXT("Passed an invalid manager to Fetch()."));
		OnImageFetched.ExecuteIfBound(EDiscordResult::InvalidManager, {});
		return;
	}

	const auto Callback = [](void* Data, FRawDiscord::EDiscordResult Result, FRawDiscord::DiscordImageHandle Raw)
	{
		if (Result != FRawDiscord::EDiscordResult::Ok)
		{
			UE_LOG(LogDiscordImage, Error, TEXT("Failed to fetch image. Error: %d."), Result);
		}

		FOnImageFetchedEvent* const Cb = (FOnImageFetchedEvent*)Data;
	
		FDiscordImageHandle Handle(Raw);
	
		Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result), Handle);
	
		delete Cb;
	};

	IMAGE_CALL_RAW_CHECKED(fetch, Handle.ToRaw(), bRefresh, new FOnImageFetchedEvent(MoveTemp(OnImageFetched)), Callback);
}

FDiscordImageDimensions UDiscordImageManager::GetDimensions(const FDiscordImageHandle& Handle)
{
	FRawDiscord::DiscordImageDimensions Raw;
	FMemory::Memzero(Raw);
	
	IMAGE_CALL_RAW_CHECKED(get_dimensions, Handle.ToRaw(), &Raw);
	
	FDiscordImageDimensions Dim;

	Dim.Height = static_cast<int64>(Raw.height);
	Dim.Width  = static_cast<int64>(Raw.width);
	
	return Dim;
}

EDiscordResult UDiscordImageManager::GetData(const FDiscordImageHandle& Handle, TArray<uint8>& OutData)
{
	const FDiscordImageDimensions Dimensions = GetDimensions(Handle);
	const int32 Size = Dimensions.Height * Dimensions.Width * 4;

	OutData.Empty(Size);

	OutData.AddUninitialized(Size);
	
	const auto Result = DISCORD_CALL_RAW_CHECKED_Ret(image, get_data, Handle.ToRaw(), OutData.GetData(), Size);

	return FDiscordResult::ToEDiscordResult(Result);
}

UTexture2D* UDiscordImageManager::GetTexture(const FDiscordImageHandle& Handle)
{
	TArray<uint8> RawImage;

	const auto Result = GetData(Handle, RawImage);
	if (Result != EDiscordResult::Ok)
	{
		UE_LOG(LogDiscordImage, Error, TEXT("GetTexture() failed as GetData() returned an error: %s"), FDiscordResult::DiscordResultToString(Result));
		return UTexture2D::CreateTransient(0, 0, PF_R8G8B8A8);;
	}

	if (RawImage.Num() <= 0)
	{
		UE_LOG(LogDiscordImage, Warning, TEXT("GetTexture(): GetData returned 0 byte. Did you forget to call fetch first?"));
		return UTexture2D::CreateTransient(0, 0, PF_R8G8B8A8);
	}

	UTexture2D* const Texture = UTexture2D::CreateTransient(Handle.Size, Handle.Size, PF_R8G8B8A8);

#if ENGINE_MAJOR_VERSION >= 5
	uint8* const MipData = (uint8*)Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
#else
	uint8* const MipData = (uint8*)Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
#endif

	FMemory::Memcpy(MipData, (void*)RawImage.GetData(), FMath::Min(Handle.Size*Handle.Size*4, (int64)RawImage.Num()));

#if ENGINE_MAJOR_VERSION >= 5
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
#else
	Texture->PlatformData->Mips[0].BulkData.Unlock();
#endif

	Texture->UpdateResource();
	
	return Texture;
}

#undef ACTIVITY_CALL_RAW_CHECKED
