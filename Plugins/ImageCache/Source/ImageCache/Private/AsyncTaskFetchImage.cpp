// Copyright Qibo Pang 2022. All Rights Reserved.

#include "AsyncTaskFetchImage.h"
#include "Interfaces/IHttpRequest.h"
#include "Engine/Texture2D.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "ImageCache.h"
#include "Modules/ModuleManager.h"
#include "Misc/ScopeLock.h"
#include "ImageCacheModule.h"
#include "Async/Async.h"


UAsyncTaskFetchImage::UAsyncTaskFetchImage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}

UAsyncTaskFetchImage* UAsyncTaskFetchImage::FetchImage(FString URL)
{
	UAsyncTaskFetchImage* FetchTask = NewObject<UAsyncTaskFetchImage>();
	FetchTask->StartFileRequest(URL);

	return FetchTask;
}


void UAsyncTaskFetchImage::StartFileRequest(FString URL) 
{
	//Check cache.if it already cached, need not to create texture again.
	UCacheItem* CacheItem = UImageCache::GetInstance()->FindFromCache(URL);
	if (CacheItem != nullptr) {
		
		UE_LOG(ImageCacheLog, Log, TEXT("Already cached for: %s"), *URL);
		Async(EAsyncExecution::TaskGraphMainThread, [&, URL]()
		{
			UCacheItem* Item = UImageCache::GetInstance()->FindFromCache(URL);
			if (Item)
			{
				this->OnSuccess.Broadcast(URL, Cast<UCacheImageItem>(Item)->Texture);
			}
			else
			{
				this->OnFail.Broadcast(URL, nullptr);
			}
			this->RemoveFromRoot();
		});
		return;
	}
	
	// Create the Http request and add to pending request list
	UE_LOG(ImageCacheLog, Log, TEXT("Start download Image from: %s"),*URL);

//#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 25
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
//#else
//	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
//#endif

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncTaskFetchImage::HandleFileRequest);
	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();
}

void UAsyncTaskFetchImage::HandleFileRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{

	FString URL = HttpRequest->GetURL();
	RemoveFromRoot();

	if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
	{	
		auto& Data = HttpResponse->GetContent();
		size_t Size = static_cast<size_t>(HttpResponse->GetContentLength());
		
		UCacheImageItem* NewItem = NewObject<UCacheImageItem>();
		NewItem->InitByData(URL, Data);
		if (UImageCache::GetInstance()->AddToCache(NewItem))
		{
			UTexture2DDynamic* Texture = NewItem->Texture;
			OnSuccess.Broadcast(URL, Texture);
			return;
		}
	}

	OnFail.Broadcast(URL,nullptr);

}
