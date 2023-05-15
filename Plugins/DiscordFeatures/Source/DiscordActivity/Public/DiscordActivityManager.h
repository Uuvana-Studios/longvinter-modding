// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordActivityManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam   (FOnActivityJoin,        const FString&,      Secret);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam   (FOnActivitySpectate,    const FString&,      Secret);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam   (FOnActivityJoinRequest, const FDiscordUser&, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActivityInvite,      const EDiscordActivityActionType, Action, const FDiscordUser&, User, const FDiscordActivity&, Activity);

UCLASS()
class DISCORDACTIVITY_API UDiscordActivityManager : public UDiscordManager
{
	GENERATED_BODY()
public:

    UPROPERTY(BlueprintAssignable)
    FOnActivityJoin         OnActivityJoin;
    UPROPERTY(BlueprintAssignable)
    FOnActivitySpectate     OnActivitySpectate;
    UPROPERTY(BlueprintAssignable)
    FOnActivityJoinRequest  OnActivityJoinRequest;
    UPROPERTY(BlueprintAssignable)
    FOnActivityInvite       OnActivityInvite;

public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Activity", BlueprintPure, meta = (CompactNodeTitle = "ACTIVITY MANAGER", BlueprintAutocast))
	static UPARAM(DisplayName = "Activity Manager") UDiscordActivityManager* GetActivityManager(UDiscordCore* DiscordCore);
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Activity")
	void RegisterCommand(const FString& Command);

	UFUNCTION(BlueprintCallable, Category = "Discord|Activity")
	void RegisterSteam(const int64 SteamId);

	void UpdateActivity(const FDiscordActivity& Activity, FDiscordResultCallback OnActivityUpdated = FDiscordResultCallback());

    void ClearActivity(FDiscordResultCallback OnActivityCleared = FDiscordResultCallback());

    void SendRequestReply(const int64 UserId, const EDiscordActivityJoinRequestReply& Reply, FDiscordResultCallback OnResponded = FDiscordResultCallback());

    void SendInvite(const int64 UserId, const EDiscordActivityActionType Action, const FString& Content, FDiscordResultCallback OnInviteSent = FDiscordResultCallback());

    void AcceptInvite(const int64 UserId, FDiscordResultCallback OnAcceptInviteResponse = FDiscordResultCallback());

private:
    void OnActivityJoinInternal(const char* secret);
    void OnActivitySpectateInternal(const char* secret);
    void OnActivityJoinRequestInternal(FRawDiscord::DiscordUser* user);
    void OnActivityInviteInternal(FRawDiscord::EDiscordActivityActionType type, FRawDiscord::DiscordUser* user, FRawDiscord::DiscordActivity* activity);
};