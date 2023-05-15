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
#include "Interfaces/OnlineSessionInterface.h"

#include "OnlineSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_CreateSessionComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_StartSessionComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_UpdateSessionComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_EndSessionComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_DestroySessionComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_MatchmakingComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_CancelMatchmakingComplete_BP, FName, SessionName, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSession_FindSessionsComplete_BP, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSession_CancelFindSessionsComplete_BP, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSession_PingSearchResultsComplete_BP, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_JoinSessionComplete_BP, FName, SessionName, EOnJoinSessionCompleteResult_, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSession_FindFriendSessionComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, const TArray<FOnlineSessionSearchResultBP>&, FriendSearchResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSession_SessionUserInviteAccepted_BP, bool, bWasSuccessful, int32, ControllerId, FUniqueNetIdRepl, UserId, FOnlineSessionSearchResultBP, InviteResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSession_SessionInviteReceived_BP, FUniqueNetIdRepl, UserId, FUniqueNetIdRepl, FromId, FString, AppId, FOnlineSessionSearchResultBP, InviteResult);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSession_RegisterPlayersComplete_BP, FName, SessionName, const TArray<FUniqueNetIdRepl>&, PlayerIds, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSession_UnregisterPlayersComplete_BP, FName, SessionName, const TArray<FUniqueNetIdRepl>&, PlayerIds, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSession_SessionFailure_BP, FUniqueNetIdRepl, PlayerId, ESessionFailure_, FailureType);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineSession, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnCreateSessionComplete;
    FDelegateHandle DelegateHandle_OnStartSessionComplete;
    FDelegateHandle DelegateHandle_OnUpdateSessionComplete;
    FDelegateHandle DelegateHandle_OnEndSessionComplete;
    FDelegateHandle DelegateHandle_OnDestroySessionComplete;
    FDelegateHandle DelegateHandle_OnMatchmakingComplete;
    FDelegateHandle DelegateHandle_OnCancelMatchmakingComplete;
    FDelegateHandle DelegateHandle_OnFindSessionsComplete;
    FDelegateHandle DelegateHandle_OnCancelFindSessionsComplete;
    FDelegateHandle DelegateHandle_OnPingSearchResultsComplete;
    FDelegateHandle DelegateHandle_OnJoinSessionComplete;
    TArray<FDelegateHandle> DelegateHandle_OnFindFriendSessionComplete;
    FDelegateHandle DelegateHandle_OnSessionUserInviteAccepted;
    FDelegateHandle DelegateHandle_OnSessionInviteReceived;
    FDelegateHandle DelegateHandle_OnRegisterPlayersComplete;
    FDelegateHandle DelegateHandle_OnUnregisterPlayersComplete;
    FDelegateHandle DelegateHandle_OnSessionFailure;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineSessionSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Session")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_CreateSessionComplete_BP OnCreateSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_StartSessionComplete_BP OnStartSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_UpdateSessionComplete_BP OnUpdateSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_EndSessionComplete_BP OnEndSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_DestroySessionComplete_BP OnDestroySessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_MatchmakingComplete_BP OnMatchmakingComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_CancelMatchmakingComplete_BP OnCancelMatchmakingComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_FindSessionsComplete_BP OnFindSessionsComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_CancelFindSessionsComplete_BP OnCancelFindSessionsComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_PingSearchResultsComplete_BP OnPingSearchResultsComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_JoinSessionComplete_BP OnJoinSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_FindFriendSessionComplete_BP OnFindFriendSessionComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_SessionUserInviteAccepted_BP OnSessionUserInviteAccepted;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_SessionInviteReceived_BP OnSessionInviteReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_RegisterPlayersComplete_BP OnRegisterPlayersComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_UnregisterPlayersComplete_BP OnUnregisterPlayersComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Session")
    FSession_SessionFailure_BP OnSessionFailure;

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    FUniqueNetIdRepl CreateSessionIdFromString(FString SessionIdStr);

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    UNamedOnlineSession* GetNamedSession(FName SessionName);

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    void RemoveNamedSession(FName SessionName);

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool HasPresenceSession();

    UFUNCTION(BlueprintPure, Category = "Online|Session")
    EOnlineSessionState_ GetSessionState(FName SessionName);

    // @generator-excluded-with-pick
    // bool CreateSession(int32 HostingPlayerNum, FName SessionName, const FOnlineSessionSettings& NewSessionSettings);

    friend class UOnlineSessionSubsystemCreateSession;
    
    friend class UOnlineSessionSubsystemStartSession;
    
    friend class UOnlineSessionSubsystemUpdateSession;
    
    friend class UOnlineSessionSubsystemEndSession;
    
    friend class UOnlineSessionSubsystemDestroySession;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool IsPlayerInSession(FName SessionName, FUniqueNetIdRepl UniqueId);

    friend class UOnlineSessionSubsystemStartMatchmaking;
    
    // @generator-excluded-with-pick
    // bool CancelMatchmaking(int32 SearchingPlayerNum, FName SessionName);

    friend class UOnlineSessionSubsystemCancelMatchmaking;
    
    // @generator-excluded-with-pick
    // bool FindSessions(int32 SearchingPlayerNum, const TSharedRef<FOnlineSessionSearch>& SearchSettings);

    friend class UOnlineSessionSubsystemFindSessions;
    
    // @generator-excluded-with-pick
    // bool FindSessionById(const FUniqueNetId& SearchingUserId, const FUniqueNetId& SessionId, const FUniqueNetId& FriendId);

    friend class UOnlineSessionSubsystemFindSessionById;
    
    friend class UOnlineSessionSubsystemCancelFindSessions;
    
    friend class UOnlineSessionSubsystemPingSearchResults;
    
    // @generator-excluded-with-pick
    // bool JoinSession(int32 LocalUserNum, FName SessionName, const FOnlineSessionSearchResult& DesiredSession);

    friend class UOnlineSessionSubsystemJoinSession;
    
    friend class UOnlineSessionSubsystemFindFriendSession;
    
    // @generator-excluded-with-pick
    // bool FindFriendSession(const FUniqueNetId& LocalUserId, const FUniqueNetId& Friend);

    // @generator-excluded-with-pick
    // bool FindFriendSession(const FUniqueNetId& LocalUserId, const TArray<TSharedRef<const FUniqueNetId>>& FriendList);

    // @generator-excluded-with-pick
    // bool SendSessionInviteToFriend(int32 LocalUserNum, FName SessionName, const FUniqueNetId& Friend);

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool SendSessionInviteToFriend(FUniqueNetIdRepl LocalUserId, FName SessionName, FUniqueNetIdRepl Friend);

    // @generator-excluded-with-pick
    // bool SendSessionInviteToFriends(int32 LocalUserNum, FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& Friends);

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    bool SendSessionInviteToFriends(FUniqueNetIdRepl LocalUserId, FName SessionName, TArray<FUniqueNetIdRepl> Friends);

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    UOnlineSessionSettings* GetSessionSettings(FName SessionName);

    friend class UOnlineSessionSubsystemRegisterPlayers;
    
    friend class UOnlineSessionSubsystemUnregisterPlayers;
    
    friend class UOnlineSessionSubsystemRegisterLocalPlayer;
    
    friend class UOnlineSessionSubsystemUnregisterLocalPlayer;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    int32 GetNumSessions();

    UFUNCTION(BlueprintCallable, Category = "Online|Session")
    void DumpSessionState();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemCreateSessionCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemCreateSession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemCreateSessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemCreateSessionCallbackPin OnCreateSessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemCreateSession* CreateSession(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl HostingPlayerId, FName SessionName, FOnlineSessionSettingsBP NewSessionSettings
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnCreateSessionComplete;
    void HandleCallback_OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__HostingPlayerId;

UPROPERTY()
    FName __Store__SessionName;

UPROPERTY()
    FOnlineSessionSettingsBP __Store__NewSessionSettings;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemStartSessionCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemStartSession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemStartSessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemStartSessionCallbackPin OnStartSessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemStartSession* StartSession(
        UOnlineSessionSubsystem* Subsystem
        ,FName SessionName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnStartSessionComplete;
    void HandleCallback_OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    FName __Store__SessionName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemUpdateSessionCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemUpdateSession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemUpdateSessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemUpdateSessionCallbackPin OnUpdateSessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemUpdateSession* UpdateSession(
        UOnlineSessionSubsystem* Subsystem
        ,FName SessionName, FOnlineSessionSettingsBP UpdatedSessionSettings, bool bShouldRefreshOnlineData
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnUpdateSessionComplete;
    void HandleCallback_OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    FName __Store__SessionName;

UPROPERTY()
    FOnlineSessionSettingsBP __Store__UpdatedSessionSettings;

UPROPERTY()
    bool __Store__bShouldRefreshOnlineData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemEndSessionCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemEndSession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemEndSessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemEndSessionCallbackPin OnEndSessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemEndSession* EndSession(
        UOnlineSessionSubsystem* Subsystem
        ,FName SessionName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnEndSessionComplete;
    void HandleCallback_OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    FName __Store__SessionName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemDestroySessionCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemDestroySession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemDestroySessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemDestroySessionCallbackPin OnDestroySessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemDestroySession* DestroySession(
        UOnlineSessionSubsystem* Subsystem
        ,FName SessionName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    FName __Store__SessionName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemStartMatchmakingCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemStartMatchmaking : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemStartMatchmakingCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemStartMatchmakingCallbackPin OnMatchmakingComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemStartMatchmaking* StartMatchmaking(
        UOnlineSessionSubsystem* Subsystem
        ,TArray<FUniqueNetIdRepl> LocalPlayers, FName SessionName, FOnlineSessionSettingsBP NewSessionSettings, UOnlineSessionSearch* SearchSettings
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnMatchmakingComplete;
    void HandleCallback_OnMatchmakingComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__LocalPlayers;

UPROPERTY()
    FName __Store__SessionName;

UPROPERTY()
    FOnlineSessionSettingsBP __Store__NewSessionSettings;

UPROPERTY()
    UOnlineSessionSearch* __Store__SearchSettings;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemCancelMatchmakingCallbackPin, FName, SessionName, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemCancelMatchmaking : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemCancelMatchmakingCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemCancelMatchmakingCallbackPin OnCancelMatchmakingComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemCancelMatchmaking* CancelMatchmaking(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl SearchingPlayerId, FName SessionName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnCancelMatchmakingComplete;
    void HandleCallback_OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__SearchingPlayerId;

UPROPERTY()
    FName __Store__SessionName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineSessionSubsystemFindSessionsCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemFindSessions : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemFindSessionsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemFindSessionsCallbackPin OnFindSessionsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemFindSessions* FindSessions(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl SearchingPlayerId, UOnlineSessionSearch* SearchSettings
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnFindSessionsComplete;
    void HandleCallback_OnFindSessionsComplete(bool bWasSuccessful);

    UPROPERTY()
    FUniqueNetIdRepl __Store__SearchingPlayerId;

UPROPERTY()
    UOnlineSessionSearch* __Store__SearchSettings;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionSubsystemFindSessionByIdCallbackPin, int32, LocalUserNum, bool, bWasSuccessful, FOnlineSessionSearchResultBP, SearchResult);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemFindSessionById : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemFindSessionByIdCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemFindSessionByIdCallbackPin OnSingleSessionResultComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemFindSessionById* FindSessionById(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl SearchingUserId, FUniqueNetIdRepl SessionId, FUniqueNetIdRepl FriendId, FString UserData
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);

    UPROPERTY()
    FUniqueNetIdRepl __Store__SearchingUserId;

UPROPERTY()
    FUniqueNetIdRepl __Store__SessionId;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__UserData;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineSessionSubsystemCancelFindSessionsCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemCancelFindSessions : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemCancelFindSessionsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemCancelFindSessionsCallbackPin OnCancelFindSessionsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemCancelFindSessions* CancelFindSessions(
        UOnlineSessionSubsystem* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnCancelFindSessionsComplete;
    void HandleCallback_OnCancelFindSessionsComplete(bool bWasSuccessful);

    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineSessionSubsystemPingSearchResultsCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemPingSearchResults : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemPingSearchResultsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemPingSearchResultsCallbackPin OnPingSearchResultsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemPingSearchResults* PingSearchResults(
        UOnlineSessionSubsystem* Subsystem
        ,FOnlineSessionSearchResultBP SearchResult
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnPingSearchResultsComplete;
    void HandleCallback_OnPingSearchResultsComplete(bool bWasSuccessful);

    UPROPERTY()
    FOnlineSessionSearchResultBP __Store__SearchResult;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemJoinSessionCallbackPin, FName, SessionName, EOnJoinSessionCompleteResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemJoinSession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemJoinSessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemJoinSessionCallbackPin OnJoinSessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemJoinSession* JoinSession(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FName SessionName, FOnlineSessionSearchResultBP DesiredSession
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnJoinSessionComplete;
    void HandleCallback_OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FName __Store__SessionName;

UPROPERTY()
    FOnlineSessionSearchResultBP __Store__DesiredSession;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemFindFriendSessionCallbackPin, bool, bWasSuccessful, const TArray<FOnlineSessionSearchResultBP>&, FriendSearchResult);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemFindFriendSession : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemFindFriendSessionCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemFindFriendSessionCallbackPin OnFindFriendSessionComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemFindFriendSession* FindFriendSession(
        UOnlineSessionSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl Friend
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnFindFriendSessionComplete;
    void HandleCallback_OnFindFriendSessionComplete(int32 LocalUserNumCb, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResult);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__Friend;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionSubsystemRegisterPlayersCallbackPin, FName, SessionName, const TArray<FUniqueNetIdRepl>&, PlayerIds, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemRegisterPlayers : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemRegisterPlayersCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemRegisterPlayersCallbackPin OnRegisterPlayersComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemRegisterPlayers* RegisterPlayers(
        UOnlineSessionSubsystem* Subsystem
        ,FName SessionName, TArray<FUniqueNetIdRepl> Players, bool bWasInvited
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnRegisterPlayersComplete;
    void HandleCallback_OnRegisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);

    UPROPERTY()
    FName __Store__SessionName;

UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__Players;

UPROPERTY()
    bool __Store__bWasInvited;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineSessionSubsystemUnregisterPlayersCallbackPin, FName, SessionName, const TArray<FUniqueNetIdRepl>&, PlayerIds, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemUnregisterPlayers : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemUnregisterPlayersCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemUnregisterPlayersCallbackPin OnUnregisterPlayersComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemUnregisterPlayers* UnregisterPlayers(
        UOnlineSessionSubsystem* Subsystem
        ,FName SessionName, TArray<FUniqueNetIdRepl> Players
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnUnregisterPlayersComplete;
    void HandleCallback_OnUnregisterPlayersComplete(FName SessionName, const TArray<TSharedRef<const FUniqueNetId>>& PlayerIds, bool bWasSuccessful);

    UPROPERTY()
    FName __Store__SessionName;

UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__Players;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemRegisterLocalPlayerCallbackPin, FUniqueNetIdRepl, Param1, EOnJoinSessionCompleteResult_, Param2);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemRegisterLocalPlayer : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemRegisterLocalPlayerCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemRegisterLocalPlayerCallbackPin OnRegisterLocalPlayerComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemRegisterLocalPlayer* RegisterLocalPlayer(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl PlayerId, FName SessionName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRegisterLocalPlayerComplete(const FUniqueNetId& Param1, EOnJoinSessionCompleteResult::Type Param2);

    UPROPERTY()
    FUniqueNetIdRepl __Store__PlayerId;

UPROPERTY()
    FName __Store__SessionName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineSessionSubsystemUnregisterLocalPlayerCallbackPin, FUniqueNetIdRepl, Param1, bool, Param2);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSessionSubsystemUnregisterLocalPlayer : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemUnregisterLocalPlayerCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineSessionSubsystemUnregisterLocalPlayerCallbackPin OnUnregisterLocalPlayerComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineSessionSubsystemUnregisterLocalPlayer* UnregisterLocalPlayer(
        UOnlineSessionSubsystem* Subsystem
        ,FUniqueNetIdRepl PlayerId, FName SessionName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineSessionSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnUnregisterLocalPlayerComplete(const FUniqueNetId& Param1, const bool Param2);

    UPROPERTY()
    FUniqueNetIdRepl __Store__PlayerId;

UPROPERTY()
    FName __Store__SessionName;
};
