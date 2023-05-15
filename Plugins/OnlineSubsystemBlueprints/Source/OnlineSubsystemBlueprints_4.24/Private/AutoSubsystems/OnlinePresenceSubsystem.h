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
#include "Interfaces/OnlinePresenceInterface.h"

#include "OnlinePresenceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPresence_PresenceReceived_BP, FUniqueNetIdRepl, UserId, FOnlineUserPresenceData, Presence);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPresence_PresenceArrayUpdated_BP, FUniqueNetIdRepl, UserId, const TArray<FOnlineUserPresenceData>&, NewPresenceArray);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePresenceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlinePresence, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnPresenceReceived;
    FDelegateHandle DelegateHandle_OnPresenceArrayUpdated;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlinePresenceSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Presence")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Presence")
    FPresence_PresenceReceived_BP OnPresenceReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Presence")
    FPresence_PresenceArrayUpdated_BP OnPresenceArrayUpdated;

    friend class UOnlinePresenceSubsystemSetPresence;
    
    friend class UOnlinePresenceSubsystemQueryPresence;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Presence")
    EOnlineCachedResult_ GetCachedPresence(FUniqueNetIdRepl User, FOnlineUserPresenceData& OutPresence);

    UFUNCTION(BlueprintCallable, Category = "Online|Presence")
    EOnlineCachedResult_ GetCachedPresenceForApp(FUniqueNetIdRepl LocalUserId, FUniqueNetIdRepl User, FString AppId, FOnlineUserPresenceData& OutPresence);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlinePresenceSubsystemSetPresenceCallbackPin, FUniqueNetIdRepl, UserId, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePresenceSubsystemSetPresence : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemSetPresenceCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemSetPresenceCallbackPin OnPresenceTaskComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Presence", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePresenceSubsystemSetPresence* SetPresence(
        UOnlinePresenceSubsystem* Subsystem
        ,FUniqueNetIdRepl User, FOnlineUserPresenceStatusData Status
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePresenceSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnPresenceTaskComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__User;

UPROPERTY()
    FOnlineUserPresenceStatusData __Store__Status;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlinePresenceSubsystemQueryPresenceCallbackPin, FUniqueNetIdRepl, UserId, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePresenceSubsystemQueryPresence : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemQueryPresenceCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePresenceSubsystemQueryPresenceCallbackPin OnPresenceTaskComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Presence", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePresenceSubsystemQueryPresence* QueryPresence(
        UOnlinePresenceSubsystem* Subsystem
        ,FUniqueNetIdRepl User
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePresenceSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnPresenceTaskComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__User;
};
