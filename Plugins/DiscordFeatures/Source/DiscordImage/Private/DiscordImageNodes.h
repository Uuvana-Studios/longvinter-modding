// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordImageManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordImageNodes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFetchResponse, const EDiscordResult, Result, const FDiscordImageHandle&, Handle);

UCLASS()
class UImageFetchProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FOnFetchResponse OnFetched;

	UPROPERTY(BlueprintAssignable)
	FOnFetchResponse OnError;

	virtual void Activate()
	{
		if (!Manager)
		{
			FFrame::KismetExecutionMessage(TEXT("Image Fetch Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
			OnResultInternal(EDiscordResult::InvalidManager, FDiscordImageHandle());
			return;
		}

		Manager->Fetch(Handle, bRefresh, FOnImageFetchedEvent::CreateUObject(this, &UImageFetchProxy::OnResultInternal));
	}


	UFUNCTION(BlueprintCallable, Category = "Discord|Image", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Fetch"))
	static UImageFetchProxy* Fetch(UDiscordImageManager* ImageManager, FDiscordImageHandle Handle, const bool bRefresh)
	{
		UImageFetchProxy* const Proxy = NewObject<UImageFetchProxy>();

		Proxy->Manager	= ImageManager;
		Proxy->Handle	= MoveTemp(Handle);
		Proxy->bRefresh = bRefresh;

		return Proxy;
	}

private:
	UPROPERTY()
	UDiscordImageManager* Manager;
	FDiscordImageHandle Handle;
	bool bRefresh;

	void OnResultInternal(const EDiscordResult Result, const FDiscordImageHandle& ImageHandle)
	{
		(Result == EDiscordResult::Ok ? OnFetched : OnError).Broadcast(Result, ImageHandle);
		SetReadyToDestroy();
	}
};
