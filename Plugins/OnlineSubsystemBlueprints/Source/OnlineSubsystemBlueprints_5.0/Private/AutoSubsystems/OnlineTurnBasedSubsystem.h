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
#include "Interfaces/OnlineTurnBasedInterface.h"

#include "OnlineTurnBasedSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTurnBasedSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineTurnBased, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineTurnBased, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineTurnBased, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineTurnBasedSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|TurnBased")
    bool IsSubsystemAvailable();

    // @generator-incompatible
    // void SetMatchmakerDelegate(FTurnBasedMatchmakerDelegatePtr Delegate);

    // @generator-incompatible
    // void ShowMatchmaker(const FTurnBasedMatchRequest& MatchRequest);

    // @generator-incompatible
    // void SetEventDelegate(FTurnBasedEventDelegateWeakPtr Delegate);

    // @generator-incompatible
    // FTurnBasedEventDelegateWeakPtr GetEventDelegate();

    friend class UOnlineTurnBasedSubsystemLoadAllMatches;
    
    friend class UOnlineTurnBasedSubsystemLoadMatchWithID;
    
    // @generator-incompatible
    // FTurnBasedMatchPtr GetMatchWithID(FString MatchID);

    // @generator-incompatible
    // void RemoveMatch(FTurnBasedMatchRef Match, FRemoveMatchSignature RemoveMatchCallback);

    // @generator-incompatible
    // void RegisterTurnBasedMatchInterfaceObject(UObject* Object);

    // @generator-incompatible
    // UObject* GetTurnBasedMatchInterfaceObject();

    UFUNCTION(BlueprintCallable, Category = "Online|TurnBased")
    int32 GetMatchDataSize();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineTurnBasedSubsystemLoadAllMatchesCallbackPin, const TArray<FString>&, Param1, bool, Param2);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTurnBasedSubsystemLoadAllMatches : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineTurnBasedSubsystemLoadAllMatchesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineTurnBasedSubsystemLoadAllMatchesCallbackPin LoadTurnBasedMatchesSignature;

    UFUNCTION(BlueprintCallable, Category = "Online|TurnBased", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineTurnBasedSubsystemLoadAllMatches* LoadAllMatches(
        UOnlineTurnBasedSubsystem* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineTurnBasedSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_LoadTurnBasedMatchesSignature(const TArray<FString>& Param1, bool Param2);

    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineTurnBasedSubsystemLoadMatchWithIDCallbackPin, FString, Param1, bool, Param2);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTurnBasedSubsystemLoadMatchWithID : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineTurnBasedSubsystemLoadMatchWithIDCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineTurnBasedSubsystemLoadMatchWithIDCallbackPin LoadTurnBasedMatchWithIDSignature;

    UFUNCTION(BlueprintCallable, Category = "Online|TurnBased", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineTurnBasedSubsystemLoadMatchWithID* LoadMatchWithID(
        UOnlineTurnBasedSubsystem* Subsystem
        ,FString MatchID
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineTurnBasedSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_LoadTurnBasedMatchWithIDSignature(FString Param1, bool Param2);

    UPROPERTY()
    FString __Store__MatchID;
};
