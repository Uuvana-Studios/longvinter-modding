// Copyright Qibo Pang 2022. All Rights Reserved.

#include "ImageCacheItem.h"
#include "ImageCacheUtil.h"
#include "ImageCacheLibrary.h"
#include "ImageCacheModule.h"

void UCacheItem::InitByCacheIndex(const FString& InURL, const FString& InFileName, int32 InSize, int64 InCreateTime)
{
	URL = InURL;
	FileName = InFileName;
	Size = InSize;
	CreateTime = InCreateTime;

	BeenParsed = false;
}

FString UCacheBinaryItem::GenerateFileName(const FString& InURL)
{
	//use crc as filename
	auto& CharArray = InURL.GetCharArray();
	TArray<TCHAR> OddCharArray, EvenCharArray;
	for (int32 i = 0; i < CharArray.Num(); i++)
	{
		if (i % 2)
			OddCharArray.Add(CharArray[i]);
		else
			EvenCharArray.Add(CharArray[i]);
	}

	uint32 CrcOdd = FCrc::MemCrc32(OddCharArray.GetData(), OddCharArray.GetTypeSize() * OddCharArray.Num());
	uint32 CrcEven = FCrc::MemCrc32(EvenCharArray.GetData(), EvenCharArray.GetTypeSize() * EvenCharArray.Num());
	FString Filename = FString::Printf(TEXT("%010u%010u"), CrcOdd, CrcEven);

	//add extension
	FString Extname = FPaths::GetExtension(InURL, true);
	if (Extname.Len() < 3 || Extname.Len() > 10)
	{
		switch (FileDesc)
		{
		case EFileTypeDesc::BinaryData:
			Extname = TEXT(".bin");
			break;
		case EFileTypeDesc::ImageFile:
			Extname = TEXT(".jpg");
			break;
		}
	}
	return Filename + Extname;
}

void UCacheBinaryItem::InitByData(const FString& InURL, const TArray<uint8>& InData)
{
	URL = InURL;
	Data = InData;

	CreateTime = UImageCacheUtil::GetNowInSeconds();
	FileName = GenerateFileName(URL);
	Size = Data.Num();

	Parse();
}

bool UCacheBinaryItem::LoadCache()
{
	FString Path = UImageCacheLibrary::GetImageCacheDirectory() + FileName;

	Data.Empty();
	if (!FFileHelper::LoadFileToArray(Data, *Path))
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Fail to read file: %s"), *Path);
		return false;
	}

	return Parse();
}

bool UCacheBinaryItem::SaveCache()
{
	if (FileName.Len() == 0)
	{
		FileName = GenerateFileName(URL);
	}

	if (Data.Num() == 0)
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Save image package failed:%s"), *FileName);
		return false;
	}
	
	FString Filepath = UImageCacheLibrary::GetImageCacheDirectory() + FileName;
	if (UImageCacheUtil::EnsureWritableFile(Filepath))
	{
		if (FFileHelper::SaveArrayToFile(Data, *Filepath))
		{
			return true;
		}
		else
		{
			UE_LOG(ImageCacheLog, Error, TEXT("Save image package failed:%s"), *FileName);
		}
	}

	return false;
}

bool UCacheBinaryItem::DeleteCache()
{
	FString Path = UImageCacheLibrary::GetImageCacheDirectory() + FileName;
	FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*Path);

	return true;
}

bool UCacheBinaryItem::IsIndexMatch()
{
	FString Path = UImageCacheLibrary::GetImageCacheDirectory() + FileName;
	if (!FPaths::FileExists(Path))
	{
		return false;
	}

	return true;
}

bool UCacheImageItem::Parse()
{
	if (Data.Num() > 0)
	{
		EImageFormat Format = EImageFormat::BMP;
		Texture = UImageCacheUtil::CreateTextureFromImage(Data.GetData(), Data.Num(), Format);

		BeenParsed = true;
	}

	return BeenParsed;
}
