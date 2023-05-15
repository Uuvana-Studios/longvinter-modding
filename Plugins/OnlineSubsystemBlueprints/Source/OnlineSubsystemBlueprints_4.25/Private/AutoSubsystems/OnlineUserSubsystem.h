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
#include "Interfaces/OnlineUserInterface.h"

#include "OnlineUserSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FUser_QueryUserInfoComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, const TArray<FUniqueNetIdRepl>&, UserIds, FString, ErrorStr);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineUser, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe>& InHandle);
    TArray<FDelegateHandle> DelegateHandle_OnQueryUserInfoComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineUserSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|User")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|User")
    FUser_QueryUserInfoComplete_BP OnQueryUserInfoComplete;

    friend class UOnlineUserSubsystemQueryUserInfo;
    
    UFUNCTION(BlueprintCallable, Category = "Online|User")
    bool GetAllUserInfo(int32 LocalUserNum, TArray<UOnlineUserRef*>& OutUsers);

    UFUNCTION(BlueprintCallable, Category = "Online|User")
    UOnlineUserRef* GetUserInfo(int32 LocalUserNum, FUniqueNetIdRepl UserId);

    friend class UOnlineUserSubsystemQueryUserIdMapping;
    
    friend class UOnlineUserSubsystemQueryExternalIdMappings;
    
    UFUNCTION(BlueprintCallable, Category = "Online|User")
    void GetExternalIdMappings(FExternalIdQueryOptionsBP QueryOptions, TArray<FString> ExternalIds, TArray<FUniqueNetIdRepl>& OutIds);

    UFUNCTION(BlueprintCallable, Category = "Online|User")
    FUniqueNetIdRepl GetExternalIdMapping(FExternalIdQueryOptionsBP QueryOptions, FString ExternalId);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineUserSubsystemQueryUserInfoCallbackPin, bool, bWasSuccessful, const TArray<FUniqueNetIdRepl>&, UserIds, FString, ErrorStr);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystemQueryUserInfo : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserInfoCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserInfoCallbackPin OnQueryUserInfoComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|User", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserSubsystemQueryUserInfo* QueryUserInfo(
        UOnlineUserSubsystem* Subsystem
        ,int32 LocalUserNum, TArray<FUniqueNetIdRepl> UserIds
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnQueryUserInfoComplete;
    void HandleCallback_OnQueryUserInfoComplete(int32 LocalUserNumCb, bool bWasSuccessful, const TArray<TSharedRef<const FUniqueNetId>>& UserIds, const FString& ErrorStr);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__UserIds;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineUserSubsystemQueryUserIdMappingCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, DisplayNameOrEmail, FUniqueNetIdRepl, FoundUserId, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystemQueryUserIdMapping : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserIdMappingCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryUserIdMappingCallbackPin OnQueryUserMappingComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|User", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserSubsystemQueryUserIdMapping* QueryUserIdMapping(
        UOnlineUserSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString DisplayNameOrEmail
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryUserMappingComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FUniqueNetId& FoundUserId, const FString& Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__DisplayNameOrEmail;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnlineUserSubsystemQueryExternalIdMappingsCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FExternalIdQueryOptionsBP, QueryOptions, const TArray<FString>&, ExternalIds, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserSubsystemQueryExternalIdMappings : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryExternalIdMappingsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserSubsystemQueryExternalIdMappingsCallbackPin OnQueryExternalIdMappingsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|User", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserSubsystemQueryExternalIdMappings* QueryExternalIdMappings(
        UOnlineUserSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FExternalIdQueryOptionsBP QueryOptions, TArray<FString> ExternalIds
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryExternalIdMappingsComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FString& Error);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FExternalIdQueryOptionsBP __Store__QueryOptions;

UPROPERTY()
    TArray<FString> __Store__ExternalIds;
};
