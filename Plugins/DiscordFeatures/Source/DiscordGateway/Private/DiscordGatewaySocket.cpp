// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordGatewaySocket.h"
#include "DiscordGateway.h"
#include "WebSocketsModule.h" 
#include "IWebSocket.h"

#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"

#include "Engine/World.h"
#include "TimerManager.h"

const TMap<int32, UDiscordGatewaySocket::TGatewayCallback> UDiscordGatewaySocket::HandledEvents =
{ 
	// { EventCode, &UDiscordGatewaySocket::CalledMethodName }

	{ 0x1, &UDiscordGatewaySocket::Event_Heartbeat },
	{ 0x9, &UDiscordGatewaySocket::Event_InvalidSession },
	{ 0xA, &UDiscordGatewaySocket::Event_Hello }
};

UDiscordGatewaySocket* UDiscordGatewaySocket::CreateSocket()
{
	return NewObject<UDiscordGatewaySocket>();
}

UDiscordGatewaySocket::UDiscordGatewaySocket()
	: bShouldHeartbeat(false)
	, HeartbeatInterval(0)
	, HeartbeatCooldown(0.f)
	, SequenceNumber(0)
{}

UDiscordGatewaySocket::~UDiscordGatewaySocket()
{
	if (IsConnected())
	{
		Socket->Close();
	}
}


bool UDiscordGatewaySocket::IsTickableWhenPaused() const
{
	return bShouldHeartbeat;
}

bool UDiscordGatewaySocket::IsTickableInEditor() const
{
	return bShouldHeartbeat;
}

void UDiscordGatewaySocket::Tick(float DeltaTime)
{
	if (bShouldHeartbeat)
	{
		HeartbeatCooldown -= DeltaTime;
		if (HeartbeatCooldown <= 0.f)
		{
			OnHeartbeatTick();
			HeartbeatCooldown = HeartbeatInterval / 1000.f;
		}
	}
}

TStatId UDiscordGatewaySocket::GetStatId() const
{
	return TStatId();
}

bool UDiscordGatewaySocket::IsTickable() const
{
	return bShouldHeartbeat;
}


bool UDiscordGatewaySocket::IsConnected() const
{
	return Socket.IsValid() ? Socket->IsConnected() : false;
}

void UDiscordGatewaySocket::Send(const FString& Data)
{
	if (!IsConnected())
	{
		UE_LOG(LogDiscordGateway, Warning, TEXT("Send() called on a socket that is not connected to the Gateway."));
		return;
	}

	Socket->Send(Data);
}

void UDiscordGatewaySocket::SendRaw(const FString& Data)
{
	Send(Data);
}

void UDiscordGatewaySocket::SendCommand(const int32 Op, const FString& Data)
{
	Send(FString::Printf
	(
		TEXT("{")
		TEXT(	"\"op\":%d,")
		TEXT(	"\"d\":%s")
		TEXT("}"),
		Op, *Data
	));
}

void UDiscordGatewaySocket::OnHeartbeatTick()
{	
	Send(FString::Printf(TEXT("{\"op\":1,\"d\":\"%d\"}"), SequenceNumber));

	UE_LOG(LogDiscordGateway, Log, TEXT("Heartbeat tick."));
}

