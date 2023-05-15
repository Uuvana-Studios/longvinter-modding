// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordLobbyNodes.h"

FDiscordLobbyTransaction& UDiscordLobbyLibrary::SetLobbyTransactionProperties(FDiscordLobbyTransaction& Transaction, const EDiscordLobbyType Type, const TMap<FString, FString> Metadata, const int64 UserId, const int64 Capacity, const bool bLocked)
{
	Transaction.SetType(Type);
	Transaction.SetOwner(UserId);
	Transaction.SetLocked(bLocked);
	for (const auto& Data : Metadata)
	{
		Transaction.SetMetadata(Data.Key, Data.Value);
	}
	return Transaction;
}

FDiscordLobbyTransaction& UDiscordLobbyLibrary::SetType(FDiscordLobbyTransaction& Transaction, const EDiscordLobbyType Type)
{
	Transaction.SetType(Type);
	return Transaction;
}

FDiscordLobbyTransaction& UDiscordLobbyLibrary::SetOwner(FDiscordLobbyTransaction& Transaction, const int64 UserId)
{
	Transaction.SetOwner(UserId);
	return Transaction;
}

FDiscordLobbyTransaction& UDiscordLobbyLibrary::SetCapacity(FDiscordLobbyTransaction& Transaction, const int64 Capacity)
{
	Transaction.SetCapacity(Capacity);
	return Transaction;
}

FDiscordLobbyTransaction& UDiscordLobbyLibrary::SetMetadata(FDiscordLobbyTransaction& Transaction, const FString& Key, const FString& Value)
{
	Transaction.SetMetadata(Key, Value);
	return Transaction;
}

FDiscordLobbyTransaction& UDiscordLobbyLibrary::DeleteMetadata(FDiscordLobbyTransaction& Transaction, const FString& Key)
{
	Transaction.DeleteMetadata(Key);
	return Transaction;
}

FDiscordLobbyTransaction& UDiscordLobbyLibrary::SetLocked(FDiscordLobbyTransaction& Transaction, const bool bLocked)
{
	Transaction.SetLocked(bLocked);
	return Transaction;
}

FDiscordLobbySearchQuery& UDiscordLobbyLibrary::Filter(FDiscordLobbySearchQuery& SearchQuery, const FString& Key, const EDiscordLobbySearchComparison Comparison, const EDiscordLobbySearchCast Cast, const FString& Value)
{
	SearchQuery.Filter(Key, Comparison, Cast, Value);
	return SearchQuery;
}

FDiscordLobbySearchQuery& UDiscordLobbyLibrary::Sort(FDiscordLobbySearchQuery& SearchQuery, const FString& Key, const EDiscordLobbySearchCast Cast, const FString& Value)
{
	SearchQuery.Sort(Key, Cast, Value);
	return SearchQuery;
}

FDiscordLobbySearchQuery& UDiscordLobbyLibrary::Limit(FDiscordLobbySearchQuery& SearchQuery, const int64 NewLimit)
{
	SearchQuery.Limit(static_cast<uint32>(NewLimit));
	return SearchQuery;
}

FDiscordLobbySearchQuery& UDiscordLobbyLibrary::Distance(FDiscordLobbySearchQuery& SearchQuery, const EDiscordLobbySearchDistance Distance)
{
	SearchQuery.Distance(Distance);
	return SearchQuery;
}

FDiscordLobbyMemberTransaction& UDiscordLobbyLibrary::Member_SetMetadata(FDiscordLobbyMemberTransaction& Transaction, const FString& Key, const FString& Value)
{
	Transaction.SetMetadata(Key, Value);
	return Transaction;
}

FDiscordLobbyMemberTransaction& UDiscordLobbyLibrary::Member_DeleteMetadata(FDiscordLobbyMemberTransaction& Transaction, const FString& Key)
{
	Transaction.DeleteMetadata(Key);
	return Transaction;
}

