// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordCore.h"
#include "DiscordCoreModule.h"

#include "DiscordSDK.h"
#include "DiscordSDKModule.h"
#include "DiscordSettings.h"


static constexpr float DISCORD_TICK_ERROR_COOLDOWN = 10.f;

struct FDiscordCoreEventsDispatcher
{
	FDiscordCoreEventsDispatcher() = delete;

#if DISCORD_WITH_ACHIEVEMENT
	static void DiscordOnUserAchievementUpdate  (void* event_data, FRawDiscord::DiscordUserAchievement* user_achievement);
#endif // DISCORD_WITH_ACHIEVEMENT

#if DISCORD_WITH_USER
	static void DiscordOnCurrentUserUpdate		(void* const event_data);
#endif // DISCORD_WITH_USER

#if DISCORD_WITH_ACTIVITY
	static void DiscordOnActivityJoin			(void* event_data, const char* secret);
	static void DiscordOnActivitySpectate		(void* event_data, const char* secret);
	static void DiscordOnActivityJoinRequest	(void* event_data, FRawDiscord::DiscordUser* user);
	static void DiscordOnActivityInvite			(void* event_data, FRawDiscord::EDiscordActivityActionType type, FRawDiscord::DiscordUser* user, FRawDiscord::DiscordActivity* activity);
#endif // DISCORD_WITH_ACTIVITY

#if DISCORD_WITH_RELATIONSHIP
	static void DiscordOnRelationshipRefreshed	(void* event_data);
	static void DiscordOnRelationshipUpdate		(void* event_data, FRawDiscord::DiscordRelationship* relationship);
#endif // DISCORD_WITH_RELATIONSHIP

#if DISCORD_WITH_OVERLAY
	static void DiscordOnOverlayToggle			(void* event_data, bool locked);
#endif // DISCORD_WITH_OVERLAY

#if DISCORD_WITH_STORE
	static void DiscordOnEntitlementCreate		(void* event_data, FRawDiscord::DiscordEntitlement* entitlement);
	static void DiscordOnEntitlementDelete		(void* event_data, FRawDiscord::DiscordEntitlement* entitlement);
#endif // DISCORD_WITH_STORE

#if DISCORD_WITH_VOICE
	static void DiscordOnVoiceSettingsUpdate	(void* event_data);
#endif // DISCORD_WITH_VOICE

#if DISCORD_WITH_LOBBY
	static void DiscordOnLobbyUpdate			(void* event_data, int64_t lobby_id);
	static void DiscordOnLobbyDelete			(void* event_data, int64_t lobby_id, uint32_t reason);
	static void DiscordOnLobbyMemberConnect		(void* event_data, int64_t lobby_id, int64_t user_id);
	static void DiscordOnLobbyMemberUpdate		(void* event_data, int64_t lobby_id, int64_t user_id);
	static void DiscordOnLobbyMemberDisconnect	(void* event_data, int64_t lobby_id, int64_t user_id);
	static void DiscordOnLobbyMessage			(void* event_data, int64_t lobby_id, int64_t user_id, uint8* data, uint32_t data_length);
	static void DiscordOnLobbySpeaking          (void* event_data, int64_t lobby_id, int64_t user_id, bool speaking);
	static void DiscordOnLobbyNetworkMessage	(void* event_data, int64_t lobby_id, int64_t user_id, uint8 channel_id, uint8* data, uint32_t data_length);
#endif // DISCORD_WITH_LOBBY

#if DISCORD_WITH_NETWORK
	static void DiscordOnNetworkMessage			(void* event_data, FRawDiscord::DiscordNetworkPeerId peer_id, FRawDiscord::DiscordNetworkChannelId channel_id, uint8* data, uint32 data_length);
	static void DiscordOnNetworkRouteUpdate		(void* event_data, const char* route_data);
#endif // DISCORD_WITH_NETWORK
};

const TCHAR* FDiscordResult::DiscordResultToString(const FRawDiscord::EDiscordResult Result)
{
	return FDiscordResult::DiscordResultToString(FDiscordResult::ToEDiscordResult(Result));
}

