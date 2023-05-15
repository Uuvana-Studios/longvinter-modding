// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlinePartyInterface.h"
#include "OSBSessionTypes.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OSBPartyTypes.generated.h"

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UParty : public UObject
{
    GENERATED_BODY()

private:
    TWeakPtr<const FOnlineParty> Party;

public:
    UFUNCTION(BlueprintCallable, Category = "Online")
    UPartyId *GetPartyId();

    UFUNCTION(BlueprintCallable, Category = "Online")
    int64 GetPartyTypeId();

    UFUNCTION(BlueprintCallable, Category = "Online")
    FUniqueNetIdRepl GetLeaderId();

    static UParty *FromNative(FOnlinePartyConstPtr InObj);
    FOnlinePartyConstPtr ToNative() const
    {
        if (this->Party.IsValid())
        {
            return this->Party.Pin();
        }

        return nullptr;
    }
    bool IsValid() const
    {
        return this->Party.IsValid();
    }
};

UENUM(BlueprintType, Meta = (ScriptName = "EMemberConnectionStatus"))
enum class EMemberConnectionStatus_ : uint8
{
    Uninitialized,
    Disconnected,
    Initializing,
    Connected
};

UENUM(BlueprintType, Meta = (ScriptName = "EMemberExitedReason"))
enum class EMemberExitedReason_ : uint8
{
    Unknown,
    Left,
    Removed,
    Kicked
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FPartyMemberAttributeChanged,
    FUniqueNetIdRepl,
    ChangedUserId,
    FString,
    Attribute,
    FString,
    NewValue,
    FString,
    OldValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FPartyMemberConnectionStatusChanged,
    FUniqueNetIdRepl,
    ChangedUserId,
    EMemberConnectionStatus_,
    NewStatus,
    EMemberConnectionStatus_,
    PreviousStatus);

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UBlueprintPartyMember : public UObject
{
    GENERATED_BODY()

private:
    FOnlinePartyMemberConstPtr PartyMember;
    FDelegateHandle AttributeHandle;
    FDelegateHandle ConnectionStatusHandle;

    void OnMemberAttributeChanged(
        const FUniqueNetId &ChangedUserId,
        const FString &Attribute,
        const FString &NewValue,
        const FString &OldValue);
    void OnMemberConnectionStatusChanged(
        const FUniqueNetId &ChangedUserId,
        const EMemberConnectionStatus NewMemberConnectionStatus,
        const EMemberConnectionStatus PreviousMemberConnectionStatus);

public:
    virtual void BeginDestroy() override;

    UPROPERTY(BlueprintAssignable, Category = "Online")
    FPartyMemberAttributeChanged OnAttributeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online")
    FPartyMemberConnectionStatusChanged OnConnectionStatusChanged;

    UFUNCTION(BlueprintPure, Category = "Online")
    FUniqueNetIdRepl GetUserId();

    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetRealName();

    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetDisplayName(const FString &Platform);

    UFUNCTION(BlueprintPure, Category = "Online")
    bool GetUserAttribute(const FString &AttrName, FString &OutAttrValue);

    static UBlueprintPartyMember *FromNative(FOnlinePartyMemberConstPtr InObj);
    FOnlinePartyMemberConstPtr ToNative() const
    {
        return this->PartyMember;
    }
    bool IsValid() const
    {
        return this->PartyMember.IsValid();
    }
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UReadablePartyData : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<const FOnlinePartyData> ConstPartyData;

protected:
    void AssignFromConstPartyData(const FOnlinePartyData &InObj);

public:
    static UReadablePartyData *FromNative(const FOnlinePartyData &InObj);
    static UReadablePartyData *FromNative(TSharedPtr<const FOnlinePartyData> InObj);
    TSharedRef<const FOnlinePartyData> ToNativeConst() const
    {
        return this->ConstPartyData.ToSharedRef();
    }
    bool IsValid() const
    {
        return this->ConstPartyData.IsValid();
    }

    UFUNCTION(BlueprintPure, Category = "Online")
    virtual void GetAttribute(const FString &AttrName, bool &OutFound, FVariantDataBP &OutAttrValue) const;
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UMutablePartyData : public UReadablePartyData
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlinePartyData> MutablePartyData;

public:
    static UMutablePartyData *FromNative(FOnlinePartyData &InObj);
    static UMutablePartyData *FromNative(TSharedRef<FOnlinePartyData> InObj);
    static UMutablePartyData *FromNative(TSharedPtr<FOnlinePartyData> InObj);
    TSharedRef<FOnlinePartyData> ToNativeMutable() const
    {
        return this->MutablePartyData.ToSharedRef();
    }

    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetAttribute(const FString &AttrName, FVariantDataBP AttrValue);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void RemoveAttribute(const FString &AttrName);
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UPartyId : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<const FOnlinePartyId> PartyId;

public:
    static UPartyId *FromNative(const FOnlinePartyId &InObj);
    static UPartyId *FromNative(const TSharedPtr<const FOnlinePartyId> &InObj);
    const FOnlinePartyId &ToNative() const
    {
        return this->PartyId.ToSharedRef().Get();
    }
    bool IsValid() const
    {
        return this->PartyId.IsValid();
    }

    UFUNCTION(BlueprintPure, Category = "Online")
    FString ToDebugString()
    {
        if (!this->IsValid())
        {
            return TEXT("");
        }

        return this->PartyId->ToDebugString();
    }
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePartyJoinInfo : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<const IOnlinePartyJoinInfo> OnlinePartyJoinInfo;

public:
    static UOnlinePartyJoinInfo *FromNative(const IOnlinePartyJoinInfo &InObj);
    const IOnlinePartyJoinInfo &ToNative() const
    {
        return this->OnlinePartyJoinInfo.ToSharedRef().Get();
    }
    bool IsValid() const
    {
        return this->OnlinePartyJoinInfo.IsValid();
    }

    /**
     * Get the ID of the party that this join info is for.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    UPartyId *GetPartyId();

    /**
     * Get the source display name; For invites, this is the display name of the user that sent the invite.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetSourceDisplayName();

    /**
     * Get the source user ID; For invites, this is the ID of the user that sent the invite.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FUniqueNetIdRepl GetSourceUserId();

    UFUNCTION(BlueprintPure, Category = "Online")
    FString ToDebugString();
};

UENUM(BlueprintType, Meta = (ScriptName = "EPartyState"))
enum class EPartyState_ : uint8
{
    None,
    CreatePending,
    JoinPending,
    RejoinPending,
    LeavePending,
    Active,
    Disconnected,
    CleanUp
};

UENUM(BlueprintType, Meta = (ScriptName = "EPartySystemPermissions"))
enum class EPartySystemPermissions : uint8
{
    /** Noone has access to do that action */
    Noone,
    /** Available to the leader only */
    Leader,
    /** Available to the leader and friends of the leader only */
    Friends,
    /** Available to anyone */
    Anyone
};

