// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordApplicationManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordApplicationNodes.generated.h"

UCLASS(Abstract)
class UBaseApplicationAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	UDiscordApplicationManager* Manager;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDiscordOnOAuth2TokenReceived, EDiscordResult, Result, const FDiscordOAuth2Token&, Token);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FDiscordOnValidateOrExit,		EDiscordResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDiscordOnTickerReceived,		EDiscordResult, Result, const FString&, Ticket);


UCLASS()
class UApplicationGetOAuth2TokenProxy : public UBaseApplicationAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordOnOAuth2TokenReceived OnOAuth2TokenReceived;

	UPROPERTY(BlueprintAssignable)
	FDiscordOnOAuth2TokenReceived OnError;

	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Application", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get OAuth2 Token"))
	static UApplicationGetOAuth2TokenProxy* GetOAuth2Token(UDiscordApplicationManager* ApplicationManager);

private:
	UFUNCTION()
	void OnOAuth2TokenReceivedInternal(EDiscordResult Result, const FDiscordOAuth2Token& Token);
};

UCLASS()
class UApplicationValidateOrExitProxy : public UBaseApplicationAsyncProxy
{
	GENERATED_BODY()
public:
	// The game will keep running
	UPROPERTY(BlueprintAssignable)
	FDiscordOnValidateOrExit OnValidated;

	// The game is going to close
	UPROPERTY(BlueprintAssignable)
	FDiscordOnValidateOrExit OnExit;

	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Application", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Validate Or Exit"))
	static UApplicationValidateOrExitProxy* ValidateOrExit(UDiscordApplicationManager* ApplicationManager);

private:
	UFUNCTION()
	void OnResultInternal(EDiscordResult Result);
};


UCLASS()
class UApplicationGetTicketProxy : public UBaseApplicationAsyncProxy
{
	GENERATED_BODY()
public:
	// The game will keep running
	UPROPERTY(BlueprintAssignable)
	FDiscordOnTickerReceived OnTicketReceived;

	// Discord Result is not OK
	UPROPERTY(BlueprintAssignable)
	FDiscordOnTickerReceived OnError;

	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Application", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get Ticket"))
	static UApplicationGetTicketProxy* GetTicket(UDiscordApplicationManager* ApplicationManager);

private:
	UFUNCTION()
	void OnResultInternal(EDiscordResult Result, const FString& Ticket);
};


