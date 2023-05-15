// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "Tickable.h"
#include "DiscordManager.h"
#include "DiscordNetworkManager.generated.h"

struct IDiscordNetworkManager;

class UDiscordCore;

enum class EDiscordResult : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDiscordOnMessage, const FUint64&, PeerId, uint8, ChannelId, const TArray<uint8>&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam   (FDiscordOnRouteUpdate, const FString&, NewRoute);

/**
 *	Wrapper around the SDK's Network Manager.
*/
UCLASS(BlueprintType)
class DISCORDNETWORK_API UDiscordNetworkManager : public UDiscordManager, public FTickableGameObject
{
	GENERATED_BODY()

private:
	friend UDiscordCore;

private:
	UDiscordNetworkManager();
	~UDiscordNetworkManager();

	static UDiscordNetworkManager* CreateNetworkManager(UDiscordCore* const DiscordCore);

	void OnMessageInternal(uint64_t peer_id, uint8_t channel_id, uint8_t* data, uint32_t data_length);
	void OnRouteUpdateInternal(const char* route_data);

public:
	// Begin FTickableGameObject
	virtual TStatId GetStatId() const;
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const override;
	// End FTickableGameObject

public:
	/**
	 * Called when we received a message through Discord's Network.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordOnMessage OnMessage;

	/**
	 * Called when our route has been updated.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordOnRouteUpdate OnRouteUpdate;

public:
	/**
	 * @return The Network Manager associated with this core.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Network", BlueprintPure, meta = (CompactNodeTitle = "NETWORK MANAGER", BlueprintAutoCast))
	static UPARAM(DisplayName = "Network Manager") UDiscordNetworkManager* GetNetworkManager(UDiscordCore* const DiscordCore);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Network", DisplayName = "Get Peer ID")
	UPARAM(DisplayName = "Peer Id") FUint64 Blueprint_GetPeerId() const;
	uint64 GetPeerId() const;

	UFUNCTION(BlueprintCallable, Category = "Discord|Network", DisplayName = "Open Channel", meta = (ExpandEnumAsExecs = "Exec"))
	UPARAM(DisplayName = "Result") EDiscordResult Blueprint_OpenChannel(const FUint64 PeerId, const uint8 ChannelId, const bool bReliable, EDiscordOperationBranching& Exec) const;
	EDiscordResult OpenChannel(const uint64 PeerId, const uint8 ChannelId, const bool bReliable) const;

	UFUNCTION(BlueprintCallable, Category = "Discord|Network", DisplayName = "Open Peer", meta = (ExpandEnumAsExecs = "Exec"))
	UPARAM(DisplayName = "Result") EDiscordResult Blueprint_OpenPeer(const FUint64 PeerId, const FString& Route, EDiscordOperationBranching& Exec) const;
	EDiscordResult OpenPeer(const uint64 PeerId, const FString& Route) const;
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Network", DisplayName = "Update Peer", meta = (ExpandEnumAsExecs = "Exec"))
	UPARAM(DisplayName = "Result") EDiscordResult Blueprint_UpdatePeer(const FUint64 PeerId, const FString& Route, EDiscordOperationBranching& Exec) const;
	EDiscordResult UpdatePeer(const uint64 PeerId, const FString& Route) const;
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Network", DisplayName = "Send Message", meta = (ExpandEnumAsExecs = "Exec"))
	UPARAM(DisplayName = "Result") EDiscordResult Blueprint_SendMessage(const FUint64 PeerId, const uint8 ChannelId, TArray<uint8> Data, EDiscordOperationBranching& Exec);
	EDiscordResult SendMessage(const uint64 PeerId, const uint8 ChannelId, TArray<uint8> Data);
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Network", DisplayName = "Close Channel", meta = (ExpandEnumAsExecs = "Exec"))
	UPARAM(DisplayName = "Result") EDiscordResult Blueprint_CloseChannel(const FUint64 PeerId, const uint8 ChannelId, EDiscordOperationBranching& Exec);
	EDiscordResult CloseChannel(const uint64 PeerId, const uint8 ChannelId);
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Network", DisplayName = "Close Peer", meta = (ExpandEnumAsExecs = "Exec"))
	UPARAM(DisplayName = "Result") EDiscordResult Blueprint_ClosePeer(const FUint64 PeerId, EDiscordOperationBranching& Exec);
	EDiscordResult ClosePeer(const uint64 PeerId);

};