#define BROADCAST_CALLBACK(Success, Error) (Result == EDiscordResult::Ok ? Success : Error).Broadcast(Result)

void UBaseLobbyAsyncProxy::Error(const TCHAR* Error)
{
	FFrame::KismetExecutionMessage(Error, ELogVerbosity::Error);
}

UCreateLobbyProxy* UCreateLobbyProxy::CreateLobby(UDiscordLobbyManager* LobbyManager, const FDiscordLobbyTransaction& Transaction)
{
	UCreateLobbyProxy* const Proxy = NewObject<UCreateLobbyProxy>();

	Proxy->LobbyTransaction = Transaction;
	Proxy->Manager			= LobbyManager;

	return Proxy;
}

void UCreateLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to create Lobby: Manager is nullptr."));
		FDiscordLobby Lobby;
		OnResult(EDiscordResult::InternalError, Lobby);
		SetReadyToDestroy();
		return;
	}

	Manager->CreateLobby(LobbyTransaction, FLobbyCallback::CreateUObject(this, &UCreateLobbyProxy::OnResult));
}

void UCreateLobbyProxy::OnResult(EDiscordResult Result, FDiscordLobby& Lobby)
{
	(Result == EDiscordResult::Ok ? OnLobbyCreated : OnError).Broadcast(Result, Lobby);
	SetReadyToDestroy();
}

UUpdateLobbyProxy* UUpdateLobbyProxy::UpdateLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const FDiscordLobbyTransaction& Transaction)
{
	UUpdateLobbyProxy* const Proxy = NewObject<UUpdateLobbyProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->Transaction = Transaction;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UUpdateLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to update Lobby: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->UpdateLobby(LobbyId, Transaction, FDiscordResultCallback::CreateUObject(this, &UUpdateLobbyProxy::OnResult));
}

void UUpdateLobbyProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

UDeleteLobbyProxy* UDeleteLobbyProxy::DeleteLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId)
{
	UDeleteLobbyProxy* const Proxy = NewObject<UDeleteLobbyProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UDeleteLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to delete Lobby: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->DeleteLobby(LobbyId, FDiscordResultCallback::CreateUObject(this, &UDeleteLobbyProxy::OnResult));
}

void UDeleteLobbyProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

UConnectLobbyProxy* UConnectLobbyProxy::ConnectLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const FString& LobbySecret)
{
	UConnectLobbyProxy* const Proxy = NewObject<UConnectLobbyProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->LobbySecret = LobbySecret;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UConnectLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to connect Lobby: Manager is nullptr."));
		FDiscordLobby Lobby;
		OnResult(EDiscordResult::InternalError, Lobby);
		SetReadyToDestroy();
		return;
	}

	Manager->ConnectLobby(LobbyId, LobbySecret, FLobbyCallback::CreateUObject(this, &UConnectLobbyProxy::OnResult));
}

void UConnectLobbyProxy::OnResult(EDiscordResult Result, FDiscordLobby& Lobby)
{
	(Result == EDiscordResult::Ok ? OnLobbyConnected : OnError).Broadcast(Result, Lobby);
	SetReadyToDestroy();
}

UConnectLobbyWithActivitySecretProxy* UConnectLobbyWithActivitySecretProxy::ConnectLobbyWithActivitySecret(UDiscordLobbyManager* LobbyManager, const FString& ActivitySecret)
{
	UConnectLobbyWithActivitySecretProxy* const Proxy = NewObject<UConnectLobbyWithActivitySecretProxy>();

	Proxy->ActivitySecret = ActivitySecret;
	Proxy->Manager		  = LobbyManager;

	return Proxy;
}

void UConnectLobbyWithActivitySecretProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to connect Lobby: Manager is nullptr."));
		FDiscordLobby Lobby;
		OnResult(EDiscordResult::InternalError, Lobby);
		SetReadyToDestroy();
		return;
	}

	Manager->ConnectLobbyWithActivitySecret(ActivitySecret, FLobbyCallback::CreateUObject(this, &UConnectLobbyWithActivitySecretProxy::OnResult));
}

