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
#include "Interfaces/OnlinePartyInterface.h"

#include "OnlinePartySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FParty_PartyJoined_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FParty_PartyExited_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyStateChanged_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, EPartyState_, State, EPartyState_, PreviousState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyJIP_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, bool, Success);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyPromotionLockoutChanged_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, bool, bLockoutState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyConfigChanged_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FOnlinePartyConfiguration, PartyConfig);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyDataReceived_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FName, Namespace, UReadablePartyData*, PartyData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyMemberPromoted_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, NewLeaderId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyMemberExited_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, MemberId, EMemberExitedReason_, Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyMemberJoined_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, MemberId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FParty_PartyMemberDataReceived_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, MemberId, FName, Namespace, UReadablePartyData*, PartyData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FParty_PartyInvitesChanged_BP, FUniqueNetIdRepl, LocalUserId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyInviteRequestReceived_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, SenderId, FUniqueNetIdRepl, RequestForId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyInviteReceived_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, SenderId);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyInviteRemoved_BP, const FUniqueNetId&, LocalUserId, const FOnlinePartyId&, PartyId, const FUniqueNetId&, SenderId, EPartyInvitationRemovedReason, Reason);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FParty_PartyInviteResponseReceived_BP, const FUniqueNetId&, LocalUserId, const FOnlinePartyId&, PartyId, const FUniqueNetId&, SenderId, const EInvitationResponse, Response);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyJoinRequestReceived_BP, const FUniqueNetId&, LocalUserId, const FOnlinePartyId&, PartyId, const IOnlinePartyPendingJoinRequestInfo&, JoinRequestInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyJIPRequestReceived_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, SenderId);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_QueryPartyJoinabilityReceived_BP, const FUniqueNetId&, LocalUserId, const FOnlinePartyId&, PartyId, const IOnlinePartyPendingJoinRequestInfo&, JoinRequestInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_FillPartyJoinRequestData_BP, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, UMutablePartyData*, PartyData);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FParty_PartyAnalyticsEvent_BP, const FUniqueNetId&, LocalUserId, const FString&, EventName, const TArray<FAnalyticsEventAttribute>&, Attributes);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FParty_PartySystemStateChange_BP, EPartySystemState, NewState);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnPartyJoined;
    FDelegateHandle DelegateHandle_OnPartyExited;
    FDelegateHandle DelegateHandle_OnPartyStateChanged;
    FDelegateHandle DelegateHandle_OnPartyJIP;
    FDelegateHandle DelegateHandle_OnPartyPromotionLockoutChanged;
    FDelegateHandle DelegateHandle_OnPartyConfigChanged;
    FDelegateHandle DelegateHandle_OnPartyDataReceived;
    FDelegateHandle DelegateHandle_OnPartyMemberPromoted;
    FDelegateHandle DelegateHandle_OnPartyMemberExited;
    FDelegateHandle DelegateHandle_OnPartyMemberJoined;
    FDelegateHandle DelegateHandle_OnPartyMemberDataReceived;
    FDelegateHandle DelegateHandle_OnPartyInvitesChanged;
    FDelegateHandle DelegateHandle_OnPartyInviteRequestReceived;
    FDelegateHandle DelegateHandle_OnPartyInviteReceived;
    FDelegateHandle DelegateHandle_OnPartyJIPRequestReceived;
    FDelegateHandle DelegateHandle_OnFillPartyJoinRequestData;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlinePartySubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyJoined_BP OnPartyJoined;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyExited_BP OnPartyExited;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyStateChanged_BP OnPartyStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyJIP_BP OnPartyJIP;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyPromotionLockoutChanged_BP OnPartyPromotionLockoutChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyConfigChanged_BP OnPartyConfigChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyDataReceived_BP OnPartyDataReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberPromoted_BP OnPartyMemberPromoted;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberExited_BP OnPartyMemberExited;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberJoined_BP OnPartyMemberJoined;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyMemberDataReceived_BP OnPartyMemberDataReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyInvitesChanged_BP OnPartyInvitesChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyInviteRequestReceived_BP OnPartyInviteRequestReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyInviteReceived_BP OnPartyInviteReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_PartyJIPRequestReceived_BP OnPartyJIPRequestReceived;

    UPROPERTY(BlueprintAssignable, Category = "Online|Party")
    FParty_FillPartyJoinRequestData_BP OnFillPartyJoinRequestData;

    friend class UOnlinePartySubsystemRestoreParties;
    
    friend class UOnlinePartySubsystemRestoreInvites;
    
    friend class UOnlinePartySubsystemCleanupParties;
    
    friend class UOnlinePartySubsystemCreateParty;
    
    friend class UOnlinePartySubsystemUpdateParty;
    
    friend class UOnlinePartySubsystemJoinParty;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool JIPFromWithinParty(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl PartyLeaderId);

    friend class UOnlinePartySubsystemQueryPartyJoinability;
    
    friend class UOnlinePartySubsystemRejoinParty;
    
    // @generator-excluded-with-pick
    // bool LeaveParty(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId);

    friend class UOnlinePartySubsystemLeaveParty;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool ApproveJoinRequest(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl RecipientId, bool bIsApproved, int32 DeniedResultCode);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool ApproveJIPRequest(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl RecipientId, bool bIsApproved, int32 DeniedResultCode);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    void RespondToQueryJoinability(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl RecipientId, bool bCanJoin, int32 DeniedResultCode);

    friend class UOnlinePartySubsystemSendInvitation;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool RejectInvitation(FUniqueNetIdRepl LocalUserId, FUniqueNetIdRepl SenderId);

    // @generator-incompatible
    // void ClearInvitations(const FUniqueNetId& LocalUserId, const FUniqueNetId& SenderId, const FOnlinePartyId* PartyId);

    friend class UOnlinePartySubsystemKickMember;
    
    friend class UOnlinePartySubsystemPromoteMember;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool UpdatePartyData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FName Namespace, UReadablePartyData* PartyData);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    bool UpdatePartyMemberData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FName Namespace, UReadablePartyData* PartyMemberData);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool IsMemberLeader(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl MemberId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    int64 GetPartyMemberCount(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UParty* GetParty(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId);

    // @generator-excluded-with-pick
    // FOnlinePartyConstPtr GetParty(const FUniqueNetId& LocalUserId, const FOnlinePartyTypeId& PartyTypeId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UBlueprintPartyMember* GetPartyMember(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl MemberId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UReadablePartyData* GetPartyData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FName Namespace);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    UReadablePartyData* GetPartyMemberData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl MemberId, FName Namespace);

    // @generator-incompatible
    // IOnlinePartyJoinInfoConstPtr GetAdvertisedParty(const FUniqueNetId& LocalUserId, const FUniqueNetId& UserId, const FOnlinePartyTypeId PartyTypeId);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetJoinedParties(FUniqueNetIdRepl LocalUserId, TArray<UPartyId*>& OutPartyIdArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPartyMembers(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, TArray<UBlueprintPartyMember*>& OutPartyMembersArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPendingInvites(FUniqueNetIdRepl LocalUserId, TArray<UOnlinePartyJoinInfo*>& OutPendingInvitesArray);

    // @generator-incompatible
    // bool GetPendingJoinRequests(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, TArray<IOnlinePartyPendingJoinRequestInfoConstRef>& OutPendingJoinRequestArray);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    bool GetPendingInvitedUsers(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, TArray<FUniqueNetIdRepl>& OutPendingInvitedUserArray);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    FString MakeJoinInfoJson(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId);

    // @generator-incompatible
    // IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromJson(const FString& JoinInfoJson);

    UFUNCTION(BlueprintPure, Category = "Online|Party")
    FString MakeTokenFromJoinInfo(UOnlinePartyJoinInfo* JoinInfo);

    // @generator-incompatible
    // IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromToken(const FString& Token);

    // @generator-incompatible
    // IOnlinePartyJoinInfoConstPtr ConsumePendingCommandLineInvite();

    // @generator-incompatible
    // FDelegateHandle AddOnPartyDataReceivedDelegate_Handle(const FOnPartyDataReceivedConstDelegate& Delegate);

    // @generator-incompatible
    // FDelegateHandle AddOnPartyMemberDataReceivedDelegate_Handle(const FOnPartyMemberDataReceivedConstDelegate& Delegate);

    UFUNCTION(BlueprintCallable, Category = "Online|Party")
    void DumpPartyState();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlinePartySubsystemRestorePartiesCallbackPin, FUniqueNetIdRepl, LocalUserId, FOnlineErrorInfo, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemRestoreParties : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestorePartiesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestorePartiesCallbackPin OnRestorePartiesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemRestoreParties* RestoreParties(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRestorePartiesComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlinePartySubsystemRestoreInvitesCallbackPin, FUniqueNetIdRepl, LocalUserId, FOnlineErrorInfo, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemRestoreInvites : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestoreInvitesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRestoreInvitesCallbackPin OnRestoreInvitesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemRestoreInvites* RestoreInvites(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRestoreInvitesComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlinePartySubsystemCleanupPartiesCallbackPin, FUniqueNetIdRepl, LocalUserId, FOnlineErrorInfo, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemCleanupParties : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCleanupPartiesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCleanupPartiesCallbackPin OnCleanupPartiesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemCleanupParties* CleanupParties(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnCleanupPartiesComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlinePartySubsystemCreatePartyCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, ECreatePartyCompletionResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemCreateParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCreatePartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemCreatePartyCallbackPin OnCreatePartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemCreateParty* CreateParty(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, int64 PartyTypeId, FOnlinePartyConfiguration PartyConfig
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnCreatePartyComplete(const FUniqueNetId& LocalUserId, const TSharedPtr<const FOnlinePartyId>& PartyId, const ECreatePartyCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    int64 __Store__PartyTypeId;

UPROPERTY()
    FOnlinePartyConfiguration __Store__PartyConfig;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlinePartySubsystemUpdatePartyCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, EUpdateConfigCompletionResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemUpdateParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemUpdatePartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemUpdatePartyCallbackPin OnUpdatePartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemUpdateParty* UpdateParty(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FOnlinePartyConfiguration PartyConfig, bool bShouldRegenerateReservationKey
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnUpdatePartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EUpdateConfigCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UPartyId* __Store__PartyId;

UPROPERTY()
    FOnlinePartyConfiguration __Store__PartyConfig;

UPROPERTY()
    bool __Store__bShouldRegenerateReservationKey;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlinePartySubsystemJoinPartyCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, EJoinPartyCompletionResult_, Result, int32, NotApprovedReason);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemJoinParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemJoinPartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemJoinPartyCallbackPin OnJoinPartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemJoinParty* JoinParty(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UOnlinePartyJoinInfo* OnlinePartyJoinInfo
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnJoinPartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EJoinPartyCompletionResult Result, const int32 NotApprovedReason);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UOnlinePartyJoinInfo* __Store__OnlinePartyJoinInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlinePartySubsystemQueryPartyJoinabilityCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, EJoinPartyCompletionResult_, Result, int32, NotApprovedReason);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemQueryPartyJoinability : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemQueryPartyJoinabilityCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemQueryPartyJoinabilityCallbackPin OnQueryPartyJoinabilityComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemQueryPartyJoinability* QueryPartyJoinability(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UOnlinePartyJoinInfo* OnlinePartyJoinInfo
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryPartyJoinabilityComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EJoinPartyCompletionResult Result, const int32 NotApprovedReason);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UOnlinePartyJoinInfo* __Store__OnlinePartyJoinInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlinePartySubsystemRejoinPartyCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, EJoinPartyCompletionResult_, Result, int32, NotApprovedReason);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemRejoinParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRejoinPartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemRejoinPartyCallbackPin OnJoinPartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemRejoinParty* RejoinParty(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, int64 PartyTypeId, TArray<FUniqueNetIdRepl> FormerMembers
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnJoinPartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EJoinPartyCompletionResult Result, const int32 NotApprovedReason);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UPartyId* __Store__PartyId;

UPROPERTY()
    int64 __Store__PartyTypeId;

UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__FormerMembers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlinePartySubsystemLeavePartyCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, ELeavePartyCompletionResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemLeaveParty : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemLeavePartyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemLeavePartyCallbackPin OnLeavePartyComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemLeaveParty* LeaveParty(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, bool bSynchronizeLeave
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLeavePartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const ELeavePartyCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UPartyId* __Store__PartyId;

UPROPERTY()
    bool __Store__bSynchronizeLeave;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlinePartySubsystemSendInvitationCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, RecipientId, ESendPartyInvitationCompletionResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemSendInvitation : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemSendInvitationCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemSendInvitationCallbackPin OnSendPartyInvitationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemSendInvitation* SendInvitation(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl Recipient
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnSendPartyInvitationComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, const ESendPartyInvitationCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UPartyId* __Store__PartyId;

UPROPERTY()
    FUniqueNetIdRepl __Store__Recipient;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlinePartySubsystemKickMemberCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, MemberId, EKickMemberCompletionResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemKickMember : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemKickMemberCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemKickMemberCallbackPin OnKickPartyMemberComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemKickMember* KickMember(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl TargetMemberId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnKickPartyMemberComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const EKickMemberCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UPartyId* __Store__PartyId;

UPROPERTY()
    FUniqueNetIdRepl __Store__TargetMemberId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlinePartySubsystemPromoteMemberCallbackPin, FUniqueNetIdRepl, LocalUserId, UPartyId*, PartyId, FUniqueNetIdRepl, MemberId, EPromoteMemberCompletionResult_, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartySubsystemPromoteMember : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemPromoteMemberCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePartySubsystemPromoteMemberCallbackPin OnPromotePartyMemberComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Party", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePartySubsystemPromoteMember* PromoteMember(
        UOnlinePartySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl TargetMemberId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePartySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnPromotePartyMemberComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const EPromoteMemberCompletionResult Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    UPartyId* __Store__PartyId;

UPROPERTY()
    FUniqueNetIdRepl __Store__TargetMemberId;
};
