// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordLobbyManager.h"
#include "DiscordLobby.h"
#include "DiscordManagerUtils.h"

class FLobbyCallbackFunctions
{
	FLobbyCallbackFunctions() = delete;
public:
	static void Callback_Result_Lobby_AutoDelete(void* Data, FRawDiscord::EDiscordResult Result, FRawDiscord::DiscordLobby* RawLobby)
	{
		FLobbyCallback* const Cb = (FLobbyCallback*)Data;

		FDiscordLobby Lobby(RawLobby);

		Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result), Lobby);

		delete Cb;
	};
};

FDiscordLobby::FDiscordLobby(FRawDiscord::DiscordLobby* Raw)
	: Secret(Raw->secret)
{
	Capacity = Raw->capacity;
	Id		 = Raw->id;
	bLocked  = Raw->locked;
	OwnerId  = Raw->owner_id;
	Type	 = static_cast<EDiscordLobbyType>(Raw->type);
}

#define LOBBY_CALL_RAW(Func, ...) { \
	if (Raw != nullptr) \
	{ \
		Raw-> Func (Raw, __VA_ARGS__); \
	} \
	else \
	{ \
		UE_LOG(LogDiscordLobby, Warning, TEXT("Called function ") TEXT( # Func ) TEXT(" on an invalid object.")); \
	} \
}

void FDiscordLobbyTransaction::SetType(const EDiscordLobbyType Type)
{
	LOBBY_CALL_RAW(set_type, static_cast<FRawDiscord::EDiscordLobbyType>(Type));
}

void FDiscordLobbyTransaction::SetOwner(const int64 UserId)
{
	LOBBY_CALL_RAW(set_owner, UserId);
}

void FDiscordLobbyTransaction::SetCapacity(const uint32 Capacity)
{
	LOBBY_CALL_RAW(set_capacity, Capacity);
}

void FDiscordLobbyTransaction::SetMetadata(const FString& Key, const FString& Value)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);
	
	FRawDiscord::DiscordMetadataValue RawValue;
	FDiscordManagerUtils::CopyStringToBuffer(Value, RawValue);

	LOBBY_CALL_RAW(set_metadata, RawKey, RawValue);
}

void FDiscordLobbyTransaction::DeleteMetadata(const FString& Key)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);

	LOBBY_CALL_RAW(delete_metadata, RawKey);
}

void FDiscordLobbyTransaction::SetLocked(const bool bLocked)
{
	LOBBY_CALL_RAW(set_locked, bLocked);
}

void FDiscordLobbyMemberTransaction::SetMetadata(const FString& Key, const FString& Value)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);

	FRawDiscord::DiscordMetadataValue RawValue;
	FDiscordManagerUtils::CopyStringToBuffer(Value, RawValue);

	LOBBY_CALL_RAW(set_metadata, RawKey, RawValue);
}

void FDiscordLobbyMemberTransaction::DeleteMetadata(const FString& Key)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);

	LOBBY_CALL_RAW(delete_metadata, RawKey);
}

void FDiscordLobbySearchQuery::Filter(const FString& Key, const EDiscordLobbySearchComparison Comparison, const EDiscordLobbySearchCast Cast, const FString& Value)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);

	const FRawDiscord::EDiscordLobbySearchComparison RawComparison = static_cast<FRawDiscord::EDiscordLobbySearchComparison>(static_cast<int32>(Comparison) - 2);

	const FRawDiscord::EDiscordLobbySearchCast RawCast = static_cast<FRawDiscord::EDiscordLobbySearchCast>(Cast);

	FRawDiscord::DiscordMetadataValue RawValue;
	FDiscordManagerUtils::CopyStringToBuffer(Value, RawValue);

	LOBBY_CALL_RAW(filter, RawKey, RawComparison, RawCast, RawValue);
}

void FDiscordLobbySearchQuery::Sort(const FString& Key, const EDiscordLobbySearchCast Cast, const FString& Value)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);

	FRawDiscord::DiscordMetadataValue RawValue;
	FDiscordManagerUtils::CopyStringToBuffer(Value, RawValue);

	const FRawDiscord::EDiscordLobbySearchCast RawCast = static_cast<FRawDiscord::EDiscordLobbySearchCast>(Cast);

	LOBBY_CALL_RAW(sort, RawKey, RawCast, RawValue);
}