const TCHAR* FDiscordResult::DiscordResultToString(const EDiscordResult Result)
{
	static const TCHAR* Results[] =
	{
		  TEXT("Ok")
		, TEXT("ServiceUnavailable")
		, TEXT("InvalidVersion")
		, TEXT("LockFailed")
		, TEXT("InternalError")
		, TEXT("InvalidPayload")
		, TEXT("InvalidCommand")
		, TEXT("InvalidPermissions")
		, TEXT("NotFetched")
		, TEXT("NotFound")
		, TEXT("Conflict")
		, TEXT("InvalidSecret")
		, TEXT("InvalidJoinSecret")
		, TEXT("NoEligibleActivity")
		, TEXT("InvalidInvite")
		, TEXT("NotAuthenticated")
		, TEXT("InvalidAccessToken")
		, TEXT("ApplicationMismatch")
		, TEXT("InvalidDataUrl")
		, TEXT("InvalidBase64")
		, TEXT("NotFiltered")
		, TEXT("LobbyFull")
		, TEXT("InvalidLobbySecret")
		, TEXT("InvalidFilename")
		, TEXT("InvalidFileSize")
		, TEXT("InvalidEntitlement")
		, TEXT("NotInstalled")
		, TEXT("NotRunning")
		, TEXT("InsufficientBuffer")
		, TEXT("PurchaseCanceled")
		, TEXT("InvalidGuild")
		, TEXT("InvalidEvent")
		, TEXT("InvalidChannel")
		, TEXT("InvalidOrigin")
		, TEXT("RateLimited")
		, TEXT("OAuth2Error")
		, TEXT("SelectChannelTimeout")
		, TEXT("GetGuildTimeout")
		, TEXT("SelectVoiceForceRequired")
		, TEXT("CaptureShortcutAlreadyListening")
		, TEXT("UnauthorizedForAchievement")
		, TEXT("InvalidGiftCode")
		, TEXT("PurchaseError")
		, TEXT("TransactionAborted")
		, TEXT("DrawingInitFailed")
		, TEXT("InvalidManager")
	};

	return Results[static_cast<int32>(Result)];
}

FDiscordUser::FDiscordUser(FRawDiscord::DiscordUser* Raw)
{	
	Username		= UTF8_TO_TCHAR(Raw->username);
	Avatar			= UTF8_TO_TCHAR(Raw->avatar);
	Discriminator	= UTF8_TO_TCHAR(Raw->discriminator);
	bBot			= Raw->bot;
	Id				= Raw->id;
}

/* static */ int64 UDiscordCore::GetTimeSinceEpoch()
{
	return FDateTime::Now().ToUnixTimestamp();
}

FDiscordActivityParty::FDiscordActivityParty(const FDiscordActivityParty& Other)
{
	*this = Other;
}

FDiscordActivityParty::FDiscordActivityParty(FDiscordActivityParty&& Other)
{
	*this = MoveTemp(Other);
}

FDiscordActivityParty& FDiscordActivityParty::operator=(const FDiscordActivityParty& Other)
{
	Id = Other.Id;
	Size = Other.Size;

	return *this;
}

FDiscordActivityParty& FDiscordActivityParty::operator=(FDiscordActivityParty&& Other)
{
	Id	 = MoveTemp(Other.Id);
	Size = Other.Size;

	return *this;
}

FDiscordActivityAssets::FDiscordActivityAssets(const FDiscordActivityAssets& Other)
{
	*this = Other;
}

FDiscordActivityAssets::FDiscordActivityAssets(FDiscordActivityAssets&& Other)
{
	*this = MoveTemp(Other);
}

FDiscordActivityAssets& FDiscordActivityAssets::operator=(const FDiscordActivityAssets& Other)
{
	LargeImage = (Other.LargeImage);
	SmallImage = (Other.SmallImage);
	LargeText  = (Other.LargeImage);
	SmallText  = (Other.SmallText);

	return *this;
}

