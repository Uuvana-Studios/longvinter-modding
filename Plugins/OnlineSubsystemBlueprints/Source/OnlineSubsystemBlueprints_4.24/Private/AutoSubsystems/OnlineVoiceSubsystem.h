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
#include "Interfaces/VoiceInterface.h"

#include "OnlineVoiceSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoice_PlayerTalkingStateChanged_BP, FUniqueNetIdRepl, Param1, bool, Param2);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineVoice, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnPlayerTalkingStateChanged;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineVoiceSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Voice")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Voice")
    FVoice_PlayerTalkingStateChanged_BP OnPlayerTalkingStateChanged;

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void ProcessMuteChangeNotification();

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void StartNetworkedVoice(int32 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void StopNetworkedVoice(int32 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool RegisterLocalTalker(int64 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void RegisterLocalTalkers();

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool UnregisterLocalTalker(int64 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void UnregisterLocalTalkers();

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool RegisterRemoteTalker(FUniqueNetIdRepl UniqueId);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool UnregisterRemoteTalker(FUniqueNetIdRepl UniqueId);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void RemoveAllRemoteTalkers();

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool IsHeadsetPresent(int64 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool IsLocalPlayerTalking(int64 LocalUserNum);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool IsRemotePlayerTalking(FUniqueNetIdRepl UniqueId);

    UFUNCTION(BlueprintPure, Category = "Online|Voice")
    bool IsMuted(int64 LocalUserNum, FUniqueNetIdRepl UniqueId);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool MuteRemoteTalker(int32 LocalUserNum, FUniqueNetIdRepl PlayerId, bool bIsSystemWide);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool UnmuteRemoteTalker(int32 LocalUserNum, FUniqueNetIdRepl PlayerId, bool bIsSystemWide);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    int32 GetNumLocalTalkers();

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void ClearVoicePackets();

    UFUNCTION(BlueprintPure, Category = "Online|Voice")
    FString GetVoiceDebugState();

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool PatchRemoteTalkerOutputToEndpoint(FString InDeviceName, bool bMuteInGameOutput);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    bool PatchLocalTalkerOutputToEndpoint(FString InDeviceName);

    UFUNCTION(BlueprintCallable, Category = "Online|Voice")
    void DisconnectAllEndpoints();
};