void FDiscordLobbySearchQuery::Limit(const uint32 NewLimit)
{
	LOBBY_CALL_RAW(limit, NewLimit);
}

void FDiscordLobbySearchQuery::Distance(const EDiscordLobbySearchDistance Distance)
{
	LOBBY_CALL_RAW(distance, static_cast<FRawDiscord::EDiscordLobbySearchDistance>(Distance));
}

#undef LOBBY_CALL_RAW

/* static */ UDiscordLobbyManager* UDiscordLobbyManager::GetLobbyManager(UDiscordCore* DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->LobbyManager)
	{
		return Cast<UDiscordLobbyManager>(DiscordCore->LobbyManager);
	}

	UDiscordLobbyManager* const Manager = NewObject<UDiscordLobbyManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->LobbyManager = Manager;

	DiscordCore->LobbyOnDelete			.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyDelete);
	DiscordCore->LobbyOnMemberConnect	.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyMemberConnect);
	DiscordCore->LobbyOnMemberDisconnect.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyMemberDisconnect);
	DiscordCore->LobbyOnMemberUpdate	.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyMemberUpdate);
	DiscordCore->LobbyOnMessage			.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyMessage);
	DiscordCore->LobbyOnNetworkMessage	.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyNetworkMessage);
	DiscordCore->LobbyOnSpeaking		.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbySpeaking);
	DiscordCore->LobbyOnUpdate			.BindUObject(Manager, &UDiscordLobbyManager::Internal_OnLobbyUpdate);

	return Manager;
}

FDiscordLobbyTransaction UDiscordLobbyManager::GetLobbyCreateTransaction()
{
	FRawDiscord::IDiscordLobbyTransaction* Transaction = nullptr;
	FRawDiscord::EDiscordResult Res = LOBBY_CALL_RAW_CHECKED_Ret(get_lobby_create_transaction, &Transaction);
	if (Res != FRawDiscord::EDiscordResult::Ok)
	{
		UE_LOG(LogDiscordLobby, Error, TEXT("GetLobbyCreateTransaction() failed. Code: %d %s."), 
			(int32)Res, FDiscordResult::DiscordResultToString(Res));
	}
	return Transaction;
}

FDiscordLobbyTransaction UDiscordLobbyManager::GetLobbyUpdateTransaction(const int64 LobbyId)
{
	FRawDiscord::IDiscordLobbyTransaction* Transaction = nullptr;
	LOBBY_CALL_RAW_CHECKED(get_lobby_update_transaction, LobbyId, &Transaction);
	return Transaction;
}

FDiscordLobbyMemberTransaction UDiscordLobbyManager::GetMemberUpdateTransaction(const int64 LobbyId, const int64 MemberId)
{
	FRawDiscord::IDiscordLobbyMemberTransaction* Transaction = nullptr;
	LOBBY_CALL_RAW_CHECKED(get_member_update_transaction, LobbyId, MemberId, &Transaction);
	return Transaction;
}

void UDiscordLobbyManager::CreateLobby(const FDiscordLobbyTransaction& Transaction, const FLobbyCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(create_lobby, Transaction.Raw, new FLobbyCallback(Callback), FLobbyCallbackFunctions::Callback_Result_Lobby_AutoDelete);
}

