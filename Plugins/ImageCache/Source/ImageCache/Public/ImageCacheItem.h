// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "IImageWrapper.h"
#include "ImageCacheCommon.h"
#include "ImageCacheItem.generated.h"

UCLASS()
class IMAGECACHE_API UCacheItem : public UObject
{
	GENERATED_BODY()

public:
	UCacheItem() {}

	virtual bool LoadCache() { return true; }
	virtual bool SaveCache() { return true; }
	virtual bool DeleteCache() { return true; }

	virtual bool IsIndexMatch() { return true; }
	virtual bool CacheParsed() { return BeenParsed; }

	void InitByCacheIndex(const FString& InURL, const FString& InFileName, int32 InSize, int64 InCreateTime);

public:
	UPROPERTY()
	FString URL;

	UPROPERTY()
	FString FileName;

	UPROPERTY()
	int32 Size = 0;

	UPROPERTY()
	EFileTypeDesc FileDesc = EFileTypeDesc::ImageFile;

	UPROPERTY()
	int64 CreateTime = 0;//in seconds

protected:

	UPROPERTY()
	bool BeenParsed = false;

};

UCLASS()
class IMAGECACHE_API UCacheBinaryItem : public UCacheItem
{
	GENERATED_BODY()

public:
	UCacheBinaryItem() 
	{
		FileDesc = EFileTypeDesc::BinaryData;
	}

	virtual void InitByData(const FString& InURL, const TArray<uint8>& InData);
	virtual bool LoadCache() override;
	virtual bool SaveCache() override;
	virtual bool DeleteCache() override;
	virtual bool IsIndexMatch() override;

	const TArray<uint8>& GetData() { return Data; }

protected:

	virtual bool Parse() { return BeenParsed; }

	virtual FString GenerateFileName(const FString& URL);

	UPROPERTY()
	TArray<uint8> Data;
};

UCLASS()
class IMAGECACHE_API UCacheImageItem : public UCacheBinaryItem
{
	GENERATED_BODY()

public:
	UCacheImageItem() 
	{
		FileDesc = EFileTypeDesc::ImageFile;
	}

	virtual bool Parse() override;
	//virtual bool SaveCache() override;

	UPROPERTY()
	UTexture2DDynamic* Texture = nullptr;

};


