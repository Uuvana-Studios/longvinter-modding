// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordNetworkManager.h"
#include "DiscordNetwork.h"
#include "DiscordManagerUtils.h"

DEFINE_LOG_CATEGORY(LogDiscordNetwork);

#define BLUEPRINT_WRAP_FUNCTION(Func) { \
	const auto Res = Func; \
	if (Res != EDiscordResult::Ok) \
	{ \
		Exec = EDiscordOperationBranching::Error; \
	} \
	else \
	{ \
		Exec = EDiscordOperationBranching::Success; \
	} \
	return Res; \
}

/* static */ UDiscordNetworkManager* UDiscordNetworkManager::CreateNetworkManager(UDiscordCore* const DiscordCore)
{
	if (DiscordCore->NetworkManager)
	{
		return Cast<UDiscordNetworkManager>(DiscordCore->NetworkManager);
	}

	UDiscordNetworkManager* const Manager = NewObject<UDiscordNetworkManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->NetworkOnMessage	 .BindUObject(Manager, &UDiscordNetworkManager::OnMessageInternal);
	DiscordCore->NetworkOnRouteUpdate.BindUObject(Manager, &UDiscordNetworkManager::OnRouteUpdateInternal);

	DiscordCore->NetworkManager = Manager;

	return Manager;
}

/* static */ UDiscordNetworkManager* UDiscordNetworkManager::GetNetworkManager(UDiscordCore* const DiscordCore)
{
	if (DiscordCore && DiscordCore->App.network) 
	{
		return CreateNetworkManager(DiscordCore);
	}
	return nullptr;
}

UDiscordNetworkManager::UDiscordNetworkManager()
	: Super()
{
}

UDiscordNetworkManager::~UDiscordNetworkManager()
{
	UE_LOG(LogDiscordNetwork, Log, TEXT("Network Manager destroyed."));
}

FUint64 UDiscordNetworkManager::Blueprint_GetPeerId() const
{
	return GetPeerId();
}

uint64 UDiscordNetworkManager::GetPeerId() const
{
	FRawDiscord::DiscordNetworkPeerId PeerId = 0;
	NETWORK_CALL_RAW_CHECKED(get_peer_id, &PeerId);
	return PeerId;
}

EDiscordResult UDiscordNetworkManager::Blueprint_OpenChannel(const FUint64 PeerId, const uint8 ChannelId, const bool bReliable, EDiscordOperationBranching& Exec) const
{
	BLUEPRINT_WRAP_FUNCTION(OpenChannel(PeerId, ChannelId, bReliable));
}

EDiscordResult UDiscordNetworkManager::OpenChannel(const uint64 PeerId, const uint8 ChannelId, const bool bReliable) const
{
	return FDiscordResult::ToEDiscordResult(NETWORK_CALL_RAW_CHECKED_Ret(open_channel, PeerId, ChannelId, bReliable));
}

EDiscordResult UDiscordNetworkManager::Blueprint_OpenPeer(const FUint64 PeerId, const FString& Route, EDiscordOperationBranching& Exec) const
{
	BLUEPRINT_WRAP_FUNCTION(OpenPeer(PeerId, Route));
}

EDiscordResult UDiscordNetworkManager::OpenPeer(const uint64 PeerId, const FString& Route) const
{

	EDiscordResult Res = EDiscordResult::InternalError;
	if (DiscordCore.IsValid() && DiscordCore->App.network)
	{
		Res = FDiscordResult::ToEDiscordResult(DiscordCore->App.network->open_peer(DiscordCore->App.network, PeerId, TCHAR_TO_ANSI(*Route)));
	}
	return Res;
}

EDiscordResult UDiscordNetworkManager::Blueprint_UpdatePeer(const FUint64 PeerId, const FString& Route, EDiscordOperationBranching& Exec) const
{
	BLUEPRINT_WRAP_FUNCTION(UpdatePeer(PeerId, Route));
}

EDiscordResult UDiscordNetworkManager::UpdatePeer(const uint64 PeerId, const FString& Route) const
{
	return FDiscordResult::ToEDiscordResult(NETWORK_CALL_RAW_CHECKED_Ret(update_peer, PeerId, TCHAR_TO_UTF8(*Route)));
}

EDiscordResult UDiscordNetworkManager::Blueprint_SendMessage(const FUint64 PeerId, const uint8 ChannelId, TArray<uint8> Data, EDiscordOperationBranching& Exec)
{
	BLUEPRINT_WRAP_FUNCTION(SendMessage(PeerId, ChannelId, Data));
}

EDiscordResult UDiscordNetworkManager::SendMessage(const uint64 PeerId, const uint8 ChannelId, TArray<uint8> Data)
{
	return FDiscordResult::ToEDiscordResult(NETWORK_CALL_RAW_CHECKED_Ret(send_message, PeerId, ChannelId, Data.GetData(), Data.Num()));
}

EDiscordResult UDiscordNetworkManager::Blueprint_CloseChannel(const FUint64 PeerId, const uint8 ChannelId, EDiscordOperationBranching& Exec)
{
	BLUEPRINT_WRAP_FUNCTION(CloseChannel(PeerId, ChannelId));
}

EDiscordResult UDiscordNetworkManager::CloseChannel(const uint64 PeerId, const uint8 ChannelId)
{
	return FDiscordResult::ToEDiscordResult(NETWORK_CALL_RAW_CHECKED_Ret(close_channel, PeerId, ChannelId));
}

EDiscordResult UDiscordNetworkManager::Blueprint_ClosePeer(const FUint64 PeerId, EDiscordOperationBranching& Exec)
{
	BLUEPRINT_WRAP_FUNCTION(ClosePeer(PeerId));
}

EDiscordResult UDiscordNetworkManager::ClosePeer(const uint64 PeerId)
{
	return FDiscordResult::ToEDiscordResult(NETWORK_CALL_RAW_CHECKED_Ret(close_peer, PeerId));
}


TStatId UDiscordNetworkManager::GetStatId() const
{
	return TStatId();
}

void UDiscordNetworkManager::Tick(float DeltaTime)
{
	if (DiscordCore.IsValid() && IsValid(DiscordCore.Get()))
	{ 
		const FRawDiscord::EDiscordResult FlushResult = 
			DiscordCore.IsValid() && DiscordCore->App.network ?
			DiscordCore->App.network->flush(DiscordCore->App.network) : FRawDiscord::EDiscordResult::InternalError;

		if (FlushResult != FRawDiscord::EDiscordResult::Ok)
		{
			UE_LOG(LogDiscordNetwork, Warning, TEXT("NetworkManager->Flush(): Result is not `OK` but %d."), (int32)FlushResult);
		}	
	}
}

bool UDiscordNetworkManager::IsTickable() const
{
	return DiscordCore.IsValid();
}

void UDiscordNetworkManager::OnMessageInternal(FRawDiscord::DiscordNetworkPeerId peer_id, FRawDiscord::DiscordNetworkChannelId channel_id, uint8* data, uint32 data_length)
{
	OnMessage.Broadcast(peer_id, channel_id, TArray<uint8>(data, data_length));
}

void UDiscordNetworkManager::OnRouteUpdateInternal(const char* route_data)
{
	OnRouteUpdate.Broadcast(UTF8_TO_TCHAR(route_data));
}

#undef BLUEPRINT_WRAP_FUNCTION
#undef NETWORK_CALL_RAW_CHECKED
	