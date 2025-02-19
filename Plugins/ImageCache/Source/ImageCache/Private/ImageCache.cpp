// Copyright Qibo Pang 2022. All Rights Reserved.

#include "ImageCache.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ImageWriteQueue.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "ImageCacheModule.h"
#include "Misc/ScopeLock.h"
#include "UObject/UObjectIterator.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture2DDynamic.h"
#include "ImageCacheUtil.h"
#include "ImageCacheSettings.h"


UImageCache::UImageCache()
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}

}

UImageCache* UImageCache::GetInstance()
{
	for (TObjectIterator<UImageCache> It; It; ++It)
	{
		return *It;
	}

	UImageCache* instance = NewObject<UImageCache>();
	UE_LOG(ImageCacheLog, Log, TEXT("UImageCache Instance created."));
	instance->Initial();
	return instance;
}

void UImageCache::Initial()
{
	// read cached items
	ReadIndexFile();
}

const static FString FieldName_Array = "Array";
const static FString FieldName_URL = "URL";
const static FString FieldName_FileName = "FileName";
const static FString FieldName_FileDesc = "FileDesc";
const static FString FieldName_Size = "Size";
const static FString FieldName_CreateTime = "CreateTime";

bool UImageCache::ReadIndexFile()
{
	FString JsonString;
	FString Path = GetCacheIndexPath();
	if(!FFileHelper::LoadFileToString(JsonString, *Path))
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Fail to read file: %s"), *Path);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	if(!FJsonSerializer::Deserialize(JsonReader, JsonObject)) 
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Fail to deserialize json reader: %s"), *Path);
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* ArrayData;
	if(!JsonObject->TryGetArrayField(FieldName_Array, ArrayData)) 
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Fail to get array field: %s, %s"), *FieldName_Array, *Path);
		return false;
	}

	for(auto& Item : *ArrayData)
	{
		FString ItemURL, ItemFileName;
		int32 ItemSize = 0;
		int32 ItemFileDesc = 0;
		int64 ItemCreateTime = 0;
		if(!Item->AsObject()->TryGetStringField(FieldName_URL, ItemURL))
			continue;
		if(!Item->AsObject()->TryGetStringField(FieldName_FileName, ItemFileName))
			continue;
		if(!Item->AsObject()->TryGetNumberField(FieldName_Size, ItemSize))
			continue;
		if(!Item->AsObject()->TryGetNumberField(FieldName_FileDesc, ItemFileDesc))
			continue;
		if (!Item->AsObject()->TryGetNumberField(FieldName_FileDesc, ItemCreateTime))
			continue;

		FString ItemFilePath = GetCacheDirectory() + ItemFileName;
		if(!IFileManager::Get().FileExists(*ItemFilePath))
			continue;

		UCacheItem* CacheItem = nullptr;

		EFileTypeDesc FileDesc = EFileTypeDesc(ItemFileDesc);
		switch(FileDesc) 
		{
			case EFileTypeDesc::BinaryData:
				CacheItem = NewObject<UCacheBinaryItem>();
				break;
			case EFileTypeDesc::ImageFile:
				CacheItem = NewObject<UCacheImageItem>();
				break;
		}

		if (CacheItem)
		{
			CacheItem->InitByCacheIndex(ItemURL, ItemFileName, ItemSize, ItemCreateTime);
			
			if (CacheItem->IsIndexMatch())
			{
				CacheMap.Add(ItemURL, CacheItem);
			}
		}
	}

	UE_LOG(ImageCacheLog, Log, TEXT("Read index file: %s"), *Path);

	return true;
}

bool UImageCache::WriteIndexFile() const
{
	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);

	JsonWriter->WriteObjectStart();
	JsonWriter->WriteArrayStart(FieldName_Array);

	TArray<UCacheItem*> Items;
	CacheMap.GenerateValueArray(Items);
	for (auto& Item : Items)
	{
		JsonWriter->WriteObjectStart();
		JsonWriter->WriteValue(FieldName_URL, Item->URL);
		JsonWriter->WriteValue(FieldName_FileName, Item->FileName);
		JsonWriter->WriteValue(FieldName_FileDesc, (int32)(Item->FileDesc));
		JsonWriter->WriteValue(FieldName_Size, Item->Size);
		JsonWriter->WriteValue(FieldName_CreateTime, Item->CreateTime);
		JsonWriter->WriteObjectEnd();
	}

	JsonWriter->WriteArrayEnd();
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();

	FString Path = GetCacheIndexPath();
	if(!FFileHelper::SaveStringToFile(JsonString, *Path)) 
	{
		UE_LOG(ImageCacheLog, Error, TEXT("Fail to write file: %s"), *Path);
		return false;
	}

	return true;
}

bool UImageCache::AddToCache(UCacheItem* Item) 
{
	if (Item && !CheckInCache(Item->URL) && Item->SaveCache())
	{
		RemoveInvalidCache();

		CacheMap.Add(Item->URL, Item);

		WriteIndexFile();

		return true;
	}
	
	return false;
}

