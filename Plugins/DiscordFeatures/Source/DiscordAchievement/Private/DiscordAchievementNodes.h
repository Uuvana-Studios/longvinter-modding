// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordAchievementManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordAchievementNodes.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordResultEvent, EDiscordResult, Result);

UCLASS(Abstract)
class UBaseAchievementAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordResultEvent OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FDiscordResultEvent OnError;

protected:
	UPROPERTY()
	UDiscordAchievementManager* Manager;

	FDiscordResultCallback CreateCallback()
	{
		return FDiscordResultCallback::CreateUObject(this, &UBaseAchievementAsyncProxy::OnResult);
	}

	bool CheckManager()
	{
		if (!Manager)
		{
			FFrame::KismetExecutionMessage(TEXT("Achievement function Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
			OnResult(EDiscordResult::InternalError);
			return false;
		}
		return true;
	}

	UFUNCTION()
	void OnResult(EDiscordResult Result)
	{
		(Result == EDiscordResult::Ok ? OnSuccess : OnError).Broadcast(Result);
		SetReadyToDestroy();
	}
};

UCLASS()
class USetUserAchievementProxy : public UBaseAchievementAsyncProxy
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Discord|Achievement", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Set User Achievement"))
	static USetUserAchievementProxy* SetUserAchievement(UDiscordAchievementManager* AchievementManager, const int64 AchievementId, uint8 PercentComplete)
	{
		USetUserAchievementProxy* Proxy = NewObject<USetUserAchievementProxy>();

		Proxy->Manager = AchievementManager;
		Proxy->Achievement = AchievementId;
		Proxy->Percent = PercentComplete;

		return Proxy;
	}

	virtual void Activate()
	{
		if (CheckManager())
		{
			Manager->SetUserAchievement(Achievement, Percent, CreateCallback());
		}
	}

private:
	int64 Achievement;
	uint8 Percent;
};

UCLASS()
class UFetchUserAchievementsProxy : public UBaseAchievementAsyncProxy
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Discord|Achievement", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Fetch User Achievement"))
	static UFetchUserAchievementsProxy* FetchUserAchievements(UDiscordAchievementManager* AchievementManager)
	{
		UFetchUserAchievementsProxy* Proxy = NewObject<UFetchUserAchievementsProxy >();

		Proxy->Manager = AchievementManager;

		return Proxy;
	}

	virtual void Activate()
	{
		if (CheckManager())
		{
			Manager->FetchUserAchievements(CreateCallback());
		}
	}
};

