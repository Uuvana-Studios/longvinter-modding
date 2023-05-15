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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceChat_OnVoiceChatReconnectedDelegate_BP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceChat_OnVoiceChatConnectedDelegate_BP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChat_OnVoiceChatDisconnectedDelegate_BP, FVoiceChatResultBP, Reason);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceChatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    FDelegateHandle DelegateHandle_FOnVoiceChatReconnectedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatConnectedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatDisconnectedDelegate;
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

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChat")
    FVoiceChat_OnVoiceChatReconnectedDelegate_BP OnVoiceChatReconnected;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatReconnectedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChat")
    FVoiceChat_OnVoiceChatConnectedDelegate_BP OnVoiceChatConnected;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatConnectedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChat")
    FVoiceChat_OnVoiceChatDisconnectedDelegate_BP OnVoiceChatDisconnected;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatDisconnectedDelegate;
    
    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat")
    bool Initialize_();

    // @generator-duplicate-needs-picking
    // void Initialize();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat")
    bool Uninitialize_();

    // @generator-duplicate-needs-picking
    // void Uninitialize();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChat")
    bool IsInitialized();

    friend class UOnlineVoiceChatSubsystemConnect;
    
    friend class UOnlineVoiceChatSubsystemDisconnect;
    
    UFUNCTION(BlueprintPure, Category = "Online|VoiceChat")
    bool IsConnecting();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChat")
    bool IsConnected();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineVoiceChatSubsystemConnectCallbackPin, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceChatSubsystemConnect : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceChatSubsystemConnectCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceChatSubsystemConnectCallbackPin OnVoiceChatConnectComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineVoiceChatSubsystemConnect* Connect(
        UOnlineVoiceChatSubsystem* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineVoiceChatSubsystem* __Store__Subsystem;
    void HandleCallback_OnVoiceChatConnectComplete(const FVoiceChatResult& Result);

    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlineVoiceChatSubsystemDisconnectCallbackPin, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceChatSubsystemDisconnect : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceChatSubsystemDisconnectCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceChatSubsystemDisconnectCallbackPin OnVoiceChatDisconnectComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChat", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineVoiceChatSubsystemDisconnect* Disconnect(
        UOnlineVoiceChatSubsystem* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineVoiceChatSubsystem* __Store__Subsystem;
    void HandleCallback_OnVoiceChatDisconnectComplete(const FVoiceChatResult& Result);

    
};
