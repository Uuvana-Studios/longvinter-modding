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
#include "Interfaces/OnlineGroupsInterface.h"

#include "OnlineGroupsSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineGroupsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineGroups, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineGroups, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineGroups, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineGroupsSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Groups")
    bool IsSubsystemAvailable();

    // @generator-incompatible
    // void CreateGroup(const FUniqueNetId& ContextUserId, const FGroupDisplayInfo& GroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

    // @generator-incompatible
    // void FindGroups(const FUniqueNetId& ContextUserId, const FGroupSearchOptions& SearchOptions, const FOnFindGroupsCompleted& OnCompleted);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryGroupInfo(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryGroupNameExist(FUniqueNetIdRepl ContextUserId, FString GroupName);

    // @generator-incompatible
    // TSharedPtr<const IGroupInfo> GetCachedGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void JoinGroup(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void LeaveGroup(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void CancelRequest(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void AcceptInvite(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void DeclineInvite(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryGroupRoster(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @generator-incompatible
    // TSharedPtr<const IGroupRoster> GetCachedGroupRoster(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryUserMembership(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl UserId);

    // @generator-incompatible
    // TSharedPtr<const IUserMembership> GetCachedUserMembership(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryOutgoingApplications(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl UserId);

    // @generator-incompatible
    // TSharedPtr<const IApplications> GetCachedApplications(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryOutgoingInvitations(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryIncomingInvitations(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl UserId);

    // @generator-incompatible
    // TSharedPtr<const IInvitations> GetCachedInvitations(const FUniqueNetId& ContextUserId, const FUniqueNetId& UserId);

    // @generator-incompatible
    // void UpdateGroupInfo(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FGroupDisplayInfo& GroupInfo, const FOnGroupsRequestCompleted& OnCompleted);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void AcceptUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void DeclineUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void InviteUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId, bool bAllowBlocked);

    // @generator-duplicate-needs-picking
    // void InviteUser(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId, const FUniqueNetId& UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void CancelInvite(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void RemoveUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void PromoteUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void DemoteUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void BlockUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void UnblockUser(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryGroupInvites(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @generator-incompatible
    // TSharedPtr<const IGroupInvites> GetCachedGroupInvites(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryGroupRequests(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @generator-incompatible
    // TSharedPtr<const IGroupRequests> GetCachedGroupRequests(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryGroupBlacklist(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    // @generator-incompatible
    // TSharedPtr<const IGroupBlacklist> GetCachedGroupBlacklist(const FUniqueNetId& ContextUserId, const FUniqueNetId& GroupId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryIncomingApplications(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl UserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryConfigHeadcount(FUniqueNetIdRepl ContextUserId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void QueryConfigMembership(FUniqueNetIdRepl ContextUserId);

    // @generator-incompatible
    // TSharedPtr<const FGroupConfigEntryInt> GetCachedConfigInt(const FString& Key);

    // @generator-incompatible
    // TSharedPtr<const FGroupConfigEntryBool> GetCachedConfigBool(const FString& Key);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void TransferGroup(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId, FUniqueNetIdRepl NewOwnerId);

    // @result-delegates-not-compatible: FOnGroupsRequestCompleted(FGroupsResult Param1)
    // void DeleteGroup(FUniqueNetIdRepl ContextUserId, FUniqueNetIdRepl GroupId);

    UFUNCTION(BlueprintCallable, Category = "Online|Groups")
    void SetNamespace(FString Ns);

    UFUNCTION(BlueprintPure, Category = "Online|Groups")
    FString GetNamespace();
};
