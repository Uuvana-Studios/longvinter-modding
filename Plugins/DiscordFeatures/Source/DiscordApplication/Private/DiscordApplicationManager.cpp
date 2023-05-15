// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordApplicationManager.h"
#include "DiscordManagerUtils.h"

/* static */ UDiscordApplicationManager* UDiscordApplicationManager::GetApplicationManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->ApplicationManager)
	{
		return Cast<UDiscordApplicationManager>(DiscordCore->ApplicationManager);
	}

	UDiscordApplicationManager* const Manager = NewObject<UDiscordApplicationManager>();

	DiscordCore->ApplicationManager = Manager;

	Manager->DiscordCore = DiscordCore;

	return Manager;
}

FString UDiscordApplicationManager::GetCurrentLocale()
{
	FRawDiscord::DiscordLocale Locale;
	FMemory::Memzero(Locale);

	APPLICATION_CALL_RAW_CHECKED(get_current_locale, &Locale);

	return UTF8_TO_TCHAR(Locale);
}

FString UDiscordApplicationManager::GetCurrentBranch()
{
	FRawDiscord::DiscordBranch Branch;
	FMemory::Memzero(Branch);

	APPLICATION_CALL_RAW_CHECKED(get_current_branch, &Branch);

	return UTF8_TO_TCHAR(Branch);
}

void UDiscordApplicationManager::GetOAuth2Token(const FOnOAuth2TokenReceived& OnOAuth2TokenReceived)
{
	const auto Callback = [](void* Data, FRawDiscord::EDiscordResult Result, FRawDiscord::DiscordOAuth2Token* RawToken)
	{
		FOnOAuth2TokenReceived* const Cb = static_cast<FOnOAuth2TokenReceived*>(Data);
		FDiscordOAuth2Token  Token;
		Token.AccessToken	= UTF8_TO_TCHAR(RawToken->access_token);
		Token.Expires		= RawToken->expires;
		Token.Scopes		= UTF8_TO_TCHAR(RawToken->scopes);

		Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result), Token);

		delete Cb;
	};
	APPLICATION_CALL_RAW_CHECKED(get_oauth2_token, new FOnOAuth2TokenReceived(OnOAuth2TokenReceived), Callback);
}

void UDiscordApplicationManager::ValidateOrExit(const FDiscordResultCallback& OnValidated)
{
	APPLICATION_CALL_RAW_CHECKED(validate_or_exit, new FDiscordResultCallback(OnValidated), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordApplicationManager::GetTicket(const FOnTicketReceived& OnTicketReceived)
{
	const auto Callback = [](void* const Data, const FRawDiscord::EDiscordResult Result, const char* Ticket)
	{
		FOnTicketReceived* const Cb = (FOnTicketReceived*)Data;

		Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result), UTF8_TO_TCHAR(Ticket));

		delete Cb;
	};

	APPLICATION_CALL_RAW_CHECKED(get_ticket, new FOnTicketReceived(OnTicketReceived), Callback);
}