FDiscordActivityAssets& FDiscordActivityAssets::operator=(FDiscordActivityAssets&& Other)
{
	LargeImage = MoveTemp(Other.LargeImage);
	SmallImage = MoveTemp(Other.SmallImage);
	LargeText  = MoveTemp(Other.LargeImage);
	SmallText  = MoveTemp(Other.SmallText);

	return *this;
}

FDiscordActivity::FDiscordActivity(struct FRawDiscord::DiscordActivity* Raw)
	: Type(static_cast<EDiscordActivityType>(Raw->type))
	, ApplicationId(Raw->application_id)
	, Name(UTF8_TO_TCHAR(Raw->name))
	, State(UTF8_TO_TCHAR(Raw->state))
	, Details(UTF8_TO_TCHAR(Raw->details))
	, bInstance(Raw->instance)
{
	Timestamps.End    = Raw->timestamps.end;
	Timestamps.Start  = Raw->timestamps.start;
	Assets.LargeImage = UTF8_TO_TCHAR(Raw->assets.large_image);
	Assets.LargeText  = UTF8_TO_TCHAR(Raw->assets.large_text);
	Assets.SmallImage = UTF8_TO_TCHAR(Raw->assets.small_image);
	Assets.SmallText  = UTF8_TO_TCHAR(Raw->assets.small_text);
	Party.Id				= Raw->party.id;
	Party.Size.CurrentSize	= Raw->party.size.current_size;
	Party.Size.MaxSize      = Raw->party.size.max_size;
	Secrets.Join	 = UTF8_TO_TCHAR(Raw->secrets.join);
	Secrets.Match	 = UTF8_TO_TCHAR(Raw->secrets.match);
	Secrets.Spectate = UTF8_TO_TCHAR(Raw->secrets.spectate);
}

FDiscordActivity::FDiscordActivity(FDiscordActivity&& Other)
{
	*this = MoveTemp(Other);
}

FDiscordActivity::FDiscordActivity(const FDiscordActivity& Other)
{
	*this = Other;
}

FDiscordActivity& FDiscordActivity::operator=(const FDiscordActivity& Other)
{
	Name			= (Other.Name);
	Type			= (Other.Type);
	ApplicationId	= (Other.ApplicationId);
	State			= (Other.State);
	Details			= (Other.Details);
	bInstance		= (Other.bInstance);
	Timestamps		= (Other.Timestamps);
	Assets			= (Other.Assets);
	Party			= (Other.Party);
	Secrets			= (Other.Secrets);

	return *this;
}

FDiscordActivity& FDiscordActivity::operator=(FDiscordActivity&& Other)
{
	Name			= MoveTempIfPossible(Other.Name);
	Type			= MoveTempIfPossible(Other.Type);
	ApplicationId	= MoveTempIfPossible(Other.ApplicationId);
	State			= MoveTempIfPossible(Other.State);
	Details			= MoveTempIfPossible(Other.Details);
	bInstance		= MoveTempIfPossible(Other.bInstance);
	Timestamps		= MoveTempIfPossible(Other.Timestamps);
	Assets			= MoveTempIfPossible(Other.Assets);
	Party			= MoveTempIfPossible(Other.Party);
	Secrets			= MoveTempIfPossible(Other.Secrets);

	return *this;
}

FDiscordEntitlement::FDiscordEntitlement(FRawDiscord::DiscordEntitlement* Raw)
{
	Id = Raw->id;
	SkuId = Raw->sku_id;
	Type = static_cast<EDiscordEntitlementType>(Raw->type);
}