UENUM(BlueprintType, Meta = (ScriptName = "EJoinRequestAction"))
enum class EJoinRequestAction_ : uint8
{
    Manual,
    AutoApprove,
    AutoReject
};

USTRUCT(BlueprintType)
struct FOnlinePartyConfiguration
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    EJoinRequestAction_ JoinRequestAction;

    /* Who can see this party and join it through presence? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    EPartySystemPermissions PresencePermissions;

    /* Who can send out invites to other users when they're in this party? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    EPartySystemPermissions InvitePermissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool ChatEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool ShouldRemoveOnDisconnection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool IsAcceptingMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    int32 NotAcceptingMembersReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    int32 MaxMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Nickname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Password;

    static FOnlinePartyConfiguration FromNative(const FPartyConfiguration &Config);
    TSharedPtr<FPartyConfiguration> ToNative();
};

UENUM(BlueprintType, Meta = (ScriptName = "ECreatePartyCompletionResult"))
enum class ECreatePartyCompletionResult_ : uint8
{
    UnknownClientFailure = (uint8)-100,
    AlreadyInPartyOfSpecifiedType,
    AlreadyCreatingParty,
    AlreadyInParty,
    FailedToCreateMucRoom,
    NoResponse,
    LoggedOut,
    NotPrimaryUser,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "EJoinPartyCompletionResult"))
enum class EJoinPartyCompletionResult_ : uint8
{
    /** Unspecified error.  No message sent to party leader. */
    UnknownClientFailure = (uint8)-100,
    /** Your build id does not match the build id of the party */
    BadBuild,
    /** Your provided access key does not match the party's access key */
    InvalidAccessKey,
    /** The party leader already has you in the joining players list */
    AlreadyInLeadersJoiningList,
    /** The party leader already has you in the party members list */
    AlreadyInLeadersPartyRoster,
    /** The party leader rejected your join request because the party is full*/
    NoSpace,
    /** The party leader rejected your join request for a game specific reason, indicated by the NotApprovedReason
       parameter */
    NotApproved,
    /** The player you send the join request to is not a member of the specified party */
    RequesteeNotMember,
    /** The player you send the join request to is not the leader of the specified party */
    RequesteeNotLeader,
    /** A response was not received from the party leader in a timely manner, the join attempt is considered failed */
    NoResponse,
    /** You were logged out while attempting to join the party */
    LoggedOut,
    /** You were unable to rejoin the party */
    UnableToRejoin,
    /** Your platform is not compatible with the party */
    IncompatiblePlatform,

    /** We are currently waiting for a response for a previous join request for the specified party.  No message sent to
       party leader. */
    AlreadyJoiningParty,
    /** We are already in the party that you are attempting to join.  No message sent to the party leader. */
    AlreadyInParty,
    /** The party join info is invalid.  No message sent to the party leader. */
    JoinInfoInvalid,
    /** We are already in a party of the specified type.  No message sent to the party leader. */
    AlreadyInPartyOfSpecifiedType,
    /** Failed to send a message to the party leader.  No message sent to the party leader. */
    MessagingFailure,

    /** Game specific reason, indicated by the NotApprovedReason parameter.  Message might or might not have been sent
       to party leader. */
    GameSpecificReason,

    /** DEPRECATED */
    UnknownInternalFailure = 0,

    /** Successully joined the party */
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "ELeavePartyCompletionResult"))
enum class ELeavePartyCompletionResult_ : uint8
{
    /** Unspecified error.  No message sent. */
    UnknownClientFailure = (uint8)-100,
    /** Timed out waiting for a response to the message.  Party has been left. */
    NoResponse,
    /** You were logged out while attempting to leave the party.  Party has been left. */
    LoggedOut,

