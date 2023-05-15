// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DiscordUserManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordUserNodes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetUserEvent, EDiscordResult, Result, const FDiscordUser&, User);

UCLASS(MinimalAPI)
class UGetUserProxy final : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnGetUserEvent OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FOnGetUserEvent OnError;

	virtual void Activate()
	{
		if (!Manager)
		{
			FFrame::KismetExecutionMessage(TEXT("Read Async Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
			OnResult(EDiscordResult::InternalError, {});
			return;
		}

		Manager->GetUser(UserId, FDiscordUserCallback::CreateUObject(this, &UGetUserProxy::OnResult));
	}


	UFUNCTION(BlueprintCallable, Category = "Discord|User", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get User"))
	static UGetUserProxy* GetUser(UDiscordUserManager* UserManager, const int64 UserId)
	{
		UGetUserProxy* const Proxy = NewObject<UGetUserProxy>();

		Proxy->Manager = UserManager;
		Proxy->UserId  = UserId;

		return Proxy;
	}

private:
	UPROPERTY()
	UDiscordUserManager* Manager;

	int64 UserId;

	void OnResult(EDiscordResult Result, const FDiscordUser& User)
	{
		(EDiscordResult::Ok == Result ? OnSuccess : OnError).Broadcast(Result, User);
		SetReadyToDestroy();
	}
};