void DiscordLogCallback(void* const Data, FRawDiscord::EDiscordLogLevel Level, const char* message)
{
	const FString Message = UTF8_TO_TCHAR(message);

	switch (Level)
	{
	case FRawDiscord::EDiscordLogLevel::Error: UE_LOG(LogDiscordCore, Error,   TEXT("Discord Error: %s."),   *Message); break;
	case FRawDiscord::EDiscordLogLevel::Warn:  UE_LOG(LogDiscordCore, Warning, TEXT("Discord Warning: %s."), *Message); break;
	case FRawDiscord::EDiscordLogLevel::Info:  UE_LOG(LogDiscordCore, Log,     TEXT("Discord Info: %s."),    *Message); break;
	case FRawDiscord::EDiscordLogLevel::Debug: UE_LOG(LogDiscordCore, Verbose, TEXT("Discord Debug: %s."),   *Message); break;
	}

	UDiscordCore* const Core = (UDiscordCore*)Data;
	if (Core)
	{
		Core->LogHookEvent.ExecuteIfBound(static_cast<EDiscordLogLevel>(Level), Message);
	}
	else
	{
		UE_LOG(LogDiscordCore, Warning, TEXT("Passed a nullptr Core to SetLogHook."));
	}
}

void UDiscordCore::SetLogHook(const FDiscordLogEvent& OnLogCallback)
{
	LogHookEvent = OnLogCallback;
}

#define DISCORD_DISPATCHER_CALLBACK(Name, ...) { \
	UDiscordCore* const Core = (UDiscordCore*)event_data; \
	if (Core-> Name .IsBound()) \
	{ \
		Core-> Name .Execute(__VA_ARGS__); \
	} \
	else \
	{ \
		UE_LOG(LogDiscordCore, Warning, TEXT("Event ")  TEXT(#Name) TEXT(" received but there isn't any listener for it.")); \
	} \
}

#if DISCORD_WITH_USER
void FDiscordCoreEventsDispatcher::DiscordOnCurrentUserUpdate(void* const event_data)
{
	DISCORD_DISPATCHER_CALLBACK(UserOnUpdate);
}
#endif // DISCORD_WITH_USER

#if DISCORD_WITH_ACTIVITY
void FDiscordCoreEventsDispatcher::DiscordOnActivityJoin(void* event_data, const char* secret)
{
	DISCORD_DISPATCHER_CALLBACK(ActivityOnJoin, secret);
}

void FDiscordCoreEventsDispatcher::DiscordOnActivitySpectate(void* event_data, const char* secret)
{
	DISCORD_DISPATCHER_CALLBACK(ActivityOnSpectate, secret);
}

void FDiscordCoreEventsDispatcher::DiscordOnActivityJoinRequest(void* event_data, FRawDiscord::DiscordUser* user)
{
	DISCORD_DISPATCHER_CALLBACK(ActivityOnJoinRequest, user);
}

void FDiscordCoreEventsDispatcher::DiscordOnActivityInvite(void* event_data, FRawDiscord::EDiscordActivityActionType type, FRawDiscord::DiscordUser* user, FRawDiscord::DiscordActivity* activity)
{
	DISCORD_DISPATCHER_CALLBACK(ActivityOnInvite, type, user, activity);
}
#endif // DISCORD_WITH_ACTIVITY

#if DISCORD_WITH_RELATIONSHIP
void FDiscordCoreEventsDispatcher::DiscordOnRelationshipRefreshed(void* event_data)
{
	DISCORD_DISPATCHER_CALLBACK(RelationshipOnRefreshed);
}

void FDiscordCoreEventsDispatcher::DiscordOnRelationshipUpdate(void* event_data, FRawDiscord::DiscordRelationship* relationship)
{
	DISCORD_DISPATCHER_CALLBACK(RelationshipOnUpdate, relationship)
}
#endif // DISCORD_WITH_RELATIONSHIP

#if DISCORD_WITH_ACHIEVEMENT
void FDiscordCoreEventsDispatcher::DiscordOnUserAchievementUpdate(void* event_data, FRawDiscord::DiscordUserAchievement* user_achievement)
{
	DISCORD_DISPATCHER_CALLBACK(AchievementOnUserAchivementUpdate, user_achievement);
}
#endif // DISCORD_WITH_ACHIEVEMENT

#if DISCORD_WITH_LOBBY
void FDiscordCoreEventsDispatcher::DiscordOnLobbyUpdate(void* event_data, int64_t lobby_id)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnUpdate, lobby_id);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbyDelete(void* event_data, int64_t lobby_id, uint32_t reason)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnDelete, lobby_id, reason);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbyMemberConnect(void* event_data, int64_t lobby_id, int64_t user_id)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnMemberConnect, lobby_id, user_id);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbyMemberUpdate(void* event_data, int64_t lobby_id, int64_t user_id)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnMemberUpdate, lobby_id, user_id);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbyMemberDisconnect(void* event_data, int64_t lobby_id, int64_t user_id)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnMemberDisconnect, lobby_id, user_id);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbyMessage(void* event_data, int64_t lobby_id, int64_t user_id, uint8* data, uint32_t data_length)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnMessage, lobby_id, user_id, data, data_length);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbySpeaking(void* event_data, int64_t lobby_id, int64_t user_id, bool speaking)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnSpeaking, lobby_id, user_id, speaking);
}

