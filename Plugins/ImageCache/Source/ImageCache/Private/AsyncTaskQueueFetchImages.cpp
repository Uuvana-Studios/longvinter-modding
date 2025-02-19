// Copyright Qibo Pang 2022. All Rights Reserved.

#include "AsyncTaskQueueFetchImages.h"
#include "Interfaces/IHttpRequest.h"
#include "Engine/Texture2D.h"
#include "Interfaces/IHttpResponse.h"
#include "HttpModule.h"
#include "ImageCache.h"
#include "Modules/ModuleManager.h"
#include "Misc/ScopeLock.h"
#include "ImageCacheModule.h"
#include "Async/Async.h"
#include "AsyncTaskFetchImage.h"

UAsyncTaskQueueFetchImages::UAsyncTaskQueueFetchImages(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		AddToRoot();
	}
}


UAsyncTaskQueueFetchImages* UAsyncTaskQueueFetchImages::QueueFetchImages(const TArray<FString>& Items)
{
	
	UAsyncTaskQueueFetchImages* FetchTask = NewObject<UAsyncTaskQueueFetchImages>();
	FetchTask->QueueItems = Items;
	FetchTask->HandleNext();
	
	return FetchTask;
}


void UAsyncTaskQueueFetchImages::HandleNext()
{
	if (QueueItems.Num() == 0) 
	{
		Async(EAsyncExecution::TaskGraphMainThread, [&]()
		{
			this->OnAllFinished.Broadcast(true, TEXT(""), nullptr, SuccessItems, FailedItems);
			this->RemoveFromRoot();
		});
		return;
	}

	FScopeLock Lock(&m_CriticalSection);
	CurrentItem = QueueItems.Pop();

	//if (CurrentItem.CacheFileType == EFileTypeDesc::ImageFile) 
	{
		UAsyncTaskFetchImage* FetchTask = UAsyncTaskFetchImage::FetchImage(CurrentItem);
		
		FScriptDelegate SuccessDelegate;
		SuccessDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UAsyncTaskQueueFetchImages, HandleFetchImageSucess));
		FetchTask->OnSuccess.Add(SuccessDelegate);

		FScriptDelegate FailDelegate;
		FailDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UAsyncTaskQueueFetchImages, HandleFetchImageFail));
		FetchTask->OnFail.Add(FailDelegate);

		CurrentFetchTask = FetchTask;
	}
}

void UAsyncTaskQueueFetchImages::HandleFetchImageSucess(const FString& URL, UTexture2DDynamic* Texture) {

	FScopeLock Lock(&m_CriticalSection);
	SuccessItems.Add(CurrentItem);

	Async(EAsyncExecution::TaskGraphMainThread, [this, URL, Texture]()
		{
			this->OnOneFinished.Broadcast(true, URL, Texture, SuccessItems, FailedItems);;
		});
	
	HandleNext();
}

void UAsyncTaskQueueFetchImages::HandleFetchImageFail(const FString& URL, UTexture2DDynamic* Texture) {
	FScopeLock Lock(&m_CriticalSection);
	FailedItems.Add(CurrentItem);
	
	Async(EAsyncExecution::TaskGraphMainThread, [this, URL, Texture]()
		{
			this->OnOneFinished.Broadcast(true, URL, Texture, SuccessItems, FailedItems);;
		});
	HandleNext();
}

void UAsyncTaskQueueFetchImages::HandleFetchImagePackageSucess(const FString& URL, const TArray<UTexture2DDynamic*>& Textures) {
	FScopeLock Lock(&m_CriticalSection);
	SuccessItems.Add(CurrentItem);
	HandleNext();
}

void UAsyncTaskQueueFetchImages::HandleFetchImagePackageFail(const FString& URL, const TArray<UTexture2DDynamic*>& Textures) {
	FScopeLock Lock(&m_CriticalSection);
	FailedItems.Add(CurrentItem);
	HandleNext();
}