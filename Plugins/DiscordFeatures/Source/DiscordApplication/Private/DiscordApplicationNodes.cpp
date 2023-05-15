// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordApplicationNodes.h"

#define GET_EVENT_FOR_RESULT(OnOk, OnNotOk) (Result == EDiscordResult::Ok ? OnOk : OnNotOk)

/* static */ UApplicationGetOAuth2TokenProxy* UApplicationGetOAuth2TokenProxy::GetOAuth2Token(UDiscordApplicationManager* ApplicationManager)
{
	UApplicationGetOAuth2TokenProxy* const Proxy = NewObject<UApplicationGetOAuth2TokenProxy>();

	Proxy->Manager = ApplicationManager;

	return Proxy;
}

void UApplicationGetOAuth2TokenProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to get OAuth2Token: Manager is nullptr."), ELogVerbosity::Error);
		OnOAuth2TokenReceivedInternal(EDiscordResult::InternalError, FDiscordOAuth2Token());
		return;
	}

	Manager->GetOAuth2Token(FOnOAuth2TokenReceived::CreateUObject(this, &UApplicationGetOAuth2TokenProxy::OnOAuth2TokenReceivedInternal));
}

void UApplicationGetOAuth2TokenProxy::OnOAuth2TokenReceivedInternal(EDiscordResult Result, const FDiscordOAuth2Token& Token)
{
	GET_EVENT_FOR_RESULT(OnOAuth2TokenReceived, OnError).Broadcast(Result, Token);
	SetReadyToDestroy();
}

/* static */ UApplicationValidateOrExitProxy* UApplicationValidateOrExitProxy::ValidateOrExit(UDiscordApplicationManager* ApplicationManager)
{
	UApplicationValidateOrExitProxy* const Proxy = NewObject<UApplicationValidateOrExitProxy>();

	Proxy->Manager = ApplicationManager;

	return Proxy;
}

void UApplicationValidateOrExitProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to validate or exit: Manager is nullptr."), ELogVerbosity::Error);
		OnResultInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->ValidateOrExit(FDiscordResultCallback::CreateUObject(this, &UApplicationValidateOrExitProxy::OnResultInternal));
}

void UApplicationValidateOrExitProxy::OnResultInternal(EDiscordResult Result)
{
	GET_EVENT_FOR_RESULT(OnValidated, OnExit).Broadcast(Result);
	SetReadyToDestroy();
}

UApplicationGetTicketProxy* UApplicationGetTicketProxy::GetTicket(UDiscordApplicationManager* ApplicationManager)
{
	UApplicationGetTicketProxy* const Proxy = NewObject<UApplicationGetTicketProxy>();

	Proxy->Manager = ApplicationManager;

	return Proxy;
}

void UApplicationGetTicketProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to get ticket: Manager is nullptr."), ELogVerbosity::Error);
		OnResultInternal(EDiscordResult::InternalError, TEXT(""));
		return;
	}

	Manager->GetTicket(FOnTicketReceived::CreateUObject(this, &UApplicationGetTicketProxy::OnResultInternal));
}

void UApplicationGetTicketProxy::OnResultInternal(EDiscordResult Result, const FString& Ticket)
{
	GET_EVENT_FOR_RESULT(OnTicketReceived, OnError).Broadcast(Result, Ticket);
	SetReadyToDestroy();
}

#undef GET_EVENT_FOR_RESULT