void FDiscordCoreEventsDispatcher::DiscordOnLobbyNetworkMessage(void* event_data, int64_t lobby_id, int64_t user_id, uint8 channel_id, uint8* data, uint32_t data_length)
{
	DISCORD_DISPATCHER_CALLBACK(LobbyOnNetworkMessage, lobby_id, user_id, channel_id, data, data_length);
}
#endif // DISCORD_WITH_LOBBY

#if DISCORD_WITH_OVERLAY
void FDiscordCoreEventsDispatcher::DiscordOnOverlayToggle(void* event_data, bool locked)
{
	DISCORD_DISPATCHER_CALLBACK(OverlayOnToggle, locked);
}
#endif // DISCORD_WITH_OVERLAY

#if DISCORD_WITH_STORE
void FDiscordCoreEventsDispatcher::DiscordOnEntitlementCreate(void* event_data, FRawDiscord::DiscordEntitlement* entitlement)
{
	DISCORD_DISPATCHER_CALLBACK(EntitlementOnCreate, entitlement);
}

void FDiscordCoreEventsDispatcher::DiscordOnEntitlementDelete(void* event_data, FRawDiscord::DiscordEntitlement* entitlement)
{
	DISCORD_DISPATCHER_CALLBACK(EntitlementOnDelete, entitlement);
}
#endif // DISCORD_WITH_STORE

#if DISCORD_WITH_VOICE
void FDiscordCoreEventsDispatcher::DiscordOnVoiceSettingsUpdate(void* event_data)
{
	DISCORD_DISPATCHER_CALLBACK(VoiceOnSettingsUpdate);
}
#endif // DISCORD_WITH_VOICE

#if DISCORD_WITH_NETWORK
void FDiscordCoreEventsDispatcher::DiscordOnNetworkMessage(void* event_data, FRawDiscord::DiscordNetworkPeerId peer_id, FRawDiscord::DiscordNetworkChannelId channel_id, uint8* data, uint32_t data_length)
{
	DISCORD_DISPATCHER_CALLBACK(NetworkOnMessage, peer_id, channel_id, data, data_length);
}

void FDiscordCoreEventsDispatcher::DiscordOnNetworkRouteUpdate(void* event_data, const char* route_data)
{
	DISCORD_DISPATCHER_CALLBACK(NetworkOnRouteUpdate, route_data);
}
#endif // DISCORD_WITH_NETWORK

#undef DISCORD_DISPATCHER_CALLBACK

UDiscordCore::UDiscordCore() : Super()
{

}

UDiscordCore::~UDiscordCore()
{
	if (App.core)
	{
		// See https://github.com/discord/gamesdk-and-dispatch/issues/37.
		// To avoid a crash, we have no other choice but to leak the core on Windows.
		// TODO: Fix the issue as soon as a patch is available.
#if !PLATFORM_WINDOWS
		App.core->destroy(App.core);
#endif
		FMemory::Memzero(&App, sizeof(FDiscordApplication));
	}

	UE_LOG(LogDiscordCore, Log, TEXT("Discord Core destroyed."));
}

