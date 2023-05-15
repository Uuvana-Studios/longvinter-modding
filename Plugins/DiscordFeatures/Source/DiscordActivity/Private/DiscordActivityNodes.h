// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordActivityManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordActivityNodes.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordCallbackEvent, EDiscordResult, Result);

UCLASS()
class UBaseActivityAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	UDiscordActivityManager* Manager;
};

UCLASS()
class UUpdateActivityProxy : public UBaseActivityAsyncProxy
{
	GENERATED_BODY()
public:
	/**
	 * Update the user's current activity
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Activity", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Update Activity"))
	static UUpdateActivityProxy* UpdateActivity(UDiscordActivityManager* ActivityManager, const FDiscordActivity& Activity);

	virtual void Activate();

	/**
	 * Called when the activity has been updated.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnActivityUpdated;

	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnError;

private:
	FDiscordActivity Activity;

	UFUNCTION()
	void OnActivityUpdatedInternal(EDiscordResult Result);
};

UCLASS()
class UClearActivityProxy : public UBaseActivityAsyncProxy
{
	GENERATED_BODY()
public:
	/**
	 * Clears the current activity.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Activity", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Clear Activity"))
	static UClearActivityProxy* ClearActivity(UDiscordActivityManager* ActivityManager);

	virtual void Activate();

	/**
	 * Called when the activity has been cleared.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnActivityCleared;
	
	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnError;	

private:
	UFUNCTION()
	void OnActivityClearedInternal(EDiscordResult Result);
};


UCLASS()
class USendRequestReplyProxy : public UBaseActivityAsyncProxy
{
	GENERATED_BODY()
public:
	/**
	 * Clears the current activity.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Activity", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Send Request Reply"))
	static USendRequestReplyProxy* SendRequestReply(UDiscordActivityManager* ActivityManager, const int64 UserId, const EDiscordActivityJoinRequestReply Reply);

	virtual void Activate();

	/**
	 * Called when the activity has been cleared.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnRequestReply;

	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnError;

private:

	int64 UserId;
	EDiscordActivityJoinRequestReply Reply;

	UFUNCTION()
	void OnRequestReplyInternal(EDiscordResult Result);
};

UCLASS()
class USendInviteProxy : public UBaseActivityAsyncProxy
{
	GENERATED_BODY()
public:
	/**
	 * Clears the current activity.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Activity", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Send Invite"))
	static USendInviteProxy* SendInvite(UDiscordActivityManager* ActivityManager, const int64& UserId, const EDiscordActivityActionType Action, const FString& Content);

	virtual void Activate();

	/**
	 * Called when the activity has been cleared.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnInviteSent;

	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnError;

private:

	int64 UserId;
	EDiscordActivityActionType Action;
	FString Content;

	UFUNCTION()
	void OnInviteSentInternal(EDiscordResult Result);
};

UCLASS()
class UAcceptInvite : public UBaseActivityAsyncProxy
{
	GENERATED_BODY()
public:
	/**
	 * Clears the current activity.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Activity", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Accept Invite"))
	static UAcceptInvite* AcceptInvite(UDiscordActivityManager* ActivityManager, const int64& UserId);

	virtual void Activate();

	/**
	 * Called when the activity has been cleared.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnInviteAccepted;

	UPROPERTY(BlueprintAssignable)
	FDiscordCallbackEvent OnError;

private:

	int64 UserId;

	UFUNCTION()
	void OnInviteAcceptedInternal(EDiscordResult Result);
};


