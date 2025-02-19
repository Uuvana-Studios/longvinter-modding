// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ImageCacheCommon.h"

#include "AsyncTaskQueueFetchImages.generated.h"

class UTexture2DDynamic;

USTRUCT(BlueprintType)
struct FFetchImageItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageCache")
		FString Url;


	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageCache")
	//	EFileTypeDesc CacheFileType;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FFetchImageFinishedDelegate, bool, Succes, FString, URL, UTexture2DDynamic*, Texture, const TArray<FString>&, SucceedItems, const TArray<FString>&, FailedItems);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "ImageCache")
class IMAGECACHE_API UAsyncTaskQueueFetchImages : public UBlueprintAsyncActionBase
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()

public:
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "ImageCache")
		static UAsyncTaskQueueFetchImages* QueueFetchImages(const TArray<FString>& Items);

	UPROPERTY(BlueprintAssignable)
		FFetchImageFinishedDelegate OnOneFinished;

	UPROPERTY(BlueprintAssignable)
		FFetchImageFinishedDelegate OnAllFinished;


private:

	void HandleNext();

	UFUNCTION()
		void HandleFetchImageSucess(const FString& URL, UTexture2DDynamic* Texture);
	UFUNCTION()
		void HandleFetchImageFail(const FString& URL, UTexture2DDynamic* Texture);
	UFUNCTION()
		void HandleFetchImagePackageSucess(const FString& URL, const TArray<UTexture2DDynamic*>& Textures);
	UFUNCTION()
		void HandleFetchImagePackageFail(const FString& URL, const TArray<UTexture2DDynamic*>& Textures);

	TArray<FString> QueueItems;
	TArray<FString> SuccessItems;
	TArray<FString> FailedItems;
	FString CurrentItem;
	
	UPROPERTY()
		UBlueprintAsyncActionBase* CurrentFetchTask;

	FCriticalSection m_CriticalSection;
};
