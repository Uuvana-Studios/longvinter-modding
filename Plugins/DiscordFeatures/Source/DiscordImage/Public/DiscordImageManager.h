// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordImageManager.generated.h"

UENUM()
enum class EDiscordImageType : uint8
{
	// Image is a user's avatar
	User
};

USTRUCT(BlueprintType)
struct FDiscordImageDimensions
{
	GENERATED_BODY()
public:
	// The width of the image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Activity")
	int64 Width = 0;
	// The height of the image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Activity")
	int64 Height = 0;
};

USTRUCT(BlueprintType)
struct FDiscordImageHandle
{
	GENERATED_BODY()
private:
	friend class UDiscordImageManager;

	FDiscordImageHandle(const FRawDiscord::DiscordImageHandle& Raw);
	FRawDiscord::DiscordImageHandle ToRaw() const;

public:
	FDiscordImageHandle() {};
	// The source of the image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Activity")
	EDiscordImageType Type = EDiscordImageType::User;
	// The id of the user whose avatar you want to get
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Activity")
	int64 Id = 0;
	// The resolution at which you want the image. (16, 32, 64, 128, 256).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Activity")
	int64 Size = 256;
};

DECLARE_DELEGATE_TwoParams(FOnImageFetchedEvent, const EDiscordResult, const FDiscordImageHandle&);

UCLASS()
class DISCORDIMAGE_API UDiscordImageManager : public UDiscordManager
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Image", BlueprintPure, meta = (CompactNodeTitle = "IMAGE MANAGER", BlueprintAutocast))
	static UPARAM(DisplayName = "Image Manager") UDiscordImageManager* GetImageManager(UDiscordCore* DiscordCore);

	/**
	 * Fetches an image.
	 * @param Handle The image handle.
	 * @param bRefresh If we want to refresh the image and not take it from cache.
	 * @param OnImageFetched Callback called when the image has been fetched.
	*/
	void Fetch(const FDiscordImageHandle& Handle, const bool bRefresh, FOnImageFetchedEvent OnImageFetched);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Image")
	FDiscordImageDimensions GetDimensions(const FDiscordImageHandle& Handle);

	/**
	 * Get the image as binary
	 * Be carefull that TArray store length as an int32 where the DiscordSDK uses a uint32.
	 * @param Handle The image handle .
	 * @param OutData The data as binary.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Image")
	EDiscordResult GetData(const FDiscordImageHandle& Handle, UPARAM(DisplayName = "Data") TArray<uint8>& OutData);

	UFUNCTION(BlueprintCallable, Category = "Discord|Image", BlueprintPure)
	UPARAM(DisplayName = "Image") UTexture2D* GetTexture(const FDiscordImageHandle& Handle);

};
