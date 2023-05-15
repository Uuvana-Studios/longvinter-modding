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
#include "Interfaces/OnlineTitleFileInterface.h"

#include "OnlineTitleFileSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTitleFile_EnumerateFilesComplete_BP, bool, bWasSuccessful, FString, Error);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTitleFile_ReadFileComplete_BP, bool, bWasSuccessful, FString, FileName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTitleFile_ReadFileProgress_BP, FString, FileName, int64, NumBytes);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTitleFileSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineTitleFile, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnEnumerateFilesComplete;
    FDelegateHandle DelegateHandle_OnReadFileComplete;
    FDelegateHandle DelegateHandle_OnReadFileProgress;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineTitleFileSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|TitleFile")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|TitleFile")
    FTitleFile_EnumerateFilesComplete_BP OnEnumerateFilesComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|TitleFile")
    FTitleFile_ReadFileComplete_BP OnReadFileComplete;

    UPROPERTY(BlueprintAssignable, Category = "Online|TitleFile")
    FTitleFile_ReadFileProgress_BP OnReadFileProgress;

    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile")
    bool GetFileContents(FString FileName, UFileData*& FileContents);

    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile")
    bool ClearFiles();

    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile")
    bool ClearFile(FString FileName);

    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile")
    void DeleteCachedFiles(bool bSkipEnumerated);

    friend class UOnlineTitleFileSubsystemEnumerateFiles;
    
    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile")
    void GetFileList(TArray<FCloudFileHeaderBP>& Files);

    friend class UOnlineTitleFileSubsystemReadFile;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineTitleFileSubsystemEnumerateFilesCallbackPin, bool, bWasSuccessful, FString, Error);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTitleFileSubsystemEnumerateFiles : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineTitleFileSubsystemEnumerateFilesCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineTitleFileSubsystemEnumerateFilesCallbackPin OnEnumerateFilesComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineTitleFileSubsystemEnumerateFiles* EnumerateFiles(
        UOnlineTitleFileSubsystem* Subsystem
        ,FPagedQueryBP Page
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineTitleFileSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnEnumerateFilesComplete;
    void HandleCallback_OnEnumerateFilesComplete(bool bWasSuccessful, const FString& Error);

    UPROPERTY()
    FPagedQueryBP __Store__Page;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineTitleFileSubsystemReadFileCallbackPin, bool, bWasSuccessful, FString, FileName, int64, NumBytes);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTitleFileSubsystemReadFile : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineTitleFileSubsystemReadFileCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineTitleFileSubsystemReadFileCallbackPin OnReadFileComplete;
    UPROPERTY(BlueprintAssignable)
    FOnlineTitleFileSubsystemReadFileCallbackPin OnReadFileProgress;

    UFUNCTION(BlueprintCallable, Category = "Online|TitleFile", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineTitleFileSubsystemReadFile* ReadFile(
        UOnlineTitleFileSubsystem* Subsystem
        ,FString FileName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineTitleFileSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    FDelegateHandle DelegateHandle_OnReadFileComplete;
    FDelegateHandle ProgressDelegateHandle_OnReadFileComplete;
    void HandleCallback_OnReadFileComplete(bool bWasSuccessful, const FString& FileName);
    void HandleCallbackProgress_OnReadFileComplete(const FString& FileName, uint64 NumBytes);

    UPROPERTY()
    FString __Store__FileName;
};
