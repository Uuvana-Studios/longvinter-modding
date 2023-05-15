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
#include "Interfaces/OnlineUserCloudInterface.h"

#include "OnlineUserCloudSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUserCloud_EnumerateUserFilesComplete_BP, bool, bWasSuccessful, FUniqueNetIdRepl, UserId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUserCloud_ReadUserFileComplete_BP, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUserCloud_WriteUserFileProgress_BP, int32, BytesWritten, FUniqueNetIdRepl, UserId, FString, FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUserCloud_WriteUserFileComplete_BP, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUserCloud_WriteUserFileCanceled_BP, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUserCloud_DeleteUserFileComplete_BP, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FUserCloud_RequestUsageInfoComplete_BP, bool, bWasSuccessful, const FUniqueNetId&, UserId, int64, BytesUsed, const TOptional<int64>&, TotalQuota);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnEnumerateUserFilesComplete;
    FDelegateHandle DelegateHandle_OnReadUserFileComplete;
    FDelegateHandle DelegateHandle_OnWriteUserFileProgress;
    FDelegateHandle DelegateHandle_OnWriteUserFileComplete;
    FDelegateHandle DelegateHandle_OnWriteUserFileCanceled;
    FDelegateHandle DelegateHandle_OnDeleteUserFileComplete;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineUserCloudSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|UserCloud")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_EnumerateUserFilesComplete_BP OnEnumerateUserFilesComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_ReadUserFileComplete_BP OnReadUserFileComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_WriteUserFileProgress_BP OnWriteUserFileProgress;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_WriteUserFileComplete_BP OnWriteUserFileComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_WriteUserFileCanceled_BP OnWriteUserFileCanceled;

    UPROPERTY(BlueprintAssignable, Category = "Online|UserCloud")
    FUserCloud_DeleteUserFileComplete_BP OnDeleteUserFileComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool GetFileContents(FUniqueNetIdRepl UserId, FString FileName, UFileData*& FileContents);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool ClearFiles(FUniqueNetIdRepl UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool ClearFile(FUniqueNetIdRepl UserId, FString FileName);

    friend class UOnlineUserCloudSubsystemEnumerateUserFiles;
    
    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void GetUserFileList(FUniqueNetIdRepl UserId, TArray<FCloudFileHeaderBP>& UserFiles);

    friend class UOnlineUserCloudSubsystemReadUserFile;
    
    friend class UOnlineUserCloudSubsystemWriteUserFile;
    
    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void CancelWriteUserFile(FUniqueNetIdRepl UserId, FString FileName);

    friend class UOnlineUserCloudSubsystemDeleteUserFile;
    
    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    bool RequestUsageInfo(FUniqueNetIdRepl UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void DumpCloudState(FUniqueNetIdRepl UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud")
    void DumpCloudFileState(FUniqueNetIdRepl UserId, FString FileName);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineUserCloudSubsystemEnumerateUserFilesCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemEnumerateUserFiles : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemEnumerateUserFilesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemEnumerateUserFilesCallbackPin OnEnumerateUserFilesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemEnumerateUserFiles* EnumerateUserFiles(
        UOnlineUserCloudSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserCloudSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnEnumerateUserFilesComplete;
    void HandleCallback_OnEnumerateUserFilesComplete(bool bWasSuccessful, const FUniqueNetId& UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineUserCloudSubsystemReadUserFileCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemReadUserFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemReadUserFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemReadUserFileCallbackPin OnReadUserFileComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemReadUserFile* ReadUserFile(
        UOnlineUserCloudSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString FileName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserCloudSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnReadUserFileComplete;
    void HandleCallback_OnReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__FileName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineUserCloudSubsystemWriteUserFileCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName, int32, BytesWritten);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemWriteUserFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnWriteUserFileComplete;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnWriteUserFileProgress;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemWriteUserFileCallbackPin OnWriteUserFileCanceled;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemWriteUserFile* WriteUserFile(
        UOnlineUserCloudSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString FileName, UFileData* FileContents, bool bCompressBeforeUpload
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserCloudSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnWriteUserFileComplete;
    FDelegateHandle ProgressDelegateHandle_OnWriteUserFileComplete;
    FDelegateHandle CancelDelegateHandle_OnWriteUserFileComplete;
    void HandleCallback_OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);
    void HandleCallbackProgress_OnWriteUserFileComplete(int32 BytesWritten, const FUniqueNetId& UserId, const FString& FileName);
    void HandleCallbackCancel_OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__FileName;

UPROPERTY()
    UFileData* __Store__FileContents;

UPROPERTY()
    bool __Store__bCompressBeforeUpload;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineUserCloudSubsystemDeleteUserFileCallbackPin, bool, bWasSuccessful, FUniqueNetIdRepl, UserId, FString, FileName);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserCloudSubsystemDeleteUserFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemDeleteUserFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineUserCloudSubsystemDeleteUserFileCallbackPin OnDeleteUserFileComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|UserCloud", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineUserCloudSubsystemDeleteUserFile* DeleteUserFile(
        UOnlineUserCloudSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineUserCloudSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnDeleteUserFileComplete;
    void HandleCallback_OnDeleteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__FileName;

UPROPERTY()
    bool __Store__bShouldCloudDelete;

UPROPERTY()
    bool __Store__bShouldLocallyDelete;
};