void UDiscordGatewaySocket::SendIdentifyCommand
(
	const FString&								Token, 
	const FDiscordGatewayIdentifyProperties&	Properties, 
	const FDiscordGatewayShard&					Shard, 
	const FDiscordGatewayStatusUpdate&			Presence,
	const TArray<EDiscordGatewayIntents>&		Intents,
	const int32									LargeThreshold,
	const bool									bGuildSubscriptions
)
{
	const int32 IdentifyOp = 2;

	TSharedPtr<FJsonObject> RootObject			= MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> DataObject			= MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PropertiesObject	= MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> PresenceObject		= MakeShared<FJsonObject>();

	RootObject->SetNumberField(TEXT("op"), IdentifyOp);
	RootObject->SetObjectField(TEXT("d"), DataObject);

	DataObject->SetStringField(TEXT("token"), Token);
	DataObject->SetObjectField(TEXT("properties"), PropertiesObject);

	if (!Properties.OS.IsEmpty())
	{
		PropertiesObject->SetStringField(TEXT("os"), Properties.OS);
	}
	
	if (!Properties.Browser.IsEmpty())
	{
		PropertiesObject->SetStringField(TEXT("browser"), Properties.Browser);
	}

	if (!Properties.Device.IsEmpty())
	{
		PropertiesObject->SetStringField(TEXT("device"), Properties.Device);
	}

	DataObject->SetNumberField(TEXT("large_threshold"), LargeThreshold);

	if (Shard.NumShards >= 0)
	{
		TArray<TSharedPtr<FJsonValue>> ShardArray;
		ShardArray.Reserve(2);

		ShardArray.Add(MakeShared<FJsonValueNumber>(Shard.ShardId));
		ShardArray.Add(MakeShared<FJsonValueNumber>(Shard.NumShards));

		DataObject->SetArrayField(TEXT("shard"), ShardArray);
	}

	if (Presence.Activities.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ActivitesObject;

		ActivitesObject.Reserve(Presence.Activities.Num());

		for (const FDiscordGatewayActivity& Activity : Presence.Activities)
		{
			TSharedPtr<FJsonValueObject> ActivityValueObject = MakeShared<FJsonValueObject>(MakeShared<FJsonObject>());
			TSharedPtr<FJsonObject>		 ActivityObject		 = ActivityValueObject->AsObject();

			if (Activity.ApplicationId != -1)
			{
				ActivityObject->SetNumberField(TEXT("application_id"), Activity.ApplicationId);
			}

			ActivityObject->SetStringField(TEXT("Name"), Activity.Name);
			ActivityObject->SetNumberField(TEXT("type"), static_cast<int32>(Activity.Type));

			if (!Activity.Url.IsEmpty())
			{
				ActivityObject->SetStringField(TEXT("url"), Activity.Url);
			}

			ActivityObject->SetNumberField(TEXT("created_at"), Activity.CreatedAt);

			if (Activity.Timestamps.End != 1 && Activity.Timestamps.Start != -1)
			{
				TSharedPtr<FJsonObject> TimestampsObject = MakeShared<FJsonObject>();

				TimestampsObject->SetNumberField(TEXT("start"), Activity.Timestamps.Start);
				TimestampsObject->SetNumberField(TEXT("end"),   Activity.Timestamps.End);
				
				ActivityObject->SetObjectField(TEXT("timestamps"), TimestampsObject);
			}

			if (Activity.ApplicationId != -1)
			{
				ActivityObject->SetNumberField(TEXT("application_id"), Activity.ApplicationId);
			}

			if (!Activity.Details.IsEmpty())
			{
				ActivityObject->SetStringField(TEXT("details"), Activity.Details);
			}

			if (!Activity.State.IsEmpty())
			{
				ActivityObject->SetStringField(TEXT("state"), Activity.State);
			}

			// Emoji
			if (!Activity.Emoji.Name.IsEmpty())
			{
				TSharedPtr<FJsonObject> EmojiObject = MakeShared<FJsonObject>();

				EmojiObject->SetStringField(TEXT("name"), Activity.Emoji.Name);
				EmojiObject->SetBoolField(TEXT("animated"), Activity.Emoji.bAnimated);

				if (Activity.Emoji.Id != -1)
				{
					EmojiObject->SetNumberField(TEXT("id"), Activity.Emoji.Id);
				}
				
				ActivityObject->SetObjectField(TEXT("emoji"), EmojiObject);
			}

			// Party
			if (!Activity.Party.Id.IsEmpty())
			{
				TSharedPtr<FJsonObject> PartyObject = MakeShared<FJsonObject>();

				PartyObject->SetStringField(TEXT("id"), Activity.Party.Id);

				TArray<TSharedPtr<FJsonValue>> SizeValues;

				if (Activity.Party.CurrentSize != -1)
				{
					SizeValues.Add(MakeShared<FJsonValueNumber>(Activity.Party.CurrentSize));
				}

				if (Activity.Party.MaxSize != -1)
				{
					SizeValues.Add(MakeShared<FJsonValueNumber>(Activity.Party.MaxSize));
				}

				PartyObject->SetArrayField(TEXT("size"), SizeValues);

				ActivityObject->SetObjectField(TEXT("party"), PartyObject);
			}

			// Assets
			{
				TSharedPtr<FJsonObject> AssetsObject = MakeShared<FJsonObject>();
				if (!Activity.Assets.LargeImage.IsEmpty())
				{
					AssetsObject->SetStringField(TEXT("large_image"), Activity.Assets.LargeImage);
				}
				if (!Activity.Assets.LargeText.IsEmpty())
				{
					AssetsObject->SetStringField(TEXT("large_text"), Activity.Assets.LargeText);
				}
				if (!Activity.Assets.SmallImage.IsEmpty())
				{
					AssetsObject->SetStringField(TEXT("small_image"), Activity.Assets.SmallImage);
				}
				if (!Activity.Assets.SmallText.IsEmpty())
				{
					AssetsObject->SetStringField(TEXT("small_text"), Activity.Assets.SmallText);
				}

				if (AssetsObject->Values.Num() > 0)
				{
					ActivityObject->SetObjectField(TEXT("assets"), AssetsObject);
				}
			}

			// Secrets
			{
				TSharedPtr<FJsonObject> SecretsObject = MakeShared<FJsonObject>();
				if (!Activity.Secrets.Join.IsEmpty())
				{
					SecretsObject->SetStringField(TEXT("join"), Activity.Secrets.Join);
				}
				if (!Activity.Secrets.Match.IsEmpty())
				{
					SecretsObject->SetStringField(TEXT("match"), Activity.Secrets.Match);
				}
				if (!Activity.Secrets.Spectate.IsEmpty())
				{
					SecretsObject->SetStringField(TEXT("spectate"), Activity.Secrets.Spectate);
				}

				if (SecretsObject->Values.Num() > 0)
				{
					ActivityObject->SetObjectField(TEXT("secrets"), SecretsObject);
				}
			}

			ActivityObject->SetBoolField(TEXT("instance"), Activity.bInstance);

			{
				int32 Flags = 0;
				for (const EDiscordGatewayActivityFlags& Flag : Activity.Flags)
				{
					Flags |= (1 << static_cast<int32>(Flag));
				}
				if (Flags != 0)
				{
					ActivityObject->SetNumberField(TEXT("flags"), Flags);
				}
			}

			ActivitesObject.Add(ActivityValueObject);
		}

		PresenceObject->SetArrayField(TEXT("activities"), ActivitesObject);
	}

	if (Presence.Since != -1)
	{
		PresenceObject->SetNumberField(TEXT("since"), Presence.Since);
	}

	if (!Presence.Status.IsEmpty())
	{
		PresenceObject->SetStringField(TEXT("status"),	Presence.Status);
		PresenceObject->SetBoolField  (TEXT("afk"),		Presence.bAfk);
	}

	if (PresenceObject->Values.Num() > 0)
	{
		DataObject->SetObjectField(TEXT("presence"), PresenceObject);
	}

	if (!bGuildSubscriptions)
	{
		DataObject->SetBoolField(TEXT("guild_subscriptions"), bGuildSubscriptions);
	}

	if (Intents.Num() > 0)
	{
		int32 IntentsVal = 0;
		for (const auto& Intent : Intents)
		{
			IntentsVal |= 1 << static_cast<int32>(Intent);
		}
		DataObject->SetNumberField(TEXT("intents"), IntentsVal);
	}

	// Converts the object to an FString to send it.
	FString StringifiedJson;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR> > > Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StringifiedJson);
	if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogDiscordGateway, Error, TEXT("Failed to serialize JSON data for INDENTIFY command."));
		return;
	}

	Send(StringifiedJson);

	UE_LOG(LogDiscordGateway, Log, TEXT("Indentify command sent to the Discord Gateway Server."));

