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
#include "Interfaces/OnlineGameActivityInterface.h"

#include "OnlineGameActivitySubsystem.generated.h"

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGameActivity_GameActivityActivationRequested_BP, const FUniqueNetId&, LocalUserId, const FString&, ActivityId, const FOnlineSessionSearchResult*, SessionInfo);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameActivitySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineGameActivity, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineGameActivity, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineGameActivity, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineGameActivitySubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|GameActivity")
    bool IsSubsystemAvailable();

    // @generator-incompatible
    // void StartActivity(const FUniqueNetId& LocalUserId, const FString& ActivityId, const FOnlineEventParms& Parms, FOnStartActivityComplete CompletionDelegate);

    // @generator-incompatible
    // void EndActivity(const FUniqueNetId& LocalUserId, const FString& ActivityId, EOnlineActivityOutcome ActivityOutcome, const FOnlineEventParms& Parms, FOnEndActivityComplete CompletionDelegate);

    friend class UOnlineGameActivitySubsystemSetActivityAvailability;
    
    friend class UOnlineGameActivitySubsystemSetActivityPriority;
    
    // @generator-incompatible
    // void UpdatePlayerLocation(const FUniqueNetId& LocalUserId, TOptional<FOnlineActivityPlayerLocation>& ActivityPlayerLocation);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineGameActivitySubsystemSetActivityAvailabilityCallbackPin, FUniqueNetIdRepl, LocalUserId, FOnlineErrorInfo, Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameActivitySubsystemSetActivityAvailability : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameActivitySubsystemSetActivityAvailabilityCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameActivitySubsystemSetActivityAvailabilityCallbackPin OnSetActivityAvailabilityComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameActivity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameActivitySubsystemSetActivityAvailability* SetActivityAvailability(
        UOnlineGameActivitySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FString ActivityId, bool bEnabled
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineGameActivitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSetActivityAvailabilityComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FString __Store__ActivityId;

UPROPERTY()
    bool __Store__bEnabled;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineGameActivitySubsystemSetActivityPriorityCallbackPin, FUniqueNetIdRepl, LocalUserId, FOnlineErrorInfo, Status);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGameActivitySubsystemSetActivityPriority : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineGameActivitySubsystemSetActivityPriorityCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineGameActivitySubsystemSetActivityPriorityCallbackPin OnSetActivityPriorityComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|GameActivity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineGameActivitySubsystemSetActivityPriority* SetActivityPriority(
        UOnlineGameActivitySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, TMap<FString, int32> PrioritizedActivities
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineGameActivitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSetActivityPriorityComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    TMap<FString, int32> __Store__PrioritizedActivities;
};