/* static */ UDiscordCore* UDiscordCore::CreateDiscordCore(const EDiscordCoreCreationFlags CreationFlags)
{
	UDiscordCore* const NewCore = NewObject<UDiscordCore>();
	
	if (NewCore->InitCore(CreationFlags))
	{
		UE_LOG(LogDiscordCore, Log, TEXT("New Discord Core initialized."));
		return NewCore;
	}

	UE_LOG(LogDiscordCore, Warning, TEXT("Discord Core initialization failed."));

	return nullptr;
}

bool UDiscordCore::InitCore(
	const EDiscordCoreCreationFlags CreationFlags
)
{
	FRawDiscord::DiscordCreateParams Params;

	UDiscordSettings::SetDefaultDiscordCreateParams(&Params);

	Params.client_id  = UDiscordSettings::GetClientId();

	UE_LOG(LogDiscordCore, Log, TEXT("Application ID is %lld."), Params.client_id);

	Params.flags      = (uint64)(CreationFlags == EDiscordCoreCreationFlags::Default ? FRawDiscord::EDiscordCreateFlags::Default : FRawDiscord::EDiscordCreateFlags::NoRequireDiscord);
	Params.event_data = this;

	Params.achievement_events   = &AchievementEvents;
	Params.activity_events		= &ActivitiesEvents;
	Params.application_events   = &ApplicationEvents;
	Params.image_events			= &ImageEvents;
	Params.network_events		= &NetworkEvents;
	Params.overlay_events		= &OverlayEvents;
	Params.relationship_events	= &RelationshipsEvents;
	Params.storage_events		= &StorageEvents;
	Params.store_events			= &StoreEvents;
	Params.user_events			= &UserEvents;
	Params.voice_events			= &VoiceEvents;
	Params.lobby_events			= &LobbyEvents;

#if DISCORD_WITH_ACHIEVEMENT
	AchievementEvents.on_user_achievement_update	= FDiscordCoreEventsDispatcher::DiscordOnUserAchievementUpdate;
#endif // DISCORD_WITH_ACHIEVEMENT

#if DISCORD_WITH_ACTIVITY
	ActivitiesEvents.on_activity_invite				= FDiscordCoreEventsDispatcher::DiscordOnActivityInvite;
	ActivitiesEvents.on_activity_join				= FDiscordCoreEventsDispatcher::DiscordOnActivityJoin;
	ActivitiesEvents.on_activity_join_request		= FDiscordCoreEventsDispatcher::DiscordOnActivityJoinRequest;
	ActivitiesEvents.on_activity_spectate			= FDiscordCoreEventsDispatcher::DiscordOnActivitySpectate;
#endif // DISCORD_WITH_ACTIVITY

#if DISCORD_WITH_LOBBY
	LobbyEvents.on_lobby_delete						= FDiscordCoreEventsDispatcher::DiscordOnLobbyDelete;
	LobbyEvents.on_lobby_message					= FDiscordCoreEventsDispatcher::DiscordOnLobbyMessage;
	LobbyEvents.on_lobby_update						= FDiscordCoreEventsDispatcher::DiscordOnLobbyUpdate;
	LobbyEvents.on_member_connect					= FDiscordCoreEventsDispatcher::DiscordOnLobbyMemberConnect;
	LobbyEvents.on_member_disconnect				= FDiscordCoreEventsDispatcher::DiscordOnLobbyMemberDisconnect;
	LobbyEvents.on_member_update					= FDiscordCoreEventsDispatcher::DiscordOnLobbyMemberUpdate;
	LobbyEvents.on_network_message					= FDiscordCoreEventsDispatcher::DiscordOnLobbyNetworkMessage;
	LobbyEvents.on_speaking							= FDiscordCoreEventsDispatcher::DiscordOnLobbySpeaking;
#endif // DISCORD_WITH_LOBBY

#if DISCORD_WITH_OVERLAY
	OverlayEvents.on_toggle							= FDiscordCoreEventsDispatcher::DiscordOnOverlayToggle;
#endif // DISCORD_WITH_OVERLAY

#if DISCORD_WITH_RELATIONSHIP
	RelationshipsEvents.on_refresh					= FDiscordCoreEventsDispatcher::DiscordOnRelationshipRefreshed;
	RelationshipsEvents.on_relationship_update		= FDiscordCoreEventsDispatcher::DiscordOnRelationshipUpdate;
#endif // DISCORD_WITH_RELATIONSHIP

#if DISCORD_WITH_STORE
	StoreEvents.on_entitlement_create				= FDiscordCoreEventsDispatcher::DiscordOnEntitlementCreate;
	StoreEvents.on_entitlement_delete				= FDiscordCoreEventsDispatcher::DiscordOnEntitlementDelete;
#endif // DISCORD_WITH_STORE

#if DISCORD_WITH_NETWORK
	NetworkEvents.on_message						= FDiscordCoreEventsDispatcher::DiscordOnNetworkMessage;
	NetworkEvents.on_route_update					= FDiscordCoreEventsDispatcher::DiscordOnNetworkRouteUpdate;
#endif // DISCORD_WITH_NETWORK

#if DISCORD_WITH_VOICE
	VoiceEvents.on_settings_update					= FDiscordCoreEventsDispatcher::DiscordOnVoiceSettingsUpdate;
#endif // DISCORD_WITH_VOICE

#if DISCORD_WITH_USER
	UserEvents.on_current_user_update				= FDiscordCoreEventsDispatcher::DiscordOnCurrentUserUpdate;
#endif // DISCORD_WITH_USER

	App.core = FDiscordSdk::Get()->CreateCore(Params);

	if (App.core)
	{
		App.core->set_log_hook(App.core, FRawDiscord::EDiscordLogLevel::Debug, this, DiscordLogCallback);

		App.achievements	= App.core->get_achievement_manager	(App.core);
		App.activities		= App.core->get_activity_manager	(App.core);
		App.application		= App.core->get_application_manager	(App.core);
		App.lobbies			= App.core->get_lobby_manager		(App.core);
		App.relationships	= App.core->get_relationship_manager(App.core);
		App.users			= App.core->get_user_manager		(App.core);
		App.network			= App.core->get_network_manager		(App.core);
		App.voice			= App.core->get_voice_manager		(App.core);
		App.image			= App.core->get_image_manager		(App.core);
		App.storage			= App.core->get_storage_manager		(App.core);
		App.store			= App.core->get_store_manager		(App.core);
		App.overlay			= App.core->get_overlay_manager		(App.core);

		return true;
	}

	return false;
}

