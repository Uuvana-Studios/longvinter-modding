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
#include "Interfaces/OnlineAvatarInterface.h"

#include "OnlineAvatarSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAvatarSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineAvatar, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineAvatar, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineAvatar, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineAvatarSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Avatar")
    bool IsSubsystemAvailable();

    friend class UOnlineAvatarSubsystemGetAvatar;
    
    friend class UOnlineAvatarSubsystemGetAvatarUrl;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineAvatarSubsystemGetAvatarCallbackPin, bool, bWasSuccessful, UTexture*, ResultTexture);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAvatarSubsystemGetAvatar : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarCallbackPin OnGetAvatarComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Avatar", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineAvatarSubsystemGetAvatar* GetAvatar(
        UOnlineAvatarSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FUniqueNetIdRepl TargetUserId, UTexture* DefaultTexture
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineAvatarSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnGetAvatarComplete(bool bWasSuccessful, TSoftObjectPtr<UTexture> ResultTexture);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;

UPROPERTY()
    UTexture* __Store__DefaultTexture;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineAvatarSubsystemGetAvatarUrlCallbackPin, bool, bWasSuccessful, FString, ResultAvatarUrl);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineAvatarSubsystemGetAvatarUrl : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarUrlCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineAvatarSubsystemGetAvatarUrlCallbackPin OnGetAvatarUrlComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Avatar", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineAvatarSubsystemGetAvatarUrl* GetAvatarUrl(
        UOnlineAvatarSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FUniqueNetIdRepl TargetUserId, FString DefaultAvatarUrl
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineAvatarSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnGetAvatarUrlComplete(bool bWasSuccessful, FString ResultAvatarUrl);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;

UPROPERTY()
    FString __Store__DefaultAvatarUrl;
};
