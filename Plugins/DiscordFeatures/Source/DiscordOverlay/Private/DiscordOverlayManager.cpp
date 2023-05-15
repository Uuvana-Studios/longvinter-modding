// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordOverlayManager.h"
#include "DiscordManagerUtils.h"


/* static */ UDiscordOverlayManager* UDiscordOverlayManager::GetOverlayManager(UDiscordCore* DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->OverlayManager)
	{
		return Cast<UDiscordOverlayManager>(DiscordCore->OverlayManager);
	}

	UDiscordOverlayManager* const Manager = NewObject<UDiscordOverlayManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->OverlayManager = Manager;

	DiscordCore->OverlayOnToggle.BindUObject(Manager, &UDiscordOverlayManager::OnOverlayToggleInternal);

	return Manager;
}

bool UDiscordOverlayManager::IsEnabled()
{
	bool bEnabled = false;
	OVERLAY_CALL_RAW_CHECKED(is_enabled, &bEnabled);
	return bEnabled;
}

bool UDiscordOverlayManager::IsLocked()
{
	bool bLocked = false;
	OVERLAY_CALL_RAW_CHECKED(is_enabled, &bLocked);
	return bLocked;
}

void UDiscordOverlayManager::SetLocked(const bool bLocked, const FDiscordResultCallback& Callback)
{
	OVERLAY_CALL_RAW_CHECKED(set_locked, bLocked, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordOverlayManager::OpenActivityInvite(const EDiscordActivityActionType Type, const FDiscordResultCallback& Callback)
{
	OVERLAY_CALL_RAW_CHECKED(open_activity_invite, static_cast<FRawDiscord::EDiscordActivityActionType>(Type), new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordOverlayManager::OpenGuildInvite(const FString& Code, const FDiscordResultCallback& Callback)
{
	OVERLAY_CALL_RAW_CHECKED(open_guild_invite, TCHAR_TO_UTF8(*Code), new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordOverlayManager::OpenVoiceSettings(const FDiscordResultCallback& Callback)
{
	OVERLAY_CALL_RAW_CHECKED(open_voice_settings, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordOverlayManager::OnOverlayToggleInternal(const bool bLocked)
{
	OnOverlayToggle.Broadcast(bLocked);
}