uint64 UDiscordCore::GetApplicationId()
{
	return UDiscordSettings::GetClientId();
}

TStatId UDiscordCore::GetStatId() const
{
	return TStatId();
}

void UDiscordCore::Tick(float DeltaTime)
{
	if (App.core)
	{
		FRawDiscord::EDiscordResult TickResult = App.core->run_callbacks(App.core);

		OnRunCallbacks.Broadcast(FDiscordResult::ToEDiscordResult(TickResult));

		if (TickResult != FRawDiscord::EDiscordResult::Ok)
		{
			// Used to prevent spamming the output log.
			// Static as the result would be the same for all cores.
			static float ErrorCooldown = 0.f;

			ErrorCooldown = FMath::Max(ErrorCooldown - DeltaTime, 0.f);

			if (FMath::IsNearlyZero(ErrorCooldown))
			{
				UE_LOG(LogDiscordCore, Warning, TEXT("Discord RunCallbacks() returned \"%d: %s\"."),
					(int32)TickResult, FDiscordResult::DiscordResultToString(static_cast<EDiscordResult>(TickResult)));
				
				ErrorCooldown = DISCORD_TICK_ERROR_COOLDOWN;
			}
		}
	}
}

bool UDiscordCore::IsTickable() const
{
	return App.core != nullptr;
}

void UDiscordCore::OnCurrentUserUpdated()
{

}
