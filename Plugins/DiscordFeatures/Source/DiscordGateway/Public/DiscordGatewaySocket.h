// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Tickable.h"
#include "DiscordGatewaySocket.generated.h"

class IWebSocket;
class FJsonValue;

DECLARE_MULTICAST_DELEGATE(FOnGatewaySocketReady);
DECLARE_MULTICAST_DELEGATE(FOnGatewaySocketInvalidSession);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGatewaySocketConnectionError, const FString& /* Reason */);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGatewaySocketClosed, int32 /* StatusCode */, const FString& /* Reason */, bool /* bWasClean */);

DECLARE_MULTICAST_DELEGATE_FourParams(FOnGatewaySocketMessage, const EDiscordGatewayOpCode& /* op */, const TSharedPtr<FJsonValue>& /* d */, const TOptional<int32>& /* s */, const TOptional<FString>& /* t */);


/**
 *	Doesn't follow the API byte order because of UBT.
 *  You can retrieve API byte order with (1 << (int32)Flag).
 **/
UENUM(BlueprintType)
enum class EDiscordGatewayIntents : uint8
{
	GUILDS						= 0x0,
	GUILD_MEMBERS				= 0x1,
	GUILD_BANS					= 0x2,
	GUILD_EMOJIS				= 0x3,
	GUILD_INTEGRATIONS			= 0x4,
	GUILD_WEBHOOKS				= 0x5,
	GUILD_INVITES				= 0x6,
	GUILD_VOICE_STATES			= 0x7,
	GUILD_PRESENCES				= 0x8,
	GUILD_MESSAGES				= 0x9,
	GUILD_MESSAGE_REACTIONS		= 0xA,
	GUILD_MESSAGE_TYPING		= 0xB,
	DIRECT_MESSAGES				= 0xC,
	DIRECT_MESSAGE_REACTIONS	= 0xD,
	DIRECT_MESSAGE_TYPING		= 0xE
};

UENUM(BlueprintType)
enum class EDiscordGatewayActivity : uint8
{
	Game,
	Streaming,
	Listening,
	Custom,
	Competing
};

/**
 *	Doesn't follow the API byte order because of UBT. 
 *  You can retrieve API byte order with (1 << (int32)Flag).
 **/
UENUM(BlueprintType)
enum class EDiscordGatewayActivityFlags : uint8
{
	Instance	= 0x0,
	Join		= 0x1,
	Spectate	= 0x2,
	JoinRequest	= 0x3,
	Sync		= 0x4,
	Play		= 0x5
};

UENUM(BlueprintType)
enum class EDiscordGatewayOpCode : uint8
{
	// An event was dispatched.
	Dispatch = 0,
	// Fired periodically by the client to keep the connection alive.
	Heartbeat = 1, 
	// Starts a new session during the initial handshake.
	Identify = 2,
	// Update the client's presence.
	PresenceUpdate = 3,
	// Used to join / leave or move between voice channels.
	VoiceStateUpdate = 4,
	// Resume a previous session that was disconnected.
	Resume = 6,
	// You should attempt to reconnect and resume immediately.
	Reconnect = 7,
	// Request information about offline guild members in a large guild.
	RequestGuildMembers = 8,
	// The session has been invalidated.You should reconnect and identify / resume accordingly.
	InvalidSession = 9,
	// immediately after connecting, contains the heartbeat_interval to use.
	Hello = 10,
	// Sent in response to receiving a heartbeat to acknowledge that it has been received.
	HeartbeatACK = 11,

	// Unknown op code.
	Unknown = 100
};


USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayActivitySecrets
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Join;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Spectate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Match;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayActivityParty
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int32 CurrentSize = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int32 MaxSize = -1;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayActivityAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString LargeImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString LargeText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString SmallImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString SmallText;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayEmoji
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int64 Id = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	bool bAnimated;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayActivityTimestamps
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int32 Start = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int32 End = -1;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayIdentifyProperties
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString OS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Browser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Device;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayShard
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int64 ShardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int64 NumShards = -1;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayActivity
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	EDiscordGatewayActivity Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Url;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int32 CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FDiscordGatewayActivityTimestamps Timestamps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int64 ApplicationId = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Details;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FDiscordGatewayEmoji Emoji;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FDiscordGatewayActivityParty Party;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FDiscordGatewayActivityAsset Assets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FDiscordGatewayActivitySecrets Secrets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	bool bInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	TArray<EDiscordGatewayActivityFlags> Flags;
};

USTRUCT(BlueprintType)
struct DISCORDGATEWAY_API FDiscordGatewayStatusUpdate
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	int32 Since;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	TArray<FDiscordGatewayActivity> Activities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	FString Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Gateway")
	bool bAfk;
};


UCLASS(BlueprintType)
class DISCORDGATEWAY_API UDiscordGatewaySocket : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
private:
	/**
	 * Event handled internally by the socket.
	*/
	typedef void(UDiscordGatewaySocket::*TGatewayCallback)(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t);

