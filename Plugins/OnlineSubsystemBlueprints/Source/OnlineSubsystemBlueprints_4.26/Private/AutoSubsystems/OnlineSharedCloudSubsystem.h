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
#include "Interfaces/OnlineSharedCloudInterface.h"

#include "OnlineSharedCloudSubsystem.generated.h"

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSharedCloud_ReadSharedFileComplete_BP, bool, Param1, const FSharedContentHandle&, Param2);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSharedCloud_WriteSharedFileComplete_BP, bool, Param1, const FUniqueNetId&, Param2, const FString&, Param3, const TSharedRef<FSharedContentHandle>&, Param4);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineSharedCloudSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineSharedCloud, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineSharedCloud, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineSharedCloud, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineSharedCloudSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|SharedCloud")
    bool IsSubsystemAvailable();

    // @generator-incompatible
    // bool GetSharedFileContents(const FSharedContentHandle& SharedHandle, TArray<uint8>& FileContents);

    UFUNCTION(BlueprintCallable, Category = "Online|SharedCloud")
    bool ClearSharedFiles();

    // @generator-incompatible
    // bool ClearSharedFile(const FSharedContentHandle& SharedHandle);

    // @generator-incompatible
    // bool ReadSharedFile(const FSharedContentHandle& SharedHandle);

    UFUNCTION(BlueprintCallable, Category = "Online|SharedCloud")
    bool WriteSharedFile(FUniqueNetIdRepl UserId, FString Filename, UFileData*& Contents);

    // @generator-incompatible
    // void GetDummySharedHandlesForTest(TArray<TSharedRef<FSharedContentHandle>>& OutHandles);
};
