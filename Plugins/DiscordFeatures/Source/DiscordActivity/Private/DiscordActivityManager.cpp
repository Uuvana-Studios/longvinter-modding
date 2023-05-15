// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordActivityManager.h"
#include "DiscordManagerUtils.h"

/* static */ UDiscordActivityManager* UDiscordActivityManager::GetActivityManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->ActivityManager)
	{
		return Cast<UDiscordActivityManager>(DiscordCore->ActivityManager);
	}

	UDiscordActivityManager* const Manager = NewObject<UDiscordActivityManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->ActivityOnJoin			.BindUObject(Manager, &UDiscordActivityManager::OnActivityJoinInternal);
	DiscordCore->ActivityOnInvite		.BindUObject(Manager, &UDiscordActivityManager::OnActivityInviteInternal);
	DiscordCore->ActivityOnJoinRequest	.BindUObject(Manager, &UDiscordActivityManager::OnActivityJoinRequestInternal);
	DiscordCore->ActivityOnSpectate		.BindUObject(Manager, &UDiscordActivityManager::OnActivitySpectateInternal);

	DiscordCore->ActivityManager = Manager;

	return Manager;
}

void UDiscordActivityManager::OnActivityJoinInternal(const char* secret)
{
	OnActivityJoin.Broadcast(UTF8_TO_TCHAR(secret));
}

void UDiscordActivityManager::OnActivitySpectateInternal(const char* secret)
{
	OnActivitySpectate.Broadcast(UTF8_TO_TCHAR(secret));
}

void UDiscordActivityManager::OnActivityJoinRequestInternal(FRawDiscord::DiscordUser* user)
{
	FDiscordUser User(user);

	OnActivityJoinRequest.Broadcast(User);
}

void UDiscordActivityManager::OnActivityInviteInternal(FRawDiscord::EDiscordActivityActionType type, FRawDiscord::DiscordUser* user, FRawDiscord::DiscordActivity* activity)
{
	FDiscordUser User(user);
	EDiscordActivityActionType Type = static_cast<EDiscordActivityActionType>(type);
	FDiscordActivity Activity(activity);
	OnActivityInvite.Broadcast(Type, User, Activity);
}

void UDiscordActivityManager::RegisterCommand(const FString& Command)
{
	ACTIVITY_CALL_RAW_CHECKED(register_command, TCHAR_TO_UTF8(*Command));
}

void UDiscordActivityManager::RegisterSteam(const int64 SteamId)
{
	ACTIVITY_CALL_RAW_CHECKED(register_steam, static_cast<uint32>(SteamId));
}

void UDiscordActivityManager::UpdateActivity(const FDiscordActivity& Activity, FDiscordResultCallback OnActivityUpdated)
{
#define COPY_STRING_TO_BUFFER(From, To) FDiscordManagerUtils::CopyStringToBuffer(From, To)

	FRawDiscord::DiscordActivity RawActivity;
	FMemory::Memzero(RawActivity);

	//RawActivity.application_id = DiscordCore->GetApplicationId();
	
	COPY_STRING_TO_BUFFER(Activity.Assets.LargeImage, RawActivity.assets.large_image);
	COPY_STRING_TO_BUFFER(Activity.Assets.LargeText,  RawActivity.assets.large_text);
	COPY_STRING_TO_BUFFER(Activity.Assets.SmallImage, RawActivity.assets.small_image);
	COPY_STRING_TO_BUFFER(Activity.Assets.SmallText,  RawActivity.assets.small_text);

	COPY_STRING_TO_BUFFER(Activity.Details, RawActivity.details);
	
	RawActivity.instance = Activity.bInstance; 

	//COPY_STRING_TO_BUFFER(Activity.Name, RawActivity.name);

	COPY_STRING_TO_BUFFER(Activity.Party.Id, RawActivity.party.id);

	RawActivity.party.size.current_size = Activity.Party.Size.CurrentSize;
	RawActivity.party.size.max_size		= Activity.Party.Size.MaxSize;

	COPY_STRING_TO_BUFFER(Activity.Secrets.Join,	 RawActivity.secrets.join);
	COPY_STRING_TO_BUFFER(Activity.Secrets.Match,	 RawActivity.secrets.match);
	COPY_STRING_TO_BUFFER(Activity.Secrets.Spectate, RawActivity.secrets.spectate);

	COPY_STRING_TO_BUFFER(Activity.State, RawActivity.state);

	RawActivity.timestamps.start = Activity.Timestamps.Start;
	RawActivity.timestamps.end   = Activity.Timestamps.End;

	RawActivity.type = static_cast<FRawDiscord::EDiscordActivityType>(Activity.Type);

#undef COPY_STRING_TO_BUFFER	

	ACTIVITY_CALL_RAW_CHECKED(update_activity, &RawActivity, new FDiscordResultCallback(MoveTemp(OnActivityUpdated)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordActivityManager::ClearActivity(FDiscordResultCallback OnActivityCleared)
{
	ACTIVITY_CALL_RAW_CHECKED(clear_activity, new FDiscordResultCallback(MoveTemp(OnActivityCleared)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordActivityManager::SendRequestReply(const int64 UserId, const EDiscordActivityJoinRequestReply& Reply, FDiscordResultCallback OnResponded)
{
	ACTIVITY_CALL_RAW_CHECKED(send_request_reply, UserId, static_cast<FRawDiscord::EDiscordActivityJoinRequestReply>(Reply), new FDiscordResultCallback(MoveTemp(OnResponded)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordActivityManager::SendInvite(const int64 UserId, const EDiscordActivityActionType Action, const FString& Content, FDiscordResultCallback OnInviteSent)
{
	ACTIVITY_CALL_RAW_CHECKED(send_invite, UserId, static_cast<FRawDiscord::EDiscordActivityActionType>(Action), TCHAR_TO_UTF8(*Content), new FDiscordResultCallback(MoveTemp(OnInviteSent)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordActivityManager::AcceptInvite(const int64 UserId, FDiscordResultCallback OnAcceptInviteResponse)
{
	ACTIVITY_CALL_RAW_CHECKED(accept_invite, UserId, new FDiscordResultCallback(MoveTemp(OnAcceptInviteResponse)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}
