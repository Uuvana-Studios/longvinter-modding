// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ImageCacheCommon.h"

#include "AsyncTaskFetchImage.generated.h"

class UTexture2DDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFetchImageDelegate, FString, URL, UTexture2DDynamic*, Texture);

 
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "ImageCache")
class IMAGECACHE_API UAsyncTaskFetchImage : public UBlueprintAsyncActionBase
{
	//GENERATED_BODY()
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "ImageCache")
	static UAsyncTaskFetchImage* FetchImage(FString URL);

	UPROPERTY(BlueprintAssignable)
	FFetchImageDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FFetchImageDelegate OnFail;

private:
	void StartFileRequest(FString URL);

	/** Handles image requests coming from the web */
	void HandleFileRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

};
