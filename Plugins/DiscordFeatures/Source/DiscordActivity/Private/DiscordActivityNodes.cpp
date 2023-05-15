// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordActivityNodes.h"

/* static */ UUpdateActivityProxy* UUpdateActivityProxy::UpdateActivity(UDiscordActivityManager* ActivityManager, const FDiscordActivity& Activity)
{ 
	UUpdateActivityProxy* const Proxy = NewObject<UUpdateActivityProxy>();
	
	Proxy->Activity = Activity;
	Proxy->Manager = ActivityManager;
	
	return Proxy;
}

void UUpdateActivityProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Update Activity Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnActivityUpdated.Broadcast(EDiscordResult::InternalError);
		SetReadyToDestroy();
		return;
	}

	Manager->UpdateActivity(Activity, FDiscordResultCallback::CreateUObject(this, &UUpdateActivityProxy::OnActivityUpdatedInternal));
}

void UUpdateActivityProxy::OnActivityUpdatedInternal(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnActivityUpdated : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

/* static */ UClearActivityProxy* UClearActivityProxy::ClearActivity(UDiscordActivityManager* ActivityManager)
{
	UClearActivityProxy* const Proxy = NewObject<UClearActivityProxy>();

	Proxy->Manager = ActivityManager;

	return Proxy;
}

void UClearActivityProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Clear activity Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnActivityClearedInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->ClearActivity(FDiscordResultCallback::CreateUObject(this, &UClearActivityProxy::OnActivityClearedInternal));
}

void UClearActivityProxy::OnActivityClearedInternal(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnActivityCleared : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

/* static */ USendRequestReplyProxy* USendRequestReplyProxy::SendRequestReply(UDiscordActivityManager* ActivityManager, const int64 UserId, const EDiscordActivityJoinRequestReply Reply)
{
	USendRequestReplyProxy* const Proxy = NewObject<USendRequestReplyProxy>();

	Proxy->Manager = ActivityManager;
	Proxy->Reply   = Reply;
	Proxy->UserId  = UserId;

	return Proxy;
}

void USendRequestReplyProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Update Activity Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnRequestReplyInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->SendRequestReply(UserId, Reply, FDiscordResultCallback::CreateUObject(this, &USendRequestReplyProxy::OnRequestReplyInternal));
}

void USendRequestReplyProxy::OnRequestReplyInternal(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnRequestReply : OnError).Broadcast(Result);
	SetReadyToDestroy();
}


/* static */ USendInviteProxy* USendInviteProxy::SendInvite(UDiscordActivityManager* ActivityManager, const int64& UserId, const EDiscordActivityActionType Action, const FString& Content)
{
	USendInviteProxy* const Proxy = NewObject<USendInviteProxy>();

	Proxy->Manager = ActivityManager;
	Proxy->Action = Action;
	Proxy->UserId = UserId;
	Proxy->Content = Content;

	return Proxy;
}

void USendInviteProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Update Activity Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnInviteSentInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->SendInvite(UserId, Action, Content, FDiscordResultCallback::CreateUObject(this, &USendInviteProxy::OnInviteSentInternal));
}

void USendInviteProxy::OnInviteSentInternal(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnInviteSent : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

/* static */ UAcceptInvite* UAcceptInvite::AcceptInvite(UDiscordActivityManager* ActivityManager, const int64& UserId)
{
	UAcceptInvite* const Proxy = NewObject<UAcceptInvite>();

	Proxy->Manager = ActivityManager;
	Proxy->UserId = UserId;

	return Proxy;
}

void UAcceptInvite::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Accept invite Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnInviteAcceptedInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->AcceptInvite(UserId, FDiscordResultCallback::CreateUObject(this, &UAcceptInvite::OnInviteAcceptedInternal));
}

void UAcceptInvite::OnInviteAcceptedInternal(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnInviteAccepted : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

