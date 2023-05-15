// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordGatewaySocket.h"
#include "DiscordGatewayNodes.generated.h"

class UDiscordGatewaySocket;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FDiscordGatewaySocketEvent, UDiscordGatewaySocket*, Socket, const EDiscordGatewayOpCode, OpCode, const FString&, Data, const int32, Sequence, const FString&, EventName);

UCLASS()
class UConnectToDiscordGatewayProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:

	/**
	 * Called when the socket is ready and you can send the IDENTIFY command.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordGatewaySocketEvent SocketReady;

	/**
	 * Called when we failed to connect to the Gateway due to network trouble.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordGatewaySocketEvent ConnectionError;

	/**
	 * Called when the connection has been closed.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordGatewaySocketEvent ConnectionClosed;

	/**
	 * Called when the server sent an INVALID_SESSION after an IDENTIFY call.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordGatewaySocketEvent InvalidSession;

	/**
	 * Called when the socket sent a message.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordGatewaySocketEvent OnMessage;

public:
	virtual void Activate();

	
	/**
	 * Connect the underlying WebSocket to Discord's Gateway Server.
	 * @param GatewayVersion The Gateway Version you want to use.
	 * @return A wrapper around a WebSocket to easily interact with the Gateway Server.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Gateway", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Connect To Discord Gateway"))
	static UConnectToDiscordGatewayProxy* ConnectToDiscordGateway(const int32 GatewayVersion = 9);

private:
	void OnSocketReadyInternal();
	void OnSocketConnectionErrorInternal(const FString& Reason);
	void OnSocketInvalidSessionInternal();
	void OnSocketMessageInternal(const EDiscordGatewayOpCode& op, const TSharedPtr<class FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t);
	void OnSocketClosedInternal(int32 StatusCode, const FString& Reason, bool bWasClean);

private:
	int32 GatewayVersion;

	UPROPERTY()
	UDiscordGatewaySocket* Socket;
};