void UConnectLobbyWithActivitySecretProxy::OnResult(EDiscordResult Result, FDiscordLobby& Lobby)
{
	(Result == EDiscordResult::Ok ? OnLobbyConnected : OnError).Broadcast(Result, Lobby);
	SetReadyToDestroy();
}


UDisconnectLobbyProxy* UDisconnectLobbyProxy::DisconnectLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId)
{
	UDisconnectLobbyProxy* const Proxy = NewObject<UDisconnectLobbyProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UDisconnectLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to disconnect Lobby: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->DisconnectLobby(LobbyId, FDiscordResultCallback::CreateUObject(this, &UDisconnectLobbyProxy::OnResult));
}

void UDisconnectLobbyProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

UUpdateMemberLobbyProxy* UUpdateMemberLobbyProxy::UpdateMember(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const int64 UserId, const FDiscordLobbyMemberTransaction& Transaction)
{
	UUpdateMemberLobbyProxy* const Proxy = NewObject<UUpdateMemberLobbyProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->UserId  = UserId;
	Proxy->Transaction = Transaction;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UUpdateMemberLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to update member: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->UpdateMember(LobbyId, UserId, Transaction, FDiscordResultCallback::CreateUObject(this, &UUpdateMemberLobbyProxy::OnResult));
}

void UUpdateMemberLobbyProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}


USendLobbyMessageProxy* USendLobbyMessageProxy::SendLobbyMessage(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const TArray<uint8>& Data)
{
	USendLobbyMessageProxy* const Proxy = NewObject<USendLobbyMessageProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->Message = Data;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void USendLobbyMessageProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to send lobby message: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->SendLobbyMessage(LobbyId, Message, FDiscordResultCallback::CreateUObject(this, &USendLobbyMessageProxy::OnResult));
}

void USendLobbyMessageProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

USearchLobbyProxy* USearchLobbyProxy::Search(UDiscordLobbyManager* LobbyManager, const FDiscordLobbySearchQuery& SearchQuery)
{
	USearchLobbyProxy* const Proxy = NewObject<USearchLobbyProxy>();

	Proxy->SearchQuery = SearchQuery;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void USearchLobbyProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to search lobby: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->Search(SearchQuery, FDiscordResultCallback::CreateUObject(this, &USearchLobbyProxy::OnResult));
}

void USearchLobbyProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

UConnectVoiceProxy* UConnectVoiceProxy::ConnectVoice(UDiscordLobbyManager* LobbyManager, const int64 LobbyId)
{
	UConnectVoiceProxy* const Proxy = NewObject<UConnectVoiceProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UConnectVoiceProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to connect voice: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->ConnectVoice(LobbyId, FDiscordResultCallback::CreateUObject(this, &UConnectVoiceProxy::OnResult));
}

void UConnectVoiceProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

UDisconnectVoiceProxy* UDisconnectVoiceProxy::DisconnectVoice(UDiscordLobbyManager* LobbyManager, const int64 LobbyId)
{
	UDisconnectVoiceProxy* const Proxy = NewObject<UDisconnectVoiceProxy>();

	Proxy->LobbyId = LobbyId;
	Proxy->Manager = LobbyManager;

	return Proxy;
}

void UDisconnectVoiceProxy::Activate()
{
	if (!Manager)
	{
		Error(TEXT("Failed to disconnect voice: Manager is nullptr."));
		OnResult(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->DisconnectVoice(LobbyId, FDiscordResultCallback::CreateUObject(this, &UDisconnectVoiceProxy::OnResult));
}

void UDisconnectVoiceProxy::OnResult(EDiscordResult Result)
{
	BROADCAST_CALLBACK(OnSuccess, OnError);
	SetReadyToDestroy();
}

#undef BROADCAST_CALLBACK
