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
#include "Interfaces/OnlineLeaderboardInterface.h"

#include "OnlineLeaderboardsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLeaderboards_LeaderboardReadComplete_BP, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLeaderboards_LeaderboardFlushComplete_BP, FName, SessionName, bool, bWasSuccessful);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnLeaderboardReadComplete;
    FDelegateHandle DelegateHandle_OnLeaderboardFlushComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineLeaderboardsSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Leaderboards")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Leaderboards")
    FLeaderboards_LeaderboardReadComplete_BP OnLeaderboardReadComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Leaderboards")
    FLeaderboards_LeaderboardFlushComplete_BP OnLeaderboardFlushComplete;

    friend class UOnlineLeaderboardsSubsystemReadLeaderboards;
    
    friend class UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends;
    
    friend class UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank;
    
    friend class UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards")
    void FreeStats(UOnlineLeaderboardRead* ReadObject);

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards")
    bool WriteLeaderboards(FName SessionName, FUniqueNetIdRepl Player, UOnlineLeaderboardWrite* WriteObject);

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards")
    bool FlushLeaderboards(FName SessionName);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineLeaderboardsSubsystemReadLeaderboardsCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardsSubsystemReadLeaderboards : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsCallbackPin OnLeaderboardReadComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLeaderboardsSubsystemReadLeaderboards* ReadLeaderboards(
        UOnlineLeaderboardsSubsystem* Subsystem
        ,TArray<FUniqueNetIdRepl> Players, UOnlineLeaderboardRead* ReadObject
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLeaderboardsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLeaderboardReadComplete;
    void HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful);

    UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__Players;

UPROPERTY()
    UOnlineLeaderboardRead* __Store__ReadObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineLeaderboardsSubsystemReadLeaderboardsForFriendsCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsForFriendsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsForFriendsCallbackPin OnLeaderboardReadComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends* ReadLeaderboardsForFriends(
        UOnlineLeaderboardsSubsystem* Subsystem
        ,int32 LocalUserNum, UOnlineLeaderboardRead* ReadObject
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLeaderboardsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLeaderboardReadComplete;
    void HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    UOnlineLeaderboardRead* __Store__ReadObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineLeaderboardsSubsystemReadLeaderboardsAroundRankCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsAroundRankCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsAroundRankCallbackPin OnLeaderboardReadComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank* ReadLeaderboardsAroundRank(
        UOnlineLeaderboardsSubsystem* Subsystem
        ,int32 Rank, int64 Range, UOnlineLeaderboardRead* ReadObject
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLeaderboardsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLeaderboardReadComplete;
    void HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful);

    UPROPERTY()
    int32 __Store__Rank;

UPROPERTY()
    int64 __Store__Range;

UPROPERTY()
    UOnlineLeaderboardRead* __Store__ReadObject;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineLeaderboardsSubsystemReadLeaderboardsAroundUserCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsAroundUserCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLeaderboardsSubsystemReadLeaderboardsAroundUserCallbackPin OnLeaderboardReadComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Leaderboards", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser* ReadLeaderboardsAroundUser(
        UOnlineLeaderboardsSubsystem* Subsystem
        ,FUniqueNetIdRepl Player, int64 Range, UOnlineLeaderboardRead* ReadObject
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLeaderboardsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLeaderboardReadComplete;
    void HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__Player;

UPROPERTY()
    int64 __Store__Range;

UPROPERTY()
    UOnlineLeaderboardRead* __Store__ReadObject;
};
