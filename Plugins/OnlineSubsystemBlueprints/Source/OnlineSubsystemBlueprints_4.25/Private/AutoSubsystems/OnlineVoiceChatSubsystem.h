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
#include "VoiceChatUser.h"
#include "Types/OSBVoiceChatTypes.h"

#include "OnlineVoiceChatSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceChatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    void RegisterFunctionDelegates();
    void UnregisterFunctionDelegates();

public:
private:
    IVoiceChat* HandlePtr;
    IVoiceChat* GetHandle();
    bool IsHandleValid(IVoiceChat* InHandle);

public:
    UOnlineVoiceChatSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChat")
    bool IsSubsystemAvailable();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat")
    UVoiceChatUser* CreateUser();
    
    /**
     * The primary voice chat user associated with IVoiceUser. Some voice chat systems support 
     * multiple users, in which case you should use CreateUser instead of this property. If the 
     * voice chat system you are using does not support multiple users, then this is the voice 
     * chat user.
     */
    UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Online|VoiceChat")
    UVoiceChatUser* PrimaryVoiceUser;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat")
    bool Initialize_();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat")
    bool Uninitialize_();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChat")
    bool IsInitialized();
};