#if !UE_BUILD_SHIPPING
	{
		FString PrettyPrintedJson;
		TSharedRef< TJsonWriter<> > PrettyWriter = TJsonWriterFactory<>::Create(&PrettyPrintedJson);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), PrettyWriter);
		UE_LOG(LogDiscordGateway, Verbose, TEXT("Identify data : \n% s"), *PrettyPrintedJson);
	}
#endif // !UE_SHIPPING
}

void UDiscordGatewaySocket::ConnectToGateway(const int32 GatewayVersion)
{
	if (!Socket.IsValid())
	{
		const FString GatewayUrl = FString::Printf(TEXT("wss://gateway.discord.gg/?v=%d&encoding=json"), GatewayVersion);

		Socket = FWebSocketsModule::Get().CreateWebSocket(GatewayUrl, TEXT("wss"));

		Socket->OnClosed()			.AddUObject(this, &UDiscordGatewaySocket::OnClosedInternal);
		Socket->OnConnected()		.AddUObject(this, &UDiscordGatewaySocket::OnConnectedInternal);
		Socket->OnConnectionError()	.AddUObject(this, &UDiscordGatewaySocket::OnConnectionErrorInternal);
		Socket->OnMessage()			.AddUObject(this, &UDiscordGatewaySocket::OnMessageInternal);
	}

	else if (Socket->IsConnected())
	{
		UE_LOG(LogDiscordGateway, Warning, TEXT("ConnectToGateway() called on a connected Socket."));
		return;
	}

	Socket->Connect();

	UE_LOG(LogDiscordGateway, Log, TEXT("Connecting to Discord gateway server..."));
}

