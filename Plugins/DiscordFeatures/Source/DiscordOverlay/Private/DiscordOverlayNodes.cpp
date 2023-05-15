// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordOverlayNodes.h"

void UBaseOverlayAsyncProxy::OnResult(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnSuccess : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

FDiscordResultCallback UBaseOverlayAsyncProxy::CreateCallback()
{
	return FDiscordResultCallback::CreateUObject(this, &UBaseOverlayAsyncProxy::OnResult);
}

/* static */ USetLockedProxy* USetLockedProxy::SetLocked(UDiscordOverlayManager* OverlayManager, const bool bLocked)
{
	USetLockedProxy* const Proxy = NewObject<USetLockedProxy>();

	Proxy->Manager = OverlayManager;
	Proxy->bLocked = bLocked;

	return Proxy;
}

void USetLockedProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Set Locked failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResult(EDiscordResult::InternalError);
		return;
	}

	Manager->SetLocked(bLocked, CreateCallback());
}

/* static */ UOpenActivityInviteProxy* UOpenActivityInviteProxy::OpenActivityInvite(UDiscordOverlayManager* OverlayManager, const EDiscordActivityActionType Type)
{
	UOpenActivityInviteProxy* const Proxy = NewObject<UOpenActivityInviteProxy>();

	Proxy->Type = Type;
	Proxy->Manager = OverlayManager;

	return Proxy;
}

void UOpenActivityInviteProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Open Activity Invite failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResult(EDiscordResult::InternalError);
		return;
	}

	Manager->OpenActivityInvite(Type, CreateCallback());
}

/* static */ UOpenGuildInviteProxy* UOpenGuildInviteProxy::OpenGuildInvite(UDiscordOverlayManager* OverlayManager, const FString& Code)
{
	UOpenGuildInviteProxy* const Proxy = NewObject<UOpenGuildInviteProxy>();

	Proxy->Code = Code;
	Proxy->Manager = OverlayManager;

	return Proxy;
}

void UOpenGuildInviteProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Open Guild Invite failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResult(EDiscordResult::InternalError);
		return;
	}

	Manager->OpenGuildInvite(Code, CreateCallback());
}

/* static */ UOpenVoiceSettingsProxy* UOpenVoiceSettingsProxy::OpenVoiceSettings(UDiscordOverlayManager* OverlayManager)
{
	UOpenVoiceSettingsProxy* const Proxy = NewObject<UOpenVoiceSettingsProxy>();

	Proxy->Manager = OverlayManager;

	return Proxy;
}

void UOpenVoiceSettingsProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Open Voice Settings failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResult(EDiscordResult::InternalError);
		return;
	}

	Manager->OpenVoiceSettings(CreateCallback());
}