public:
	UDiscordGatewaySocket();

	~UDiscordGatewaySocket();

	/**
	 * Creates a new socket to interact with the Discord Gateway.
	 * @return A new socket.
	*/
	static UDiscordGatewaySocket* CreateSocket();

	/**
	 * Check if the internal WebSocket is connected to the Gateway Server.
	 * @return If the WebSocket is connected to the Gateway Server.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
	UPARAM(DisplayName = "Is Connected") bool IsConnected() const;

	/**
	 * Launch the process to connect to the Discord|Gateway Server.
	 * Automatically handles the Hello command and sets the heartbeat.
	 * @param GatewayVersion The Gateway version you want to use.
	*/
	void ConnectToGateway(const int32 GatewayVersion = 9);

	/**
	 * Send a command to the Gateway server.
	 * Automatically construct the root object as the following:
	 *
	 * {
	 *     "op": %d,
	 *	    "d": %s
	 * }
	 *
	 * @param Op The op code. See https://discord.com/developers/docs/topics/opcodes-and-status-codes#gateway-opcodes for an exhaustive list.
	 * @param Data A valid JSON string. (For example: {"a":"b"}, "abcde" or 1234). It is already wrapped into the "d" field.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
	void SendCommand(const int32 Op, UPARAM(DisplayName = "Data (d)") const FString& Data);
	
	/**
	 * Send data through the socket directly.
	 * @param Data The data to send.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
	void SendRaw(const FString& Data);

	/**
	 * Send the IDENTIFY command to the gateway server. 
	 * You can call this method once the socket fired the `OnSocketReady` event.
	 * @param Token					Authentication token
	 * @param Properties			Connection properties. 
	 * @param Shard					Used for Guild Sharding. See https://discord.com/developers/docs/topics/gateway#sharding.
	 * @param Presence				Presence structure for initial presence information
	 * @param Intents				The Gateway Intents you wish to receive
	 * @param LargeThreshold		Value between 50 and 250, total number of members where the gateway will stop sending offline members in the guild member list.
	 * @param bGuildSubscriptions	Enables dispatching of guild subscription events (presence and typing events).
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway", meta=(AutoCreateRefTerm = "Properties, Shard, Presence, Intents"))
	void SendIdentifyCommand
	(
		const FString&								Token, 
		const FDiscordGatewayIdentifyProperties&	Properties, 
		const FDiscordGatewayShard&					Shard, 
		const FDiscordGatewayStatusUpdate&			Presence,
		const TArray<EDiscordGatewayIntents>&		Intents,
		const int32									LargeThreshold = 50,
		const bool									bGuildSubscriptions = true
	);

	/**
	 * Terminates the WebSocket connection with the Discord Gateway.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway")
	void CloseConnection();

	/**
	 * Event called when you can safely start to send commands to the Gateway.
	*/
	FORCEINLINE FOnGatewaySocketReady& OnSocketReady()
	{
		return OnSocketReadyEvent;
	}

	/**
	 * Event called when you can safely start to send commands to the Gateway.
	*/
	FORCEINLINE FOnGatewaySocketClosed& OnSocketClosed()
	{
		return OnSocketClosedEvent;
	}

	/**
	 * Event called when the server sent 9: InvalidSession. Clients are limited to 1 identify every 5 seconds.
	*/
	FORCEINLINE FOnGatewaySocketInvalidSession& OnSocketInvalidSession()
	{
		return OnSocketInvalidSessionEvent;
	}

	/**
	 * Event called when the server sent an op that is not {0, 1, 9, 10}.
	*/
	FORCEINLINE FOnGatewaySocketMessage& OnSocketMessage()
	{
		return OnSocketMessageEvent;
	}

	/**
	 * Event called when the connection to the Gateway Server failed.
	*/
	FORCEINLINE FOnGatewaySocketConnectionError& OnSocketConnectionError()
	{
		return OnSocketConnectionErrorEvent;
	}

public:
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickableInEditor() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;

private:
	void Send(const FString& Data);

	void OnClosedInternal(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnConnectedInternal();
	void OnConnectionErrorInternal(const FString& Error);
	void OnMessageInternal(const FString& MessageString);

	void OnHeartbeatTick();

	void Event_Unhandled		(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t);
	void Event_Heartbeat		(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t);
	void Event_Hello			(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t);
	void Event_InvalidSession	(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t);

	EDiscordGatewayOpCode IntToOpCode(const int32 OpCode);

private:
	FOnGatewaySocketReady			OnSocketReadyEvent;
	FOnGatewaySocketClosed			OnSocketClosedEvent;
	FOnGatewaySocketInvalidSession	OnSocketInvalidSessionEvent;
	FOnGatewaySocketMessage			OnSocketMessageEvent;
	FOnGatewaySocketConnectionError OnSocketConnectionErrorEvent;

	TSharedPtr<IWebSocket> Socket;

	/**
	 * The events handled by the socket.
	 * Stored as: {OpCode, Callback}.
	*/
	static const TMap<int32, TGatewayCallback> HandledEvents;

	bool bShouldHeartbeat;
	int64 HeartbeatInterval;
	float HeartbeatCooldown;
	int64 SequenceNumber;
};


