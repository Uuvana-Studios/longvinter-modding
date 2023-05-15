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
#include "Interfaces/OnlineSharingInterface.h"

#include "OnlineSharingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSharing_RequestNewReadPermissionsComplete_BP, int32, LocalUserNum, bool, Param1);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSharing_RequestNewPublishPermissionsComplete_BP, int32, LocalUserNum, bool, Param1);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSharing_ReadNewsFeedComplete_BP, int32, LocalUserNum, bool, Param1);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSharing_SharePostComplete_BP, int32, LocalUserNum, bool, Param1);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSharingSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineSharing, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineSharing, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineSharing, ESPMode::ThreadSafe>& InHandle);
    TArray<FDelegateHandle> DelegateHandle_OnRequestNewReadPermissionsComplete;
    TArray<FDelegateHandle> DelegateHandle_OnRequestNewPublishPermissionsComplete;
    TArray<FDelegateHandle> DelegateHandle_OnReadNewsFeedComplete;
    TArray<FDelegateHandle> DelegateHandle_OnSharePostComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineSharingSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Sharing")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Sharing")
    FSharing_RequestNewReadPermissionsComplete_BP OnRequestNewReadPermissionsComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Sharing")
    FSharing_RequestNewPublishPermissionsComplete_BP OnRequestNewPublishPermissionsComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Sharing")
    FSharing_ReadNewsFeedComplete_BP OnReadNewsFeedComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Sharing")
    FSharing_SharePostComplete_BP OnSharePostComplete;

    // @result-delegates-not-compatible: FOnRequestCurrentPermissionsComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FSharingPermission>& Permissions)
    // void RequestCurrentPermissions(int32 LocalUserNum);

    // @generator-incompatible
    // void GetCurrentPermissions(int32 LocalUserNum, TArray<FSharingPermission>& OutPermissions);

    // @generator-incompatible
    // bool RequestNewReadPermissions(int32 LocalUserNum, EOnlineSharingCategory NewPermissions);

    // @generator-incompatible
    // bool RequestNewPublishPermissions(int32 LocalUserNum, EOnlineSharingCategory NewPermissions, EOnlineStatusUpdatePrivacy Privacy);

    UFUNCTION(BlueprintCallable, Category = "Online|Sharing")
    bool ReadNewsFeed(int32 LocalUserNum, int32 NumPostsToRead);

    // @generator-incompatible
    // EOnlineCachedResult::Type GetCachedNewsFeed(int32 LocalUserNum, int32 NewsFeedIdx, FOnlineStatusUpdate& OutNewsFeed);

    // @generator-incompatible
    // EOnlineCachedResult::Type GetCachedNewsFeeds(int32 LocalUserNum, TArray<FOnlineStatusUpdate>& OutNewsFeeds);

    // @generator-incompatible
    // bool ShareStatusUpdate(int32 LocalUserNum, const FOnlineStatusUpdate& StatusUpdate);
};