void UDiscordGatewaySocket::OnClosedInternal(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogDiscordGateway, Error, TEXT("Socket closed. Status: %d. Reason: %s Clean: %s."), StatusCode, *Reason, bWasClean ? TEXT("Yes") : TEXT("No"));

	check(GWorld->GetWorld() != nullptr);

	bShouldHeartbeat = false;

	OnSocketClosedEvent.Broadcast(StatusCode, Reason, bWasClean);
}

void UDiscordGatewaySocket::OnConnectedInternal()
{
	UE_LOG(LogDiscordGateway, Log, TEXT("Connected to Discord gateway server."));
}

void UDiscordGatewaySocket::OnConnectionErrorInternal(const FString& Error)
{
	UE_LOG(LogDiscordGateway, Error, TEXT("Connection error: %s."), *Error);

	OnSocketClosedEvent.Broadcast(-1, Error, false);
}

void UDiscordGatewaySocket::OnMessageInternal(const FString& MessageString)
{
	TSharedPtr<FJsonObject> BaseObject;

	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(MessageString);
	if (!FJsonSerializer::Deserialize(Reader, BaseObject))
	{
		UE_LOG(LogDiscordGateway, Error, TEXT("Failed to deserialize gateway server response: %s."), *MessageString);
		return;
	}

	const FString OpKey			= TEXT("op");
	const FString DataKey		= TEXT("d");
	const FString SequenceKey	= TEXT("s");
	const FString EventKey		= TEXT("t");

	if (!BaseObject->HasTypedField<EJson::Number>(OpKey))
	{
		UE_LOG(LogDiscordGateway, Warning, TEXT("Gateway sent data that missed the \"op\" field. Data received: %s."), *MessageString);
		return;
	}

	const int32 OpValue = static_cast<int32>(BaseObject->GetNumberField(OpKey));

	// HB ACK.
	if (OpValue == 11)
	{
		return;
	}

	if (!BaseObject->HasTypedField<EJson::Object>(DataKey))
	{
		UE_LOG(LogDiscordGateway, Warning, TEXT("Gateway sent data that missed the \"d\" field. Data received: %s."), *MessageString);
		return;
	}

	const TSharedPtr<FJsonValue> DataValue = BaseObject->TryGetField(DataKey);

	TOptional<int32> SequenceValue;
	if (BaseObject->HasTypedField<EJson::Number>(SequenceKey))
	{
		SequenceValue = SequenceNumber = static_cast<int64>(BaseObject->GetNumberField(SequenceKey));
	}


	TOptional<FString> EventValue;
	if (BaseObject->HasTypedField<EJson::String>(EventKey))
	{
		EventValue = BaseObject->GetStringField(EventKey);
	}

	// Dispatch the event
	const auto* const Callback = HandledEvents.Find(OpValue);
	if (Callback && *Callback)
	{
		(this->*(*Callback))(OpValue, DataValue, SequenceValue, EventValue);
		return;
	}

	Event_Unhandled(OpValue, DataValue, SequenceValue, EventValue);
}