UCacheItem* UImageCache::FindFromCache(const FString& URL)
{
	FScopeLock Lock(&CS);
	RemoveInvalidCache();

	if (CheckInCache(URL))
	{
		UCacheItem* Item = CacheMap[URL];
		if (!Item->CacheParsed())
		{
			Item->LoadCache();
		}
		return Item;
	}
	return nullptr;
}

FString UImageCache::GetCacheDirectory() const
{
	return UImageCacheSettings::GetInstance()->GetCacheFileDirectory();
}

FString UImageCache::GetCacheIndexPath() const
{
	return FPaths::Combine(GetCacheDirectory(), TEXT("ImageCacheIndex.json"));
}

int32 UImageCache::GetExpireTime() const 
{ 
	return UImageCacheSettings::GetInstance()->ImageCacheExpireTime;
}

void UImageCache::SetExpireTime(int ExpireT)
{
	FScopeLock Lock(&CS);

	UImageCacheSettings::GetInstance()->ImageCacheExpireTime = ExpireT;
	
	RemoveInvalidCache();
}

void UImageCache::SetMaxSizeGB(float SizeGB)
{
	FScopeLock Lock(&CS);

	UImageCacheSettings::GetInstance()->ImageCacheSizeGB = SizeGB;

	RemoveInvalidCache();
}

float UImageCache::GetMaxSizeGB()
{
	return UImageCacheSettings::GetInstance()->ImageCacheSizeGB;
}

void UImageCache::RemoveInvalidCache()
{
	// File deleted by users
	RemoveMismatchCache();
	// File expired
	RemoveExpiredCache();
	// File out of size
	RemoveOutofSizeCache();
}

void UImageCache::RemoveMismatchCache()
{
	bool bHaveMismatch = false;
	TArray<FString> Keys;
	CacheMap.GenerateKeyArray(Keys);
	for (FString Key : Keys)
	{
		UCacheItem* Item = CacheMap[Key];
		if (Item && !Item->IsIndexMatch())
		{
			Item->DeleteCache();
			CacheMap.Remove(Key);
			bHaveMismatch = true;
		}
	}

	if (bHaveMismatch)
	{
		WriteIndexFile();
	}
}

void UImageCache::RemoveExpiredCache()
{
	bool bHaveExpired = false;
	int64 CurrentTime = UImageCacheUtil::GetNowInSeconds();
	int32 ExpireTime = UImageCacheSettings::GetInstance()->ImageCacheExpireTime;

	TArray<FString> Keys;
	CacheMap.GenerateKeyArray(Keys);
	for (FString Key : Keys)
	{
		UCacheItem* Item = CacheMap[Key];
		if (Item && ExpireTime > 0 && (CurrentTime - Item->CreateTime) >= ExpireTime)
		{
			Item->DeleteCache();
			CacheMap.Remove(Key);
			bHaveExpired = true;
		}
	}

	if (bHaveExpired)
	{
		WriteIndexFile();
	}
}

void UImageCache::RemoveOutofSizeCache()
{
	bool bHaveOutofSize = false;
	int64 MaxSize = UImageCacheSettings::GetInstance()->GetCacheMaxSize();
	// No limit when MaxSize == 0
	if (MaxSize == 0)
	{
		return;
	}
	
	int64 UsedSize = 0;
	for (auto It : CacheMap)
	{
		UsedSize += It.Value->Size;
	}

	if (UsedSize > MaxSize)
	{
		bHaveOutofSize = true;
		auto RemoveOldestOne = [=]()
		{
			int64 OldestCreateTime = MAX_int64;
			int64 OldestFileSize = 0;
			FString OldestFileKey = "";
			for (auto It : CacheMap)
			{
				if (OldestCreateTime > It.Value->CreateTime)
				{
					OldestCreateTime = It.Value->CreateTime;
					OldestFileSize = It.Value->Size;
					OldestFileKey = It.Key;
				}
			}

			if (OldestCreateTime != MAX_int64)
			{
				CacheMap[OldestFileKey]->DeleteCache();
				CacheMap.Remove(OldestFileKey);	
			}
			
			return OldestFileSize;
		};

		while (UsedSize > MaxSize)
		{
			int64 RemovedSize = RemoveOldestOne();
			if (RemovedSize != 0)
			{
				UsedSize -= RemovedSize;
			}
			else
			{
				break;
			}
		}
	}

	if (bHaveOutofSize)
	{
		WriteIndexFile();
	}
}

void UImageCache::ClearAll() 
{
	FScopeLock Lock(&CS);

	if (CacheMap.Num() > 0)
	{
		for (auto It : CacheMap)
		{
			It.Value->DeleteCache();
		}

		CacheMap.Empty();

		WriteIndexFile();
	}
	
	UE_LOG(ImageCacheLog, Log, TEXT("Cache Cleaned"));
}

bool UImageCache::CheckInCache(const FString& URL)
{
	return CacheMap.Contains(URL);
}