    /** You are not in the specified party.  No message sent. */
    UnknownParty,
    /** You are already leaving the party.  No message sent. */
    LeavePending,

    /** DEPRECATED! */
    UnknownLocalUser,
    /** DEPRECATED! */
    NotMember,
    /** DEPRECATED! */
    MessagingFailure,
    /** DEPRECATED! */
    UnknownTransportFailure,
    /** DEPRECATED! */
    UnknownInternalFailure = 0,

    /** Successfully left the party */
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "EUpdateConfigCompletionResult"))
enum class EUpdateConfigCompletionResult_ : uint8
{
    UnknownClientFailure = (uint8)-100,
    UnknownParty,
    LocalMemberNotMember,
    LocalMemberNotLeader,
    RemoteMemberNotMember,
    MessagingFailure,
    NoResponse,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "ERequestPartyInvitationCompletionResult"))
enum class ERequestPartyInvitationCompletionResult_ : uint8
{
    NotLoggedIn = (uint8)-100,
    InvitePending,
    AlreadyInParty,
    PartyFull,
    NoPermission,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "ESendPartyInvitationCompletionResult"))
enum class ESendPartyInvitationCompletionResult_ : uint8
{
    NotLoggedIn = (uint8)-100,
    InvitePending,
    AlreadyInParty,
    PartyFull,
    NoPermission,
    RateLimited,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "EAcceptPartyInvitationCompletionResult"))
enum class EAcceptPartyInvitationCompletionResult_ : uint8
{
    NotLoggedIn = (uint8)-100,
    InvitePending,
    AlreadyInParty,
    PartyFull,
    NoPermission,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "ERejectPartyInvitationCompletionResult"))
enum class ERejectPartyInvitationCompletionResult_ : uint8
{
    NotLoggedIn = (uint8)-100,
    InvitePending,
    AlreadyInParty,
    PartyFull,
    NoPermission,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "EKickMemberCompletionResult"))
enum class EKickMemberCompletionResult_ : uint8
{
    UnknownClientFailure = (uint8)-100,
    UnknownParty,
    LocalMemberNotMember,
    LocalMemberNotLeader,
    RemoteMemberNotMember,
    MessagingFailure,
    NoResponse,
    LoggedOut,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "EPromoteMemberCompletionResult"))
enum class EPromoteMemberCompletionResult_ : uint8
{
    UnknownClientFailure = (uint8)-100,
    UnknownServiceFailure,
    UnknownParty,
    LocalMemberNotMember,
    LocalMemberNotLeader,
    PromotionAlreadyPending,
    TargetIsSelf,
    TargetNotMember,
    MessagingFailure,
    NoResponse,
    LoggedOut,
    UnknownInternalFailure = 0,
    Succeeded = 1
};

UENUM(BlueprintType, Meta = (ScriptName = "EInvitationResponse"))
enum class EInvitationResponse_ : uint8
{
    UnknownFailure,
    BadBuild,
    Rejected,
    Accepted
};