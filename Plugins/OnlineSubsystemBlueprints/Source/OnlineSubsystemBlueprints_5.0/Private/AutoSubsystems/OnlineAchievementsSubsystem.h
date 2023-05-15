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
#include "Interfaces/OnlineAchievementsInterface.h"

#include "OnlineAchievementsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAchievements_AchievementUnlocked_BP, FUniqueNetIdRepl, Param1, FString, Param2);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAchievementsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineAchievements, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineAchievements, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineAchievements, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnAchievementUnlocked;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineAchievementsSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Achievements")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Achievements")
    FAchievements_AchievementUnlocked_BP OnAchievementUnlocked;

    // @generator-incompatible
    // void WriteAchievements(const FUniqueNetId& PlayerId, FOnlineAchievementsWriteRef& WriteObject, const FOnAchievementsWrittenDelegate& Delegate);

    friend class UOnlineAchievementsSubsystemQueryAchievements;
    
    friend class UOnlineAchievementsSubsystemQueryAchievementDescriptions;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Achievements")
    EOnlineCachedResult_ GetCachedAchievement(FUniqueNetIdRepl PlayerId, FString AchievementId, FOnlineAchievementBP& OutAchievement);

    UFUNCTION(BlueprintCallable, Category = "Online|Achievements")
    EOnlineCachedResult_ GetCachedAchievements(FUniqueNetIdRepl PlayerId, TArray<FOnlineAchievementBP>& OutAchievements);

    UFUNCTION(BlueprintCallable, Category = "Online|Achievements")
    EOnlineCachedResult_ GetCachedAchievementDescription(FString AchievementId, FOnlineAchievementDescBP& OutAchievementDesc);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineAchievementsSubsystemQueryAchievementsCallbackPin, FUniqueNetIdRepl, LocalUserId, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAchievementsSubsystemQueryAchievements : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineAchievementsSubsystemQueryAchievementsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineAchievementsSubsystemQueryAchievementsCallbackPin OnQueryAchievementsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Achievements", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineAchievementsSubsystemQueryAchievements* QueryAchievements(
        UOnlineAchievementsSubsystem* Subsystem
        ,FUniqueNetIdRepl PlayerId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineAchievementsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryAchievementsComplete(const FUniqueNetId& LocalUserId, const bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__PlayerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineAchievementsSubsystemQueryAchievementDescriptionsCallbackPin, FUniqueNetIdRepl, LocalUserId, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAchievementsSubsystemQueryAchievementDescriptions : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineAchievementsSubsystemQueryAchievementDescriptionsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineAchievementsSubsystemQueryAchievementDescriptionsCallbackPin OnQueryAchievementsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Achievements", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineAchievementsSubsystemQueryAchievementDescriptions* QueryAchievementDescriptions(
        UOnlineAchievementsSubsystem* Subsystem
        ,FUniqueNetIdRepl PlayerId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineAchievementsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryAchievementsComplete(const FUniqueNetId& LocalUserId, const bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__PlayerId;
};