void UDiscordGatewaySocket::Event_Unhandled(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t)
{
	UE_LOG(LogDiscordGateway, Log, TEXT("Received op %d from Discord gateway."), op);

	OnSocketMessageEvent.Broadcast(IntToOpCode(op), d, s, t);
}

void UDiscordGatewaySocket::Event_Heartbeat(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t)
{
	// The server ask us to send a tick op.
	OnHeartbeatTick();
}

void UDiscordGatewaySocket::Event_Hello(const int32& op, const TSharedPtr<FJsonValue>& GatewayData, const TOptional<int32>& s, const TOptional<FString>& t)
{
	UE_LOG(LogDiscordGateway, Log, TEXT("Gateway server's Hello command received."));

	check(GatewayData.IsValid());

	const TSharedPtr<FJsonObject> Data = GatewayData->AsObject();

	const FString HeartbeatKey = TEXT("heartbeat_interval");

	int64 HeartbeatValue = 0;

	if (!Data->TryGetNumberField(HeartbeatKey, HeartbeatValue))
	{
		UE_LOG(LogDiscordGateway, Error, TEXT("Gateway Hello command was missing field %s in the data payload. Heartbeat has not been set."), *HeartbeatKey);
		return;
	}

	UE_LOG(LogDiscordGateway, Log, TEXT("Gateway requested a heartbeat of %d ms."), HeartbeatValue);
	
	bShouldHeartbeat = true;
	HeartbeatInterval = HeartbeatValue;
	HeartbeatCooldown = HeartbeatValue / 1000.f;

	UE_LOG(LogDiscordGateway, Log, TEXT("Discord Gateway heartbeat is set to %d milliseconds."), HeartbeatValue);
	
	OnSocketReadyEvent.Broadcast();
}

void UDiscordGatewaySocket::Event_InvalidSession(const int32& op, const TSharedPtr<FJsonValue>& d, const TOptional<int32>& s, const TOptional<FString>& t)
{
	UE_LOG(LogDiscordGateway, Warning, TEXT("Discord Gateway sent: %d: INVALID_SESSION. Clients are limited to 1 identify every 5 seconds."), op);

	OnSocketInvalidSessionEvent.Broadcast();
}

void UDiscordGatewaySocket::CloseConnection()
{
	if (Socket.IsValid())
	{
		Socket->Close();
	}
}

EDiscordGatewayOpCode UDiscordGatewaySocket::IntToOpCode(const int32 OpCode)
{
	switch (OpCode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
		return (EDiscordGatewayOpCode)OpCode;
	default:
		return EDiscordGatewayOpCode::Unknown;
	}
}
