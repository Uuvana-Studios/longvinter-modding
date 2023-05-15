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
#include "Interfaces/OnlineFriendsInterface.h"

#include "OnlineFriendsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFriends_FriendsChange_BP, int32, LocalUserNum);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFriends_OutgoingInviteSent_BP, int32, LocalUserNum);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_InviteReceived_BP, FUniqueNetIdRepl, UserId, FUniqueNetIdRepl, FriendId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_InviteAccepted_BP, FUniqueNetIdRepl, UserId, FUniqueNetIdRepl, FriendId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_InviteRejected_BP, FUniqueNetIdRepl, UserId, FUniqueNetIdRepl, FriendId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_InviteAborted_BP, FUniqueNetIdRepl, UserId, FUniqueNetIdRepl, FriendId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_FriendRemoved_BP, FUniqueNetIdRepl, UserId, FUniqueNetIdRepl, FriendId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FFriends_RejectInviteComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, FriendId, FString, ListName, FString, ErrorStr);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FFriends_DeleteFriendComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, FriendId, FString, ListName, FString, ErrorStr);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FFriends_BlockedPlayerComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, UniqueId, FString, ListName, FString, ErrorStr);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FFriends_UnblockedPlayerComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, UniqueId, FString, ListName, FString, ErrorStr);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_BlockListChange_BP, int32, LocalUserNum, FString, ListName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FFriends_QueryRecentPlayersComplete_BP, FUniqueNetIdRepl, UserId, FString, Namespace, bool, bWasSuccessful, FString, Error);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFriends_QueryBlockedPlayersComplete_BP, FUniqueNetIdRepl, UserId, bool, bWasSuccessful, FString, Error);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFriends_RecentPlayersAdded_BP, FUniqueNetIdRepl, UserId, const TArray<UOnlineRecentPlayerRef*>&, AddedPlayers);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineFriends, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe>& InHandle);
    TArray<FDelegateHandle> DelegateHandle_OnFriendsChange;
    TArray<FDelegateHandle> DelegateHandle_OnOutgoingInviteSent;
    FDelegateHandle DelegateHandle_OnInviteReceived;
    FDelegateHandle DelegateHandle_OnInviteAccepted;
    FDelegateHandle DelegateHandle_OnInviteRejected;
    FDelegateHandle DelegateHandle_OnInviteAborted;
    FDelegateHandle DelegateHandle_OnFriendRemoved;
    TArray<FDelegateHandle> DelegateHandle_OnRejectInviteComplete;
    TArray<FDelegateHandle> DelegateHandle_OnDeleteFriendComplete;
    TArray<FDelegateHandle> DelegateHandle_OnBlockedPlayerComplete;
    TArray<FDelegateHandle> DelegateHandle_OnUnblockedPlayerComplete;
    TArray<FDelegateHandle> DelegateHandle_OnBlockListChange;
    FDelegateHandle DelegateHandle_OnQueryRecentPlayersComplete;
    FDelegateHandle DelegateHandle_OnQueryBlockedPlayersComplete;
    FDelegateHandle DelegateHandle_OnRecentPlayersAdded;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineFriendsSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Friends")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_FriendsChange_BP OnFriendsChange;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_OutgoingInviteSent_BP OnOutgoingInviteSent;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_InviteReceived_BP OnInviteReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_InviteAccepted_BP OnInviteAccepted;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_InviteRejected_BP OnInviteRejected;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_InviteAborted_BP OnInviteAborted;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_FriendRemoved_BP OnFriendRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_RejectInviteComplete_BP OnRejectInviteComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_DeleteFriendComplete_BP OnDeleteFriendComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_BlockedPlayerComplete_BP OnBlockedPlayerComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_UnblockedPlayerComplete_BP OnUnblockedPlayerComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_BlockListChange_BP OnBlockListChange;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_QueryRecentPlayersComplete_BP OnQueryRecentPlayersComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_QueryBlockedPlayersComplete_BP OnQueryBlockedPlayersComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Friends")
    FFriends_RecentPlayersAdded_BP OnRecentPlayersAdded;

    friend class UOnlineFriendsSubsystemReadFriendsList;
    
    friend class UOnlineFriendsSubsystemDeleteFriendsList;
    
    friend class UOnlineFriendsSubsystemSendInvite;
    
    friend class UOnlineFriendsSubsystemAcceptInvite;
    
    friend class UOnlineFriendsSubsystemRejectInvite;
    
    friend class UOnlineFriendsSubsystemSetFriendAlias;
    
    friend class UOnlineFriendsSubsystemDeleteFriendAlias;
    
    friend class UOnlineFriendsSubsystemDeleteFriend;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    bool GetFriendsList(int32 LocalUserNum, FString ListName, TArray<UOnlineFriendRef*>& OutFriends);

    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    UOnlineFriendRef* GetFriend(int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName);

    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    bool IsFriend(int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName);

    friend class UOnlineFriendsSubsystemAddRecentPlayers;
    
    friend class UOnlineFriendsSubsystemQueryRecentPlayers;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    bool GetRecentPlayers(FUniqueNetIdRepl UserId, FString Namespace, TArray<UOnlineRecentPlayerRef*>& OutRecentPlayers);

    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    void DumpRecentPlayers();

    friend class UOnlineFriendsSubsystemBlockPlayer;
    
    friend class UOnlineFriendsSubsystemUnblockPlayer;
    
    friend class UOnlineFriendsSubsystemQueryBlockedPlayers;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    bool GetBlockedPlayers(FUniqueNetIdRepl UserId, TArray<UOnlineUserRef*>& OutBlockedPlayers);

    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    void DumpBlockedPlayers();

    friend class UOnlineFriendsSubsystemQueryFriendSettings;
    
    friend class UOnlineFriendsSubsystemUpdateFriendSettings;
    
    // @generator-excluded-with-pick
    // bool QueryFriendSettings(const FUniqueNetId& UserId, const FString& Source);

    UFUNCTION(BlueprintCallable, Category = "Online|Friends")
    bool GetFriendSettings(FUniqueNetIdRepl UserId, TMap<FString, FOnlineFriendSettingsSourceDataConfig>& OutSettings);

    friend class UOnlineFriendsSubsystemSetFriendSettings;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemReadFriendsListCallbackPin, int32, LocalUserNum, bool, bWasSuccessful, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemReadFriendsList : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemReadFriendsListCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemReadFriendsListCallbackPin OnReadFriendsListComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemReadFriendsList* ReadFriendsList(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemDeleteFriendsListCallbackPin, int32, LocalUserNum, bool, bWasSuccessful, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemDeleteFriendsList : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemDeleteFriendsListCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemDeleteFriendsListCallbackPin OnDeleteFriendsListComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemDeleteFriendsList* DeleteFriendsList(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnDeleteFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineFriendsSubsystemSendInviteCallbackPin, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, FriendId, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemSendInvite : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemSendInviteCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemSendInviteCallbackPin OnSendInviteComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemSendInvite* SendInvite(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSendInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineFriendsSubsystemAcceptInviteCallbackPin, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, FriendId, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemAcceptInvite : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemAcceptInviteCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemAcceptInviteCallbackPin OnAcceptInviteComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemAcceptInvite* AcceptInvite(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnAcceptInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemRejectInviteCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, FriendId, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemRejectInvite : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemRejectInviteCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemRejectInviteCallbackPin OnRejectInviteComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemRejectInvite* RejectInvite(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnRejectInviteComplete;
    void HandleCallback_OnRejectInviteComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemSetFriendAliasCallbackPin, int32, LocalUserNum, FUniqueNetIdRepl, FriendId, FString, ListName, FOnlineErrorInfo, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemSetFriendAlias : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemSetFriendAliasCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemSetFriendAliasCallbackPin OnSetFriendAliasComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemSetFriendAlias* SetFriendAlias(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName, FString Alias
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSetFriendAliasComplete(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnlineError& Error);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__ListName;

UPROPERTY()
    FString __Store__Alias;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemDeleteFriendAliasCallbackPin, int32, LocalUserNum, FUniqueNetIdRepl, FriendId, FString, ListName, FOnlineErrorInfo, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemDeleteFriendAlias : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemDeleteFriendAliasCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemDeleteFriendAliasCallbackPin OnDeleteFriendAliasComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemDeleteFriendAlias* DeleteFriendAlias(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnDeleteFriendAliasComplete(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnlineError& Error);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemDeleteFriendCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, FriendId, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemDeleteFriend : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemDeleteFriendCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemDeleteFriendCallbackPin OnDeleteFriendComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemDeleteFriend* DeleteFriend(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnDeleteFriendComplete;
    void HandleCallback_OnDeleteFriendComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__FriendId;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineFriendsSubsystemAddRecentPlayersCallbackPin, FUniqueNetIdRepl, UserId, FOnlineErrorInfo, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemAddRecentPlayers : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemAddRecentPlayersCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemAddRecentPlayersCallbackPin OnAddRecentPlayersComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemAddRecentPlayers* AddRecentPlayers(
        UOnlineFriendsSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, TArray<FReportPlayedWithUserInfo> InRecentPlayers, FString ListName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnAddRecentPlayersComplete(const FUniqueNetId& UserId, const FOnlineError& Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    TArray<FReportPlayedWithUserInfo> __Store__InRecentPlayers;

UPROPERTY()
    FString __Store__ListName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemQueryRecentPlayersCallbackPin, FUniqueNetIdRepl, UserId, FString, Namespace, bool, bWasSuccessful, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemQueryRecentPlayers : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemQueryRecentPlayersCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemQueryRecentPlayersCallbackPin OnQueryRecentPlayersComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemQueryRecentPlayers* QueryRecentPlayers(
        UOnlineFriendsSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString Namespace
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnQueryRecentPlayersComplete;
    void HandleCallback_OnQueryRecentPlayersComplete(const FUniqueNetId& UserId, const FString& Namespace, bool bWasSuccessful, const FString& Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__Namespace;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemBlockPlayerCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UniqueId, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemBlockPlayer : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemBlockPlayerCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemBlockPlayerCallbackPin OnBlockedPlayerComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemBlockPlayer* BlockPlayer(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl PlayerId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnBlockedPlayerComplete;
    void HandleCallback_OnBlockedPlayerComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__PlayerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineFriendsSubsystemUnblockPlayerCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UniqueId, FString, ListName, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemUnblockPlayer : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemUnblockPlayerCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemUnblockPlayerCallbackPin OnUnblockedPlayerComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemUnblockPlayer* UnblockPlayer(
        UOnlineFriendsSubsystem* Subsystem
        ,int32 LocalUserNum, FUniqueNetIdRepl PlayerId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnUnblockedPlayerComplete;
    void HandleCallback_OnUnblockedPlayerComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& ListName, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FUniqueNetIdRepl __Store__PlayerId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineFriendsSubsystemQueryBlockedPlayersCallbackPin, FUniqueNetIdRepl, UserId, bool, bWasSuccessful, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemQueryBlockedPlayers : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemQueryBlockedPlayersCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemQueryBlockedPlayersCallbackPin OnQueryBlockedPlayersComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemQueryBlockedPlayers* QueryBlockedPlayers(
        UOnlineFriendsSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnQueryBlockedPlayersComplete;
    void HandleCallback_OnQueryBlockedPlayersComplete(const FUniqueNetId& UserId, bool bWasSuccessful, const FString& Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineFriendsSubsystemQueryFriendSettingsCallbackPin, FUniqueNetIdRepl, UserId, bool, bWasSuccessful, bool, bWasUpdate, FFriendSettingsData, Settings, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemQueryFriendSettings : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemQueryFriendSettingsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemQueryFriendSettingsCallbackPin OnSettingsOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemQueryFriendSettings* QueryFriendSettings(
        UOnlineFriendsSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSettingsOperationComplete(const FUniqueNetId& UserId, bool bWasSuccessful, bool bWasUpdate, const FFriendSettings& Settings, const FString& ErrorStr);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineFriendsSubsystemUpdateFriendSettingsCallbackPin, FUniqueNetIdRepl, UserId, bool, bWasSuccessful, bool, bWasUpdate, FFriendSettingsData, Settings, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemUpdateFriendSettings : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemUpdateFriendSettingsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemUpdateFriendSettingsCallbackPin OnSettingsOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemUpdateFriendSettings* UpdateFriendSettings(
        UOnlineFriendsSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FFriendSettingsData NewSettings
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSettingsOperationComplete(const FUniqueNetId& UserId, bool bWasSuccessful, bool bWasUpdate, const FFriendSettings& Settings, const FString& ErrorStr);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FFriendSettingsData __Store__NewSettings;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineFriendsSubsystemSetFriendSettingsCallbackPin, FUniqueNetIdRepl, Param1, bool, Param2, FString, Param3);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendsSubsystemSetFriendSettings : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemSetFriendSettingsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineFriendsSubsystemSetFriendSettingsCallbackPin OnSetFriendSettingsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Friends", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineFriendsSubsystemSetFriendSettings* SetFriendSettings(
        UOnlineFriendsSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString Source, bool bNeverShowAgain
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineFriendsSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSetFriendSettingsComplete(const FUniqueNetId& Param1, bool Param2, const FString& Param3);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__Source;

UPROPERTY()
    bool __Store__bNeverShowAgain;
};
