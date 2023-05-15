// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordStorageNodes.h"

/* static */ UReadAsyncProxy* UReadAsyncProxy::ReadAsync(UDiscordStorageManager* StorageManager, const FString& Name)
{
	UReadAsyncProxy* const Proxy = NewObject<UReadAsyncProxy>();

	Proxy->Manager = StorageManager;
	Proxy->Name	   = Name;

	return Proxy;
}

void UReadAsyncProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Read Async Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		TArray<uint8> Data;
		OnResult(EDiscordResult::InternalError, Data);
		return;
	}

	Manager->ReadAsync(Name, FReadAsyncCallback::CreateUObject(this, &UReadAsyncProxy::OnResult));
}

void UReadAsyncProxy::OnResult(EDiscordResult Result, TArray<uint8>& Data)
{
	(Result == EDiscordResult::Ok ? OnSuccess : OnError).Broadcast(Result, Data);
	SetReadyToDestroy();
}


/* static */ UReadAsyncPartialProxy* UReadAsyncPartialProxy::ReadAsyncPartial(UDiscordStorageManager* StorageManager, const FString& Name, const FUint64& Offset, const FUint64& Length)
{
	UReadAsyncPartialProxy* const Proxy = NewObject<UReadAsyncPartialProxy>();

	Proxy->Manager = StorageManager;
	Proxy->Name    = Name;
	Proxy->Length  = Length;
	Proxy->Offset  = Offset;

	return Proxy;
}

void UReadAsyncPartialProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Read Async Partial Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		TArray<uint8> Data;
		OnResult(EDiscordResult::InternalError, Data);
		return;
	}

	Manager->ReadAsyncPartial(Name, Offset, Length, FReadAsyncCallback::CreateUObject(this, &UReadAsyncPartialProxy::OnResult));
}

void UReadAsyncPartialProxy::OnResult(EDiscordResult Result, TArray<uint8>& Data)
{
	(Result == EDiscordResult::Ok ? OnSuccess : OnError).Broadcast(Result, Data);
	SetReadyToDestroy();
}


/* static */ UWriteAsyncProxy* UWriteAsyncProxy::WriteAsync(UDiscordStorageManager* StorageManager, const FString& Name, const TArray<uint8>& Data)
{
	UWriteAsyncProxy* const Proxy = NewObject<UWriteAsyncProxy>();

	Proxy->Manager = StorageManager;
	Proxy->Name = Name;
	Proxy->Data = Data;

	return Proxy;
}

void UWriteAsyncProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Write Async Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResult(EDiscordResult::InternalError);
		return;
	}

	Manager->WriteAsync(Name, Data, FDiscordResultCallback::CreateUObject(this, &UWriteAsyncProxy::OnResult));
}

void UWriteAsyncProxy::OnResult(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnSuccess : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

