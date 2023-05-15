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
#include "Interfaces/OnlineIdentityInterface.h"

#include "OnlineIdentitySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentity_LoginChanged_BP, int32, LocalUserNum);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FIdentity_LoginStatusChanged_BP, int32, LocalUserNum, EOnlineLoginStatus, OldStatus, EOnlineLoginStatus, NewStatus, FUniqueNetIdRepl, NewId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FIdentity_ControllerPairingChanged_BP, int, LocalUserNum, FUniqueNetIdRepl, PreviousUser, FUniqueNetIdRepl, NewUser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FIdentity_LoginComplete_BP, int32, LocalUserNum, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, Error);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentity_LogoutComplete_BP, int32, LocalUserNum, bool, bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentity_LoginFlowLogout_BP, const TArray<FString>&, LoginDomains);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineIdentity, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineIdentity, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineIdentity, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnLoginChanged;
    TArray<FDelegateHandle> DelegateHandle_OnLoginStatusChanged;
    FDelegateHandle DelegateHandle_OnControllerPairingChanged;
    TArray<FDelegateHandle> DelegateHandle_OnLoginComplete;
    TArray<FDelegateHandle> DelegateHandle_OnLogoutComplete;
    FDelegateHandle DelegateHandle_OnLoginFlowLogout;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineIdentitySubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginChanged_BP OnLoginChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginStatusChanged_BP OnLoginStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_ControllerPairingChanged_BP OnControllerPairingChanged;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginComplete_BP OnLoginComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LogoutComplete_BP OnLogoutComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Identity")
    FIdentity_LoginFlowLogout_BP OnLoginFlowLogout;

    friend class UOnlineIdentitySubsystemLogin;
    
    friend class UOnlineIdentitySubsystemLogout;
    
    friend class UOnlineIdentitySubsystemAutoLogin;
    
    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    UUserOnlineAccountRef* GetUserAccount(FUniqueNetIdRepl UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|Identity")
    TArray<UUserOnlineAccountRef*> GetAllUserAccounts();

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FUniqueNetIdRepl GetUniquePlayerId(int32 LocalUserNum);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FUniqueNetIdRepl GetSponsorUniquePlayerId(int32 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Identity")
    FUniqueNetIdRepl CreateUniquePlayerId(FString Str);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    EOnlineLoginStatus GetLoginStatus(int32 LocalUserNum);

    // @generator-excluded-with-pick
    // ELoginStatus::Type GetLoginStatus(const FUniqueNetId& UserId);

    // @generator-excluded-with-pick
    // FString GetPlayerNickname(int32 LocalUserNum);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FString GetPlayerNickname(FUniqueNetIdRepl UserId);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FString GetAuthToken(int32 LocalUserNum);

    friend class UOnlineIdentitySubsystemRevokeAuthToken;
    
    friend class UOnlineIdentitySubsystemGetUserPrivilege;
    
    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    int32 GetPlatformUserIdFromUniqueNetId(FUniqueNetIdRepl UniqueNetId);

    UFUNCTION(BlueprintPure, Category = "Online|Identity")
    FString GetAuthType();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineIdentitySubsystemLoginCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemLogin : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLoginCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLoginCallbackPin OnLoginComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemLogin* Login(
        UOnlineIdentitySubsystem* Subsystem
        ,int32 LocalUserNum, FOnlineAccountCredential AccountCredentials
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineIdentitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLoginComplete;
    void HandleCallback_OnLoginComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

    UPROPERTY()
    int32 __Store__LocalUserNum;

UPROPERTY()
    FOnlineAccountCredential __Store__AccountCredentials;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineIdentitySubsystemLogoutCallbackPin, bool, bWasSuccessful);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemLogout : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLogoutCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemLogoutCallbackPin OnLogoutComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemLogout* Logout(
        UOnlineIdentitySubsystem* Subsystem
        ,int32 LocalUserNum
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineIdentitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLogoutComplete;
    void HandleCallback_OnLogoutComplete(int32 LocalUserNumCb, bool bWasSuccessful);

    UPROPERTY()
    int32 __Store__LocalUserNum;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineIdentitySubsystemAutoLoginCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemAutoLogin : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemAutoLoginCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemAutoLoginCallbackPin OnLoginComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemAutoLogin* AutoLogin(
        UOnlineIdentitySubsystem* Subsystem
        ,int32 LocalUserNum
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineIdentitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnLoginComplete;
    void HandleCallback_OnLoginComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

    UPROPERTY()
    int32 __Store__LocalUserNum;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineIdentitySubsystemRevokeAuthTokenCallbackPin, FUniqueNetIdRepl, UserId, FOnlineErrorInfo, OnlineError);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemRevokeAuthToken : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemRevokeAuthTokenCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemRevokeAuthTokenCallbackPin OnRevokeAuthTokenComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemRevokeAuthToken* RevokeAuthToken(
        UOnlineIdentitySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineIdentitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnRevokeAuthTokenComplete(const FUniqueNetId& UserId, const FOnlineError& OnlineError);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineIdentitySubsystemGetUserPrivilegeCallbackPin, FUniqueNetIdRepl, LocalUserId, EOnlineUserPrivilege, Privilege, int64, PrivilegeResult);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineIdentitySubsystemGetUserPrivilege : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemGetUserPrivilegeCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineIdentitySubsystemGetUserPrivilegeCallbackPin OnGetUserPrivilegeComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Identity", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineIdentitySubsystemGetUserPrivilege* GetUserPrivilege(
        UOnlineIdentitySubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, EOnlineUserPrivilege Privilege
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineIdentitySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnGetUserPrivilegeComplete(const FUniqueNetId& LocalUserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResult);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    EOnlineUserPrivilege __Store__Privilege;
};
