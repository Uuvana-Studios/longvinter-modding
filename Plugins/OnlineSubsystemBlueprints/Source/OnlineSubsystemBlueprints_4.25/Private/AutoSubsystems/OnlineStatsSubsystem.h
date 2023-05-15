// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Types/OnlineUserRef.h"
#include "Types/FriendTypes.h"
#include "Types/OnlineAccountCredential.h"
#include "Types/OnlineErrorInfo.h"
#include "Types/OnlineRecentPlayerRef.h"
#include "Types/OnlineLoginStatus.h"
#include "Types/BlockedQueryResultInfo.h"
#include "Types/OnlineUserPrivilege.h"
#include "Types/MessageTypes.h"
#include "Types/ExternalUITypes.h"
#include "Types/OSBFileTypes.h"
#include "Types/OSBPartyTypes.h"
#include "Types/OSBLobbyTypes.h"
#include "Types/OSBSessionTypes.h"
#include "Types/OSBLeaderboardTypes.h"
#include "Types/OSBStatsTypes.h"
#include "Types/OSBAchievementTypes.h"
#include "Types/OSBVoiceChatTypes.h"
#include "Helpers/ArrayConversion.h"
#include "Helpers/UniqueNetIdConversion.h"
#include "Interfaces/OnlineStatsInterface.h"

#include "OnlineStatsSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineStatsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineStats, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineStatsSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Stats")
    bool IsSubsystemAvailable();

    // @generator-excluded-with-pick
    // void QueryStats(const TSharedRef<const FUniqueNetId> LocalUserId, const TSharedRef<const FUniqueNetId> StatsUser);

    friend class UOnlineStatsSubsystemQueryStats;
    
    UFUNCTION(BlueprintPure, Category = "Online|Stats")
    FOnlineStatsUserStatsBP GetStats(FUniqueNetIdRepl StatsUserId);

    friend class UOnlineStatsSubsystemUpdateStats;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineStatsSubsystemQueryStatsCallbackPin, FOnlineErrorInfo, ResultState, const TArray<FOnlineStatsUserStatsBP>&, UsersStatsResult);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineStatsSubsystemQueryStats : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineStatsSubsystemQueryStatsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineStatsSubsystemQueryStatsCallbackPin OnlineStatsQueryUsersStatsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Stats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineStatsSubsystemQueryStats* QueryStats(
        UOnlineStatsSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, TArray<FUniqueNetIdRepl> StatUsers, TArray<FString> StatNames
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineStatsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnlineStatsQueryUsersStatsComplete(const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__StatUsers;

UPROPERTY()
    TArray<FString> __Store__StatNames;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineStatsSubsystemUpdateStatsCallbackPin, FOnlineErrorInfo, ResultState);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineStatsSubsystemUpdateStats : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineStatsSubsystemUpdateStatsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineStatsSubsystemUpdateStatsCallbackPin OnlineStatsUpdateStatsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Stats", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineStatsSubsystemUpdateStats* UpdateStats(
        UOnlineStatsSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, TArray<FOnlineStatsUserUpdatedStatsBP> UpdatedUserStats
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineStatsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnlineStatsUpdateStatsComplete(const FOnlineError& ResultState);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    TArray<FOnlineStatsUserUpdatedStatsBP> __Store__UpdatedUserStats;
};