void UDiscordLobbyManager::UpdateLobby(const int64 LobbyId, const FDiscordLobbyTransaction& Transaction, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(update_lobby, LobbyId, Transaction.Raw, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordLobbyManager::DeleteLobby(const int64 LobbyId, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(delete_lobby, LobbyId, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordLobbyManager::ConnectLobby(const int64 LobbyId, const FString& LobbySecret, const FLobbyCallback& Callback)
{
	FRawDiscord::DiscordLobbySecret Secret;
	FDiscordManagerUtils::CopyStringToBuffer(LobbySecret, Secret);

	LOBBY_CALL_RAW_CHECKED(connect_lobby, LobbyId, Secret, new FLobbyCallback(Callback), FLobbyCallbackFunctions::Callback_Result_Lobby_AutoDelete);
}

void UDiscordLobbyManager::ConnectLobbyWithActivitySecret(const FString& ActivitySecret, const FLobbyCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(connect_lobby_with_activity_secret, TCHAR_TO_UTF8(*ActivitySecret), new FLobbyCallback(Callback), FLobbyCallbackFunctions::Callback_Result_Lobby_AutoDelete);
}

FString UDiscordLobbyManager::GetLobbyActivitySecret(const int64 LobbyId)
{
	FRawDiscord::DiscordLobbySecret Secret;
	FMemory::Memzero(Secret);

	LOBBY_CALL_RAW_CHECKED(get_lobby_activity_secret, LobbyId, &Secret);

	return UTF8_TO_TCHAR(Secret);
}

void UDiscordLobbyManager::DisconnectLobby(const int64 LobbyId, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(disconnect_lobby, LobbyId, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

FDiscordLobby UDiscordLobbyManager::GetLobby(const int64 LobbyId)
{
	FRawDiscord::DiscordLobby Raw;
	LOBBY_CALL_RAW_CHECKED(get_lobby, LobbyId, &Raw);
	return FDiscordLobby(&Raw);
}

int32 UDiscordLobbyManager::LobbyMetadataCount(const int64 LobbyId)
{
	int32 Count = 0;
	LOBBY_CALL_RAW_CHECKED(lobby_metadata_count, LobbyId, &Count);
	return Count;
}

FString UDiscordLobbyManager::GetLobbyMetadataKey(const int64 LobbyId, const int32 Index)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FMemory::Memzero(RawKey);
	LOBBY_CALL_RAW_CHECKED(get_lobby_metadata_key, LobbyId, Index, &RawKey);
	return UTF8_TO_TCHAR(RawKey);
}

FString UDiscordLobbyManager::GetLobbyMetadataValue(const int64 LobbyId, const FString& Key)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);
	
	FRawDiscord::DiscordMetadataValue RawValue;
	FMemory::Memzero(RawValue);

	LOBBY_CALL_RAW_CHECKED(get_lobby_metadata_value, LobbyId, RawKey, &RawValue);

	return UTF8_TO_TCHAR(RawValue);
}

int32 UDiscordLobbyManager::MemberCount(const int64 LobbyId)
{
	int32 Count = 0;
	LOBBY_CALL_RAW_CHECKED(member_count, LobbyId, &Count);
	return Count;
}

int64 UDiscordLobbyManager::GetMemberUserId(const int64 LobbyId, const int32 Index)
{
	FRawDiscord::DiscordLobbyId UserId = 0;
	LOBBY_CALL_RAW_CHECKED(get_member_user_id, LobbyId, Index, &UserId);
	return UserId;
}

FDiscordUser UDiscordLobbyManager::GetMemberUser(const int64 LobbyId, const int64 UserId)
{
	FRawDiscord::DiscordUser RawUser;
	FMemory::Memzero(RawUser);

	LOBBY_CALL_RAW_CHECKED(get_member_user, LobbyId, UserId, &RawUser);

	return FDiscordUser(&RawUser);
}

int32 UDiscordLobbyManager::MemberMetadataCount(const int64 LobbyId, const int64 UserId)
{
	int32 Count = 0;
	LOBBY_CALL_RAW_CHECKED(member_metadata_count, LobbyId, UserId, &Count);
	return Count;
}

FString UDiscordLobbyManager::GetMemberMetadataKey(const int64 LobbyId, const int64 UserId, const int32 Index)
{
	FRawDiscord::DiscordMetadataKey Key;
	FMemory::Memzero(Key);

	LOBBY_CALL_RAW_CHECKED(get_member_metadata_key, LobbyId, UserId, Index, &Key);

	return UTF8_TO_TCHAR(Key);
}

FString UDiscordLobbyManager::GetMemberMetadataValue(const int64 LobbyId, const int64 UserId, const FString& Key)
{
	FRawDiscord::DiscordMetadataKey RawKey;
	FDiscordManagerUtils::CopyStringToBuffer(Key, RawKey);

	FRawDiscord::DiscordMetadataValue Value;
	FMemory::Memzero(Value);

	LOBBY_CALL_RAW_CHECKED(get_member_metadata_value, LobbyId, UserId, RawKey, &Value);
	
	return UTF8_TO_TCHAR(Value);
}

void UDiscordLobbyManager::UpdateMember(const int64 LobbyId, const int64 UserId, const FDiscordLobbyMemberTransaction& Transaction, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(update_member, LobbyId, UserId, Transaction.Raw, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordLobbyManager::SendLobbyMessage(const int64 LobbyId, TArray<uint8>& Data, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(send_lobby_message, LobbyId, Data.GetData(), Data.Num(), new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

FDiscordLobbySearchQuery UDiscordLobbyManager::GetSearchQuery()
{
	FRawDiscord::IDiscordLobbySearchQuery* Query = nullptr;
	LOBBY_CALL_RAW_CHECKED(get_search_query, &Query);
	return FDiscordLobbySearchQuery(Query);
}

void UDiscordLobbyManager::Search(const FDiscordLobbySearchQuery& Search, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(search, Search.Raw, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

int32 UDiscordLobbyManager::LobbyCount()
{
	int32 Count = 0;
	LOBBY_CALL_RAW_CHECKED(lobby_count, &Count);
	return Count;
}

int64 UDiscordLobbyManager::GetLobbyId(const int32 Index)
{
	FRawDiscord::DiscordLobbyId Id = 0;
	LOBBY_CALL_RAW_CHECKED(get_lobby_id, Index, &Id);
	return Id;
}

void UDiscordLobbyManager::ConnectVoice(const int64 LobbyId, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(connect_voice, LobbyId, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordLobbyManager::DisconnectVoice(const int64 LobbyId, const FDiscordResultCallback& Callback)
{
	LOBBY_CALL_RAW_CHECKED(disconnect_voice, LobbyId, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordLobbyManager::ConnectNetwork(const int64 LobbyId)
{
	LOBBY_CALL_RAW_CHECKED(connect_network, LobbyId);
}

void UDiscordLobbyManager::DisconnectNetwork(const int64 LobbyId)
{
	LOBBY_CALL_RAW_CHECKED(disconnect_network, LobbyId);
}

void UDiscordLobbyManager::FlushNetwork()
{
	if (DiscordCore.IsValid() && DiscordCore->App.lobbies) 
	{
		DiscordCore->App.lobbies->flush_network(DiscordCore->App.lobbies);
	}
}

void UDiscordLobbyManager::OpenNetworkChannel(const int64 LobbyId, const uint8 ChannelId, const bool bReliable)
{
	LOBBY_CALL_RAW_CHECKED(open_network_channel, LobbyId, ChannelId, bReliable);
}

void UDiscordLobbyManager::SendNetworkMessage(const int64 LobbyId, const int64 UserId, const uint8 ChannelId, TArray<uint8>& Data)
{
	LOBBY_CALL_RAW_CHECKED(send_network_message, LobbyId, UserId, ChannelId, Data.GetData(), Data.Num());
}

void UDiscordLobbyManager::Internal_OnLobbyUpdate(const int64 LobbyId)
{
	OnLobbyUpdate.Broadcast(LobbyId);
}

void UDiscordLobbyManager::Internal_OnLobbyDelete(const int64 LobbyId, const uint32 Reason)
{
	OnLobbyDelete.Broadcast(LobbyId, static_cast<int64>(Reason));
}

void UDiscordLobbyManager::Internal_OnLobbyMemberConnect(const int64 LobbyId, const int64 UserId)
{
	OnMemberConnect.Broadcast(LobbyId, UserId);
}

void UDiscordLobbyManager::Internal_OnLobbyMemberUpdate(const int64 LobbyId, const int64 UserId)
{
	OnMemberUpdate.Broadcast(LobbyId, UserId);
}

void UDiscordLobbyManager::Internal_OnLobbyMemberDisconnect(const int64 LobbyId, const int64 UserId)
{
	OnMemberDisconnect.Broadcast(LobbyId, UserId);
}

void UDiscordLobbyManager::Internal_OnLobbyMessage(const int64 LobbyId, const int64 UserId, uint8* Data, const uint32 Length)
{
	TArray<uint8> Uint8Data(Data, Length);
	OnLobbyMessage.Broadcast(LobbyId, UserId, Uint8Data);
}

void UDiscordLobbyManager::Internal_OnLobbySpeaking(const int64 LobbyId, const int64 UserId, const bool bSpeaking)
{
	OnSpeaking.Broadcast(LobbyId, UserId, bSpeaking);
}

void UDiscordLobbyManager::Internal_OnLobbyNetworkMessage(const int64 LobbyId, const int64 UserId, const uint8 Channel, uint8* Data, const uint32 Length)
{
	TArray<uint8> Uint8Data(Data, Length);
	OnNetworkMessage.Broadcast(LobbyId, UserId, Channel, Uint8Data);
}

