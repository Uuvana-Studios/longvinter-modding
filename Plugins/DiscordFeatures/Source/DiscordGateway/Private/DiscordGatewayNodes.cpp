// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordGatewayNodes.h"
#include "DiscordGatewaySocket.h"

#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

UConnectToDiscordGatewayProxy* UConnectToDiscordGatewayProxy::ConnectToDiscordGateway(const int32 GatewayVersion)
{
	UConnectToDiscordGatewayProxy* const Proxy = NewObject<UConnectToDiscordGatewayProxy>();

	Proxy->Socket		  = NewObject<UDiscordGatewaySocket>();
	Proxy->GatewayVersion = GatewayVersion;

	return Proxy;
}

void UConnectToDiscordGatewayProxy::Activate()
{
	Socket->OnSocketReady()				.AddUObject(this, &UConnectToDiscordGatewayProxy::OnSocketReadyInternal);
	Socket->OnSocketConnectionError()	.AddUObject(this, &UConnectToDiscordGatewayProxy::OnSocketConnectionErrorInternal);
	Socket->OnSocketInvalidSession()	.AddUObject(this, &UConnectToDiscordGatewayProxy::OnSocketInvalidSessionInternal);
	Socket->OnSocketMessage()			.AddUObject(this, &UConnectToDiscordGatewayProxy::OnSocketMessageInternal);
	Socket->OnSocketClosed()			.AddUObject(this, &UConnectToDiscordGatewayProxy::OnSocketClosedInternal);

	Socket->ConnectToGateway(GatewayVersion);
}

void UConnectToDiscordGatewayProxy::OnSocketReadyInternal()
{
	SocketReady.Broadcast(Socket, EDiscordGatewayOpCode::Unknown, TEXT(""), -1, TEXT(""));
}

void UConnectToDiscordGatewayProxy::OnSocketConnectionErrorInternal(const FString& Reason)
{
	ConnectionError.Broadcast(Socket, EDiscordGatewayOpCode::Unknown, Reason, -1, TEXT(""));
	SetReadyToDestroy();
}

void UConnectToDiscordGatewayProxy::OnSocketInvalidSessionInternal()
{
	InvalidSession.Broadcast(Socket, EDiscordGatewayOpCode::InvalidSession, TEXT("false"), -1, TEXT(""));
}

void UConnectToDiscordGatewayProxy::OnSocketMessageInternal(const EDiscordGatewayOpCode& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t)
{
	FString StringifiedJson;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR> > > Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StringifiedJson);
	FJsonSerializer::Serialize(d.ToSharedRef(), TEXT(""), Writer);
	OnMessage.Broadcast(Socket, op, StringifiedJson, s.Get(-1), t.Get(TEXT("")));
}

void UConnectToDiscordGatewayProxy::OnSocketClosedInternal(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	OnMessage.Broadcast(Socket, EDiscordGatewayOpCode::Unknown, Reason, StatusCode, TEXT(""));
	SetReadyToDestroy();
}
